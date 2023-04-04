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
#include "Hydrogen.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <kvs/Vector3>
#include <mpi.h>
#include "kvs_wrapper.h"

/*===========================================================================*/
/**
 *  @brief  Main function
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
int main( int argc, char** argv )
{
    MPI_Init(&argc, &argv);



    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &(mpi_rank) );

    // Generate hydrogen volume.
    Hydrogen hydro;

    int resol[3] = { hydro.resolution.x(), hydro.resolution.y(), hydro.resolution.z() };

    domain_parameters dom = {
        hydro.global_min_coord.x(),
        hydro.global_min_coord.y(),
        hydro.global_min_coord.z(),
        hydro.global_max_coord.x(),
        hydro.global_max_coord.y(),
        hydro.global_max_coord.z(),

        hydro.global_region[mpi_rank].x(),
        hydro.global_region[mpi_rank].y(),
        0.0,

        resol,
        hydro.cell_length
    };


    int time_step = 0;

    begin_wrapper_async_io();
    for(int i=0;i<20;i++)
    {
        if(mpi_rank==0) hydro.show();

        generate_particles( time_step, dom, hydro.values, hydro.nvariables );

        time_step++;
    }
    end_wrapper_async_io();


    MPI_Finalize();

    return 0 ;
}
