
#include "JobDispatcher.h"
#include <iostream>
#ifndef CPU_VER
#include <mpi.h>
#endif

#include "timer_simple.h"

JobDispatcher::JobDispatcher()  {}
JobDispatcher::~JobDispatcher() {}

int JobDispatcher::Initialize( int bstep, int estep, std::vector<int> volume_table )
{
    int size, workers;
#ifdef CPU_VER
    size = 1;
#else
    MPI_Comm_size( MPI_COMM_WORLD, &size );
#endif
    workers = size - 1;

    worker_list.clear();
    job_list.clear();

    for ( int w = 0; w < workers; w++ )
    {
        worker_info worker;
        worker.rank   = w + 1;
        worker.status = 0;
        worker.jid_count = 0;
        worker.jid_pack_size = jid_pack_size;
        worker.latency_over_count = 0;
        worker_list.push_back( worker );
    }

    int id = 0;
    for ( int s = bstep; s <= estep;   s++ )
    {
        for ( int v = 0; v < volume_table.size(); v++ )
        {
            job_info job;
            job.id     = id;
            job.volume = volume_table[v];
            job.step   = s;
            job.status = 0;
            job_list.push_back( job );
            id++;
        }
    }

    next_job = job_list.begin();
    jid_count = 0;

    return 0;
}

#if 0
int JobDispatcher::Initialize( int steps, int volumes )
{
    return Initialize( 0, steps - 1, volumes );
}

int JobDispatcher::Initialize( int bstep, int estep, int volumes, double latency )
{
    latency_threshold = latency;
    return Initialize( bstep, estep, volumes );
}

int JobDispatcher::Initialize( int bstep, int estep, int volumes, double latency, int pack_size )
{
    latency_threshold = latency;
    jid_pack_size = pack_size;
    return Initialize( bstep, estep, volumes );
}
#else
int JobDispatcher::Initialize( int bstep, int estep, int volumes,
                               std::vector<kvs::Vector3f> volume_min,
                               std::vector<kvs::Vector3f> volume_max,
                               double latency, int pack_size )
{
    latency_threshold = latency;
    jid_pack_size = pack_size;

    std::vector<int> table;

    for ( int v = 0; v < volumes; v++ )
    {
        table.push_back( v );
    }

    return Initialize( bstep, estep, table );
}

int JobDispatcher::Initialize( int bstep, int estep, int volumes,
                               std::vector<kvs::Vector3f> volume_min,
                               std::vector<kvs::Vector3f> volume_max,
                               double latency, int pack_size,
                               kvs::Vector3f crop_min, kvs::Vector3f crop_max )
{
    latency_threshold = latency;
    jid_pack_size = pack_size;

    std::vector<int> table;
    count_volumes = 0;

    for ( int v = 0; v < volumes; v++ )
    {
        if ( ( ( volume_min[v].x() < crop_max.x() ) && ( crop_min.x() < volume_max[v].x() ) )
                && ( ( volume_min[v].y() < crop_max.y() ) && ( crop_min.y() < volume_max[v].y() ) )
                && ( ( volume_min[v].z() < crop_max.z() ) && ( crop_min.z() < volume_max[v].z() ) ) )
        {
            table.push_back( v );
            count_volumes++;
        }
    }

    return Initialize( bstep, estep, table );
}
#endif

#ifdef CPU_VER

int JobDispatcher::DispatchNext( int worker_id, int* step, int* volume )
{

    int ret = 0;
    if ( next_job != job_list.end() )
    {
        int jid = next_job->id;

        *step   = job_list[jid].step;
        *volume = job_list[jid].volume;
        ret = 1;

        next_job++;

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

#else

int JobDispatcher::DispatchNext( int worker_id, int* step, int* volume )
{
    TIMER_STA( 21 );
    int rank, size, count;
    int ret = 0;
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    if ( rank == 0 )
    {
        TIMER_STA( 31 );
        MPI_Request* req    = new MPI_Request[size];
        MPI_Status*  status = new MPI_Status [size];
        MPI_Request* req2    = new MPI_Request[size];
        MPI_Status*  status2 = new MPI_Status [size];

        if ( worker_id >= 0 && worker_list[worker_id].status == 1 )
        {
            worker_list[worker_id].job->status = 2;
            worker_list[worker_id].status = 0;
            worker_list[worker_id].jid_count = 0;

            if ( latency_threshold >= 0.0 && latency >= latency_threshold )
                worker_list[worker_id].jid_pack_size++ ;

#if WRITE_DISPATCH_LOG
            std::cerr << "Job "
                      << worker_list[worker_id].job->step   << " "
                      << worker_list[worker_id].job->volume << " "
                      << "(Rank " << worker_list[worker_id].rank << ") "
                      << "is over." << std::endl;
            std::cerr << "Next " << worker_list[worker_id].jid_pack_size << " jobs"
                      << "(latency " << latency << " [ms])"
                      << std::endl;
            std::flush( std::cerr );
#endif
        }
        else if ( worker_id >= 0 && worker_list[worker_id].status == 2 )
        {
            worker_list[worker_id].job->status = 2;
            worker_list[worker_id].jid_count++;

            if ( worker_list[worker_id].jid_count == worker_list[worker_id].jid_pack_size - 1 )
                worker_list[worker_id].status =  1;

            if ( worker_list[worker_id].jid_pack[worker_list[worker_id].jid_count] < 0 )
                worker_list[worker_id].status =  -1;

#if WRITE_DISPATCH_LOG
            std::cerr << "Job "
                      << worker_list[worker_id].job->step   << " "
                      << worker_list[worker_id].job->volume << " "
                      << "(Rank " << worker_list[worker_id].rank << ") "
                      << "is over." << std::endl;
            std::flush( std::cerr );
#endif
            worker_list[worker_id].job++;

        }
        count = 0;
        TIMER_END( 31 );
        TIMER_STA( 30 );

        for ( std::vector<worker_info>::iterator w = worker_list.begin();
                w != worker_list.end(); w++ )
        {
            if ( w->status == 0 )
            {
                // worker is idoling
                int j;
                w->job = next_job;
                w->jid_pack.clear();
                for ( j = 0; j < w->jid_pack_size; j++ )
                {
                    if ( next_job != job_list.end() )
                    {
                        // there are jobs, with yet completed.
                        w->jid_pack.push_back( next_job->id );
                        if ( j > 0 ) w->status = 2;
                        else      w->status = 1;

                        next_job->status = 1;

#if WRITE_DISPATCH_LOG
                        std::cerr << "Dispatched Job "
                                  << next_job->step   << " "
                                  << next_job->volume << " "
                                  << "(Rank " << w->rank << ") "
                                  << std::endl;
                        std::flush( std::cerr );
#endif

                        next_job++;

                    }
                    else
                    {
                        // all jobs are completed.
                        w->jid_pack.push_back( -1 );
                        if ( j > 0 ) w->status =  2;
                        else      w->status = -1;

#if WRITE_DISPATCH_LOG
                        std::cerr << "No Job. "
                                  << "Rank " << w->rank << " is going exit."
                                  << std::endl;
                        std::flush( std::cerr );
#endif
                        break;
                    }
                }
                w->jid_pack_size = w->jid_pack.size();
                MPI_Isend( &( w->jid_pack_size ), 1, MPI_INT, w->rank, 2, MPI_COMM_WORLD, &req[count] );
                MPI_Isend( &( w->jid_pack[0] ), w->jid_pack_size, MPI_INT, w->rank, 3, MPI_COMM_WORLD, &req2[count] );
                count++;
            }
        }
        TIMER_END( 30 );

        TIMER_STA( 32 );
        ret = 0;
        for ( std::vector<worker_info>::iterator w = worker_list.begin();
                w != worker_list.end(); w++ )
        {
            if ( w->status != -1 ) ret++;
        }
        TIMER_END( 32 );

        if ( count != 0 )
        {
            TIMER_STA( 35 );
            MPI_Waitall( count, req, status );
            MPI_Waitall( count, req2, status2 );
            TIMER_END( 35 );
        }

        TIMER_STA( 33 );
        delete[] req;
        delete[] status;
        delete[] req2;
        delete[] status2;
        TIMER_END( 33 );

    }
    else
    {
        int jid;
        MPI_Status stat;
        MPI_Status stat2;

        if ( jid_count == 0 )
        {
            TIMER_STA( 40 );
            MPI_Recv( &jid_pack_recv_size, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, &stat );
            TIMER_END( 40 );
            TIMER_STA( 41 );
            jid_pack_recv.assign( jid_pack_recv_size, 0 );
            TIMER_END( 41 );
            TIMER_STA( 42 );
            MPI_Recv( &jid_pack_recv[0], jid_pack_recv_size, MPI_INT, 0, 3, MPI_COMM_WORLD, &stat2 );
            TIMER_END( 42 );
            col_send_stat = 0;
        }

        jid = jid_pack_recv[jid_count];

        if ( jid >= 0 )
        {
            *step   = job_list[jid].step;
            *volume = job_list[jid].volume;
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
            TIMER_END( 21 );
            return ret;
        }

        jid_count++;

        if ( jid_pack_recv_size == jid_count || jid_pack_recv[jid_count] < 0 )
        {
            col_send_stat = 1;
        }

        if ( jid_pack_recv_size == jid_count )
        {
            jid_count = 0;
        }
    }

    TIMER_END( 21 );
    return ret;
}

#endif

JobDispatcher::job_info JobDispatcher::GetWorkerJobInfo( int wid )
{
    return *( worker_list[wid].job );
}

void JobDispatcher::setWorkerStatus( int wid, int status )
{
    worker_list[wid].status = status;
}
#if 0
void test_JobDispatcher()
{
    JobDispatcher jd;
#ifdef CPU_VER
    int size = 1;
#else
    int size = MPI::COMM_WORLD.Get_size();
#endif
    int step, volume, wid;

    jd.Initialize( 1, 2, 2 );
    wid = 0;
    while ( jd.DispatchNext( wid, &step, &volume ) )
    {
        wid++;
        wid %= ( size - 1 );
    };
}
#endif
