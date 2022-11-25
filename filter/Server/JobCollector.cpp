#include "JobCollector.h"
#include "JobDispatcher.h"
#include "GeometryObjectBase.h"
#include "PointObject.h"
#ifndef CPU_VER
#include <mpi.h>
#endif // CPU_VER
#include "common.h"
#include <stdlib.h>
#include "timer_simple.h"

#ifndef CPU_VER

JobCollector::JobCollector( JobDispatcher* pjd )
{
    m_jd = pjd;
    m_pack_coords  = NULL;
    m_pack_colors  = NULL;
    m_pack_normals = NULL;
    m_pack_size    = 0;
    m_pack_count = 0;
    m_pack_head  = 0;
    m_batch      = false;
}

JobCollector::~JobCollector()
{
    free( m_pack_coords );
    free( m_pack_colors );
    free( m_pack_normals );
}

int JobCollector::jobCollect( pbvr::PointObject* object, VariableRange* vr, bool* invalid, int* wid )
{
    int size;
    int rank;

     MPI_Comm_size( MPI_COMM_WORLD, &size );
     MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    if ( rank == 0 )
    {
        MPI_Status stat;
        double send_time;

        if ( m_batch )
        {
            PBVR_TIMER_STA( 60 );
            MPI_Recv( &send_time, 1, MPI_DOUBLE_PRECISION, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &stat);
            PBVR_TIMER_END( 60 );

            double recv_time = GetTime();
            if ( ( recv_time - send_time ) > 0.0 ) m_jd->setLatency( recv_time - send_time );
            else                              m_jd->setLatency( 0.0 );

            const int src = stat.MPI_SOURCE;
            if ( wid ) *wid = src - 1;

        }
        else
        {
            if ( m_pack_head >= m_pack_size )
            {
                PBVR_TIMER_STA( 60 );
                MPI_Recv( &send_time, 1, MPI_DOUBLE_PRECISION, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &stat );
                PBVR_TIMER_END( 60 );

                double recv_time = GetTime();
                if ( ( recv_time - send_time ) > 0.0 ) m_jd->setLatency( recv_time - send_time );
                else                              m_jd->setLatency( 0.0 );

                const int src = stat.MPI_SOURCE;
                if ( wid ) *wid = src - 1;

                PBVR_TIMER_STA( 62 );
                MPI_Recv( &m_pack_size, sizeof( size_t ), MPI_BYTE, src, 1, MPI_COMM_WORLD, &stat );
                PBVR_TIMER_END( 62 );
                PBVR_TIMER_STA( 63 );
                MPI_Recv( &m_nvertices_list_size, sizeof( size_t ), MPI_BYTE, src, 1, MPI_COMM_WORLD, &stat );
                PBVR_TIMER_END( 63 );
                PBVR_TIMER_STA( 64 );
                m_nvertices_list.assign( m_nvertices_list_size, 0 );
                PBVR_TIMER_END( 64 );
                PBVR_TIMER_STA( 65 );
                MPI_Recv( &m_nvertices_list[0]  , sizeof( size_t )*m_nvertices_list_size, MPI_BYTE, src, 1, MPI_COMM_WORLD, &stat );
                PBVR_TIMER_END( 65 );
                PBVR_TIMER_STA( 66 );

                m_pack_coords  = ( float* )realloc( m_pack_coords , sizeof( float ) * m_pack_size );
                m_pack_colors  = ( unsigned char* )realloc( m_pack_colors , sizeof( unsigned char ) * m_pack_size );
                m_pack_normals = ( float* )realloc( m_pack_normals, sizeof( float ) * m_pack_size );
                PBVR_TIMER_END( 66 );
                PBVR_TIMER_STA( 67 );
                MPI_Recv( m_pack_coords , m_pack_size, MPI_FLOAT, src, 1, MPI_COMM_WORLD, &stat );
                MPI_Recv( m_pack_colors , m_pack_size, MPI_UNSIGNED_CHAR, src, 1, MPI_COMM_WORLD, &stat );
                MPI_Recv( m_pack_normals, m_pack_size, MPI_FLOAT, src, 1, MPI_COMM_WORLD, &stat );
                PBVR_TIMER_END( 67 );
                m_pack_count = 0;
                m_pack_head  = 0;

                // Recive and Merge Variable Range
                unsigned int vr_s;
                MPI_Recv( &vr_s, 1, MPI_UNSIGNED, src, 1, MPI_COMM_WORLD, &stat );
                char* vr_buf = new char[vr_s];
                MPI_Recv( vr_buf, vr_s, MPI_BYTE, src, 1, MPI_COMM_WORLD, &stat  );
                VariableRange vr_t;
                vr_t.unpack( vr_buf );
                vr->merge( vr_t );
                delete[] vr_buf;

                // Recive Validation
                bool sub_invalid;
                MPI_Recv( &sub_invalid, sizeof( bool ), MPI_BYTE, src, 1, MPI_COMM_WORLD, &stat );
                *invalid |= sub_invalid;
            }

            size_t nvertices = m_nvertices_list[m_pack_count];
            size_t nmemb = nvertices * 3;

            float*         coords  = &m_pack_coords[m_pack_head];
            unsigned char* colors  = &m_pack_colors[m_pack_head];
            float*         normals = &m_pack_normals[m_pack_head];

            kvs::ValueArray<kvs::Real32> coords_array( coords, nmemb );
            kvs::ValueArray<kvs::UInt8>  colors_array( colors, nmemb );
            kvs::ValueArray<kvs::Real32> normals_array( normals, nmemb );

            m_pack_head += nmemb;
            m_pack_count++;


            object->clear();
            object->setCoords( coords_array );
            object->setColors( colors_array );
            object->setNormals( normals_array );
        }

    }
    else
    {
        const size_t nvertices = object->nvertices();
        const size_t nmemb = nvertices * 3;

        std::cerr << "*nvertices: " << nvertices << std::endl;

        if ( !m_batch )
        {
            const float*         coords     = object->coords().pointer();
            const unsigned char* colors     = object->colors().pointer();
            const float*         normals    = object->normals().pointer();

            m_nvertices_list.push_back( nvertices );
            m_pack_size += nmemb;
            float*         tmp_coords;
            unsigned char* tmp_colors;
            float*         tmp_normals;
            tmp_coords  = ( float* )realloc( m_pack_coords , sizeof( float ) * m_pack_size );
            tmp_colors  = ( unsigned char* )realloc( m_pack_colors , sizeof( unsigned char ) * m_pack_size );
            tmp_normals = ( float* )realloc( m_pack_normals, sizeof( float ) * m_pack_size );
            m_pack_coords  = tmp_coords;
            m_pack_colors  = tmp_colors;
            m_pack_normals = tmp_normals;
            memcpy( &m_pack_coords[m_pack_size - nmemb], coords, sizeof( float )*nmemb );
            memcpy( &m_pack_colors[m_pack_size - nmemb], colors, sizeof( unsigned char )*nmemb );
            memcpy( &m_pack_normals[m_pack_size - nmemb], normals, sizeof( float )*nmemb );
        }

        if ( m_jd->getCollectSendState() )
        {
            double send_time = GetTime();
            size_t nvertices_list_size = m_nvertices_list.size();
            PBVR_TIMER_STA( 451 );
            MPI_Send( &send_time, 1, MPI_DOUBLE_PRECISION, 0, 1, MPI_COMM_WORLD );
            PBVR_TIMER_END( 451 );
            if ( !m_batch )
            {
                PBVR_TIMER_STA( 452 );
                MPI_Send( &m_pack_size, sizeof( size_t ), MPI_BYTE, 0, 1, MPI_COMM_WORLD );
                PBVR_TIMER_END( 452 );
                PBVR_TIMER_STA( 453 );
                MPI_Send( &nvertices_list_size, sizeof( size_t ), MPI_BYTE, 0, 1, MPI_COMM_WORLD );
                PBVR_TIMER_END( 453 );
                PBVR_TIMER_STA( 454 );
                MPI_Send( &m_nvertices_list[0], sizeof( size_t )*nvertices_list_size, MPI_BYTE, 0, 1, MPI_COMM_WORLD );
                PBVR_TIMER_END( 454 );
                PBVR_TIMER_STA( 456 );
                MPI_Send( m_pack_coords, m_pack_size, MPI_FLOAT, 0, 1, MPI_COMM_WORLD );
                MPI_Send( m_pack_colors, m_pack_size, MPI_UNSIGNED_CHAR, 0, 1, MPI_COMM_WORLD );
                MPI_Send( m_pack_normals, m_pack_size, MPI_FLOAT, 0, 1, MPI_COMM_WORLD );
                PBVR_TIMER_END( 456 );
                m_nvertices_list.clear();
                m_pack_size = 0;

                // Send Variable Range
                unsigned int vr_s = vr->byteSize();
                char* vr_buf = new char[vr_s];
                vr->pack( vr_buf );
                MPI_Send( &vr_s, 1, MPI_UNSIGNED, 0, 1, MPI_COMM_WORLD );
                MPI_Send( vr_buf, vr_s, MPI_BYTE, 0, 1, MPI_COMM_WORLD );
                delete[] vr_buf;

                // Send Validation
                MPI_Send( invalid, sizeof( bool ), MPI_BYTE, 0, 1, MPI_COMM_WORLD );
            }
        }
    }
}

void TestJobCollector()
{
}
#endif // CPU_VER
