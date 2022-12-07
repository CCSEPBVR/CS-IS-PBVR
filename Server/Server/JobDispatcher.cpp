
#include "JobDispatcher.h"
#include <iostream>
#ifndef CPU_VER
#include <mpi.h>
#endif

#include "timer_simple.h"

JobDispatcher::JobDispatcher() : m_collect_send_state(0)  {}
JobDispatcher::~JobDispatcher() {}

int JobDispatcher::initialize( const int bstep, const int estep, const std::vector<int>& volume_table )
{
    int size, workers;
#ifdef CPU_VER
    size = 1;
#else
    MPI_Comm_size( MPI_COMM_WORLD, &size );
#endif
    workers = size - 1;

    m_worker_list.clear();
    m_job_list.clear();

    for ( int w = 0; w < workers; w++ )
    {
        WorkerInfomation worker;
        worker.m_rank   = w + 1;
        worker.m_status = 0;
        worker.m_job_id_count = 0;
        worker.m_job_id_pack_size = m_job_id_pack_size;
        worker.m_latency_over_count = 0;
        m_worker_list.push_back( worker );
    }

    int id = 0;
    for ( int s = bstep; s <= estep;   s++ )
    {
        for ( int v = 0; v < volume_table.size(); v++ )
        {
            JobInfomation m_job;
            m_job.m_id     = id;
            m_job.m_volume = volume_table[v];
            m_job.m_step   = s;
            m_job.m_status = 0;
            m_job_list.push_back( m_job );
            id++;
        }
    }

    m_next_job = m_job_list.begin();
    m_job_id_count = 0;

    return 0;
}

#if 0
int JobDispatcher::initialize( int steps, int volumes )
{
    return initialize( 0, steps - 1, volumes );
}

int JobDispatcher::initialize( int bstep, int estep, int volumes, double latency )
{
    m_latency_threshold = latency;
    return initialize( bstep, estep, volumes );
}

int JobDispatcher::initialize( int bstep, int estep, int volumes, double latency, int pack_size )
{
    m_latency_threshold = latency;
    m_job_id_pack_size = pack_size;
    return initialize( bstep, estep, volumes );
}
#else
int JobDispatcher::initialize( const int bstep, const int estep, const int volumes,
                               const std::vector<kvs::Vector3f>& volume_min,
                               const std::vector<kvs::Vector3f>& volume_max,
                               const double latency, const int pack_size )
{
    m_latency_threshold = latency;
    m_job_id_pack_size = pack_size;

    std::vector<int> table;

    for ( int v = 0; v < volumes; v++ )
    {
        table.push_back( v );
    }

    return initialize( bstep, estep, table );
}

int JobDispatcher::initialize( const int bstep, const int estep, const int volumes,
                               const std::vector<kvs::Vector3f>& volume_min,
                               const std::vector<kvs::Vector3f>& volume_max,
                               const double latency, const int pack_size,
                               const kvs::Vector3f& crop_min, const kvs::Vector3f& crop_max )
{
    m_latency_threshold = latency;
    m_job_id_pack_size = pack_size;

    std::vector<int> table;
    m_count_volumes = 0;

    for ( int v = 0; v < volumes; v++ )
    {
        if ( ( ( volume_min[v].x() < crop_max.x() ) && ( crop_min.x() < volume_max[v].x() ) )
                && ( ( volume_min[v].y() < crop_max.y() ) && ( crop_min.y() < volume_max[v].y() ) )
                && ( ( volume_min[v].z() < crop_max.z() ) && ( crop_min.z() < volume_max[v].z() ) ) )
        {
            table.push_back( v );
            m_count_volumes++;
        }
    }

    return initialize( bstep, estep, table );
}
#endif


int JobDispatcher::dispatchNext_Local( int worker_id, int* step, int* volume )
{

    int ret = 0;
    if ( m_next_job != m_job_list.end() )
    {
        int jid = m_next_job->m_id;

        *step   = m_job_list[jid].m_step;
        *volume = m_job_list[jid].m_volume;
        ret = 1;

        m_next_job++;

        std::cerr << "Dispatched Job "
                  << *step   << " "
                  << *volume << " "
                  << std::endl;
        std::flush( std::cerr );
    }
    else
    {
        ret = 0;
        std::cerr << "Dispatched No Job "
                  << std::endl;
        std::flush( std::cerr );
    }

    return ret;
}

#ifdef CPU_VER

int JobDispatcher::dispatchNext( int worker_id, int* step, int* volume )
{
    return dispatchNext_Local(worker_id, step, volume);
}

#else

int JobDispatcher::dispatchNext( const int worker_id, int* step, int* volume )
{
    PBVR_TIMER_STA( 21 );
    int rank, size, count;
    int ret = 0;
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    // modify by @hira
    if (size == 1) {
        return this->dispatchNext_Local(worker_id, step, volume);
    }

    if ( rank == 0)
    {
        PBVR_TIMER_STA( 31 );
        MPI_Request* req    = new MPI_Request[size];
        MPI_Status*  status = new MPI_Status [size];
        MPI_Request* req2    = new MPI_Request[size];
        MPI_Status*  status2 = new MPI_Status [size];

        if ( worker_id >= 0 && m_worker_list[worker_id].m_status == 1 )
        {
            m_worker_list[worker_id].m_job->m_status = 2;
            m_worker_list[worker_id].m_status = 0;
            m_worker_list[worker_id].m_job_id_count = 0;

            if ( m_latency_threshold >= 0.0 && m_latency >= m_latency_threshold )
                m_worker_list[worker_id].m_job_id_pack_size++ ;

#if WRITE_DISPATCH_LOG
            std::cerr << "Job "
                      << m_worker_list[worker_id].job->step   << " "
                      << m_worker_list[worker_id].job->volume << " "
                      << "(Rank " << m_worker_list[worker_id].rank << ") "
                      << "is over." << std::endl;
            std::cerr << "Next " << m_worker_list[worker_id].m_job_id_pack_size << " jobs"
                      << "(latency " << m_latency << " [ms])"
                      << std::endl;
            std::flush( std::cerr );
#endif
        }
        else if ( worker_id >= 0 && m_worker_list[worker_id].m_status == 2 )
        {
            m_worker_list[worker_id].m_job->m_status = 2;
            m_worker_list[worker_id].m_job_id_count++;

            if ( m_worker_list[worker_id].m_job_id_count == m_worker_list[worker_id].m_job_id_pack_size - 1 )
                m_worker_list[worker_id].m_status =  1;

            if ( m_worker_list[worker_id].m_job_id_pack[m_worker_list[worker_id].m_job_id_count] < 0 )
                m_worker_list[worker_id].m_status =  -1;

#if WRITE_DISPATCH_LOG
            std::cerr << "Job "
                      << m_worker_list[worker_id].m_job->step   << " "
                      << m_worker_list[worker_id].m_job->volume << " "
                      << "(Rank " << m_worker_list[worker_id].rank << ") "
                      << "is over." << std::endl;
            std::flush( std::cerr );
#endif
            m_worker_list[worker_id].m_job++;

        }
        count = 0;
        PBVR_TIMER_END( 31 );
        PBVR_TIMER_STA( 30 );

        for ( std::vector<WorkerInfomation>::iterator w = m_worker_list.begin();
                w != m_worker_list.end(); w++ )
        {
            if ( w->m_status == 0 )
            {
                // worker is idoling
                int j;
                w->m_job = m_next_job;
                w->m_job_id_pack.clear();
                for ( j = 0; j < w->m_job_id_pack_size; j++ )
                {
                    if ( m_next_job != m_job_list.end() )
                    {
                        // there are jobs, with yet completed.
                        w->m_job_id_pack.push_back( m_next_job->m_id );
                        if ( j > 0 ) w->m_status = 2;
                        else      w->m_status = 1;

                        m_next_job->m_status = 1;

#if WRITE_DISPATCH_LOG
                        std::cerr << "Dispatched Job "
                                  << next_job->step   << " "
                                  << next_job->volume << " "
                                  << "(Rank " << w->rank << ") "
                                  << std::endl;
                        std::flush( std::cerr );
#endif

                        m_next_job++;

                    }
                    else
                    {
                        // all jobs are completed.
                        w->m_job_id_pack.push_back( -1 );
                        if ( j > 0 ) w->m_status =  2;
                        else      w->m_status = -1;

#if WRITE_DISPATCH_LOG
                        std::cerr << "No Job. "
                                  << "Rank " << w->rank << " is going exit."
                                  << std::endl;
                        std::flush( std::cerr );
#endif
                        break;
                    }
                }
                w->m_job_id_pack_size = w->m_job_id_pack.size();
                MPI_Isend( &( w->m_job_id_pack_size ), 1, MPI_INT, w->m_rank, 2, MPI_COMM_WORLD, &req[count] );
                MPI_Isend( &( w->m_job_id_pack[0] ), w->m_job_id_pack_size, MPI_INT, w->m_rank, 3, MPI_COMM_WORLD, &req2[count] );
                count++;
            }
        }
        PBVR_TIMER_END( 30 );

        PBVR_TIMER_STA( 32 );
        ret = 0;
        for ( std::vector<WorkerInfomation>::iterator w = m_worker_list.begin();
                w != m_worker_list.end(); w++ )
        {
            if ( w->m_status != -1 ) ret++;
        }
        PBVR_TIMER_END( 32 );

        if ( count != 0 )
        {
            PBVR_TIMER_STA( 35 );
            MPI_Waitall( count, req, status );
            MPI_Waitall( count, req2, status2 );
            PBVR_TIMER_END( 35 );
        }

        PBVR_TIMER_STA( 33 );
        delete[] req;
        delete[] status;
        delete[] req2;
        delete[] status2;
        PBVR_TIMER_END( 33 );

    }
    else
    {
        int jid;
        MPI_Status stat;
        MPI_Status stat2;

        if ( m_job_id_count == 0 )
        {
            PBVR_TIMER_STA( 40 );
            MPI_Recv( &m_job_id_pack_recv_size, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &stat );
            PBVR_TIMER_END( 40 );
            PBVR_TIMER_STA( 41 );
            m_job_id_pack_recv.assign( m_job_id_pack_recv_size, 0 );
            PBVR_TIMER_END( 41 );
            PBVR_TIMER_STA( 42 );
            MPI_Recv( &m_job_id_pack_recv[0], m_job_id_pack_recv_size, MPI_INT, 0, 3, MPI_COMM_WORLD, &stat2 );
            PBVR_TIMER_END( 42 );
            m_collect_send_state = 0;
        }

        jid = m_job_id_pack_recv[m_job_id_count];

        if ( jid >= 0 )
        {
            *step   = m_job_list[jid].m_step;
            *volume = m_job_list[jid].m_volume;
            ret = 1;

#if WRITE_DISPATCH_LOG
            std::cerr << "Rank " << rank << ": "
                      << "Dispatched Job "
                      << *step   << " "
                      << *volume << " "
                      << std::endl;
            std::flush( std::cerr );
#endif
        }
        else
        {
            ret = 0;

#if WRITE_DISPATCH_LOG
            std::cerr << "Rank " << rank << ": "
                      << "Dispatched No Job "
                      << std::endl;
            std::flush( std::cerr );
#endif
            PBVR_TIMER_END( 21 );
            return ret;
        }

        m_job_id_count++;

        if ( m_job_id_pack_recv_size == m_job_id_count || m_job_id_pack_recv[m_job_id_count] < 0 )
        {
            m_collect_send_state = 1;
        }

        if ( m_job_id_pack_recv_size == m_job_id_count )
        {
            m_job_id_count = 0;
        }
    }

    PBVR_TIMER_END( 21 );
    return ret;
}

#endif

JobDispatcher::JobInfomation JobDispatcher::getWorkerJobInfomation( const int wid )
{
    return *( m_worker_list[wid].m_job );
}

void JobDispatcher::setWorkerStatus( const int wid, const int status )
{
    m_worker_list[wid].m_status = status;
}
#if 0
void TestJobDispatcher()
{
    JobDispatcher jd;
#ifdef CPU_VER
    int size = 1;
#else
    int size = MPI::COMM_WORLD.Get_size();
#endif
    int step, volume, wid;

    jd.initialize( 1, 2, 2 );
    wid = 0;
    while ( jd.dispatchNext( wid, &step, &volume ) )
    {
        wid++;
        wid %= ( size - 1 );
    };
}
#endif
