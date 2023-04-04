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
    jd = pjd;
    pack_coords  = NULL;
    pack_colors  = NULL;
    pack_normals = NULL;
    pack_size    = 0;
    pack_count = 0;
    pack_head  = 0;
    batch      = false;
}

JobCollector::~JobCollector()
{
    free( pack_coords );
    free( pack_colors );
    free( pack_normals );
}

int JobCollector::JobCollect( pbvr::PointObject* object, VariableRange* vr, int* wid )
{
    int size = MPI::COMM_WORLD.Get_size();
    int rank = MPI::COMM_WORLD.Get_rank();

    if ( rank == 0 )
    {
        MPI::Status stat;
        double send_time;

        if ( batch )
        {
            TIMER_STA( 60 );
            MPI::COMM_WORLD.Recv( &send_time, 1, MPI_DOUBLE_PRECISION, MPI_ANY_SOURCE, 1, stat );
            TIMER_END( 60 );

            double recv_time = get_time();
            if ( ( recv_time - send_time ) > 0.0 ) jd->SetLatency( recv_time - send_time );
            else                              jd->SetLatency( 0.0 );

            const int src = stat.Get_source();
            if ( wid ) *wid = src - 1;

        }
        else
        {
            if ( pack_head >= pack_size )
            {
                TIMER_STA( 60 );
                MPI::COMM_WORLD.Recv( &send_time, 1, MPI_DOUBLE_PRECISION, MPI_ANY_SOURCE, 1, stat );
                TIMER_END( 60 );

                double recv_time = get_time();
                if ( ( recv_time - send_time ) > 0.0 ) jd->SetLatency( recv_time - send_time );
                else                              jd->SetLatency( 0.0 );

                const int src = stat.Get_source();
                if ( wid ) *wid = src - 1;

                TIMER_STA( 62 );
                MPI::COMM_WORLD.Recv( &pack_size          , sizeof( size_t ), MPI_BYTE, src, 1 );
                TIMER_END( 62 );
                TIMER_STA( 63 );
                MPI::COMM_WORLD.Recv( &nvertices_list_size, sizeof( size_t ), MPI_BYTE, src, 1 );
                TIMER_END( 63 );
                TIMER_STA( 64 );
                nvertices_list.assign( nvertices_list_size, 0 );
                TIMER_END( 64 );
                TIMER_STA( 65 );
                MPI::COMM_WORLD.Recv( &nvertices_list[0]  , sizeof( size_t )*nvertices_list_size, MPI_BYTE, src, 1 );
                TIMER_END( 65 );
                TIMER_STA( 66 );

                pack_coords  = ( float* )realloc( pack_coords , sizeof( float ) * pack_size );
                pack_colors  = ( unsigned char* )realloc( pack_colors , sizeof( unsigned char ) * pack_size );
                pack_normals = ( float* )realloc( pack_normals, sizeof( float ) * pack_size );
                TIMER_END( 66 );
                TIMER_STA( 67 );
                MPI::COMM_WORLD.Recv( pack_coords , pack_size, MPI_FLOAT, src, 1 );
                MPI::COMM_WORLD.Recv( pack_colors , pack_size, MPI_UNSIGNED_CHAR, src, 1 );
                MPI::COMM_WORLD.Recv( pack_normals, pack_size, MPI_FLOAT, src, 1 );
                TIMER_END( 67 );
                pack_count = 0;
                pack_head  = 0;

                // Recive and Merge Variable Range
                unsigned int vr_s;
                MPI::COMM_WORLD.Recv( &vr_s, 1, MPI_UNSIGNED, src, 1 );
                char* vr_buf = new char[vr_s];
                MPI::COMM_WORLD.Recv( vr_buf, vr_s, MPI_BYTE, src, 1 );
                VariableRange vr_t;
                vr_t.unpack( vr_buf );
                vr->merge( vr_t );
                delete[] vr_buf;
            }

            size_t nvertices = nvertices_list[pack_count];
            size_t nmemb = nvertices * 3;

            float*         coords  = &pack_coords[pack_head];
            unsigned char* colors  = &pack_colors[pack_head];
            float*         normals = &pack_normals[pack_head];

            kvs::ValueArray<kvs::Real32> coords_array( coords, nmemb );
            kvs::ValueArray<kvs::UInt8>  colors_array( colors, nmemb );
            kvs::ValueArray<kvs::Real32> normals_array( normals, nmemb );

            pack_head += nmemb;
            pack_count++;


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

        if ( !batch )
        {
            const float*         coords     = object->coords().pointer();
            const unsigned char* colors     = object->colors().pointer();
            const float*         normals    = object->normals().pointer();

            nvertices_list.push_back( nvertices );
            pack_size += nmemb;
            float*         tmp_coords;
            unsigned char* tmp_colors;
            float*         tmp_normals;
            tmp_coords  = ( float* )realloc( pack_coords , sizeof( float ) * pack_size );
            tmp_colors  = ( unsigned char* )realloc( pack_colors , sizeof( unsigned char ) * pack_size );
            tmp_normals = ( float* )realloc( pack_normals, sizeof( float ) * pack_size );
            pack_coords  = tmp_coords;
            pack_colors  = tmp_colors;
            pack_normals = tmp_normals;
            memcpy( &pack_coords[pack_size - nmemb], coords, sizeof( float )*nmemb );
            memcpy( &pack_colors[pack_size - nmemb], colors, sizeof( unsigned char )*nmemb );
            memcpy( &pack_normals[pack_size - nmemb], normals, sizeof( float )*nmemb );
        }

        if ( jd->GetColSendStat() )
        {
            double send_time = get_time();
            size_t nvertices_list_size = nvertices_list.size();
            TIMER_STA( 451 );
            MPI::COMM_WORLD.Send( &send_time, 1, MPI_DOUBLE_PRECISION, 0, 1 );
            TIMER_END( 451 );
            if ( !batch )
            {
                TIMER_STA( 452 );
                MPI::COMM_WORLD.Send( &pack_size, sizeof( size_t ), MPI_BYTE, 0, 1 );
                TIMER_END( 452 );
                TIMER_STA( 453 );
                MPI::COMM_WORLD.Send( &nvertices_list_size, sizeof( size_t ), MPI_BYTE, 0, 1 );
                TIMER_END( 453 );
                TIMER_STA( 454 );
                MPI::COMM_WORLD.Send( &nvertices_list[0], sizeof( size_t )*nvertices_list_size, MPI_BYTE, 0, 1 );
                TIMER_END( 454 );
                TIMER_STA( 456 );
                MPI::COMM_WORLD.Send( pack_coords, pack_size, MPI_FLOAT, 0, 1 );
                MPI::COMM_WORLD.Send( pack_colors, pack_size, MPI_UNSIGNED_CHAR, 0, 1 );
                MPI::COMM_WORLD.Send( pack_normals, pack_size, MPI_FLOAT, 0, 1 );
                TIMER_END( 456 );
                nvertices_list.clear();
                pack_size = 0;

                // Send Variable Range
                const unsigned int vr_s = vr->byteSize();
                char* vr_buf = new char[vr_s];
                vr->pack( vr_buf );
                MPI::COMM_WORLD.Send( &vr_s, 1, MPI_UNSIGNED, 0, 1 );
                MPI::COMM_WORLD.Send( vr_buf, vr_s, MPI_BYTE, 0, 1 );
                delete[] vr_buf;
            }
        }
    }
}

void test_JobCollector()
{
}
#endif // CPU_VER
