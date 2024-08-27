/*****************************************************************************/
/**
 *  @file   main.cpp
 *  @brief  Hydrogen volume data generator.
 *  @author Naohisa Sakamoto
 *
 *  Compile:\
 *  \code
 *  % kvsmake -G
 *  % kvsmake
 *  \endcode
 *
 *  Usage:\n
 *  \code
 *  % ./Hydrogen [-h|--help] <Output filename>
 *  \endcode
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: main.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include <string>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <kvs/Vector2>
#include <kvs/Vector3>
#include <kvs/Message>
#include <mpi.h>
#include "kvs_wrapper.h"
#include "Hydrogen.h"


void Output( void )
{
    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    std::cout<<"mpi_rank="<<mpi_rank<<std::endl;

    Hydrogen hydro;

    //各MPIプロセスにおける、gridの解像度を取得
    int nx = hydro.resol_x;
    int ny = hydro.resol_x;
    int nz = hydro.resol_x;
    int nl = hydro.nleafs;
    int resolution[4] = { nx, ny, nz, nl };
    domain_parameters dom = {
        hydro.global_min_coord.x(),
        hydro.global_min_coord.y(),
        hydro.global_min_coord.z(),
        hydro.global_max_coord.x(),
        hydro.global_max_coord.y(),
        hydro.global_max_coord.z(),
        resolution
    };

    if(mpi_rank==0)
    {
        std::cout<<"nx="<<nx<<",ny="<<ny<<",nz="<<nz<<",nl="<<nl<<std::endl;
        std::cout<<"value_size="<<nx * ny * nz * nl<<std::endl;
    }

    //入力用の多変量データを生成
    static int time_step = 0;
    std::vector<float> cell_length;
    std::vector<float> leaf_min_coord;
    const int nvariables = 4;
    float** values = new float*[nvariables];
    for( int i=0; i<nvariables; i++ )
    {
        const int values_size = nx * ny * nz * nl;
        values[i] = new float[ values_size ];
        std::fill(values[i], values[i] + values_size , 0.0 );
    }

    float min = 10;
    float max = 10;

    int cnt = 0;
    for (int l=0; l<nl; l++)
    {
        cell_length.push_back( hydro.cellLength( l ) );

        const kvs::Vector3f lm( hydro.leafMinCoord( l ) );
        leaf_min_coord.push_back( lm.x() );
        leaf_min_coord.push_back( lm.y() );
        leaf_min_coord.push_back( lm.z() );
        //if(mpi_rank==0) std::cout<<"("<<hydro.cellLength( l )<<","<<lm<<")";

        for (int k=0; k<nz; k++ )
        {
            for (int j=0; j<ny; j++ )
            {
                for (int i=0; i<nx; i++)
                {
                    values[0][cnt] = hydro.value( hydro.gridCoord( i, j, k, l ) );
                    values[1][cnt] = hydro.value( hydro.gridCoord( i, j, k, l ) );
                    values[2][cnt] = hydro.value( hydro.gridCoord( i, j, k, l ) );
                    values[3][cnt] = hydro.value( hydro.gridCoord( i, j, k, l ) );
                    cnt++;

                    float tmp = hydro.value( hydro.gridCoord( i, j, k, l ) );
                    min = min < tmp ? min : tmp;
                    max = max > tmp ? max : tmp;
                }//end of for i
            }//end of for j
        }//end of for k
    }//end of for l

    if(mpi_rank==0) std::cout<<std::endl<<"min="<<min<<",max="<<max<<std::endl;
    generate_particles( time_step, dom,
                        cell_length, leaf_min_coord,
                        nvariables, (const float**)values );

    time_step++;

    for( int i=0; i<nvariables; i++ )
    {
        delete [] values[i];
    }
    delete [] values;
}

/*===========================================================================*/
/**
 *  @brief  Main function
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
int main( int argc, char** argv )
{
    MPI_Init( &argc, &argv );

    // Simulation loop
    begin_wrapper_async_io();
    while( 1 )
    {
        //Simulation();
        Output();
    }
    end_wrapper_async_io();

    MPI_Finalize();

    return 0 ;
}
