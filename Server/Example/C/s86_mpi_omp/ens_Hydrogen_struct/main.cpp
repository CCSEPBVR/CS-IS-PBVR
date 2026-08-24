/*****************************************************************************/
/**
 *  @file   main.cpp
 *  @brief  Hydrogen volume data generator.
 *  @author Naohisa Sakamoto
 *
 *  Compile:\
 *  \code
 *  % vismodulemake -G
 *  % vismodulemake
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
 *  See http://www.viz.media.kyoto-u.ac.jp/vismodule/copyright/ for details.
 *
 *  $Id: main.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "Hydrogen.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include "kvs_wrapper_common.h"
#include "kvs_wrapper.h"

#ifndef CPU_VER
#include <mpi.h>
#endif

/*===========================================================================*/
/**
 *  @brief  Main function
 *  @param  argc [in] argument count
 *  @param  argv [in] argument values
 */
/*===========================================================================*/
int main( int argc, char** argv )
{
    int mpi_rank;
    int ens_num = 1;
#ifndef CPU_VER
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &(mpi_rank) );
    MPI_Comm_size( MPI_COMM_WORLD, &ens_num );
#else
    mpi_rank = 0;
#endif

    // Generate hydrogen volume.
    Hydrogen hydro;

    int resol[3] = { hydro.resolution.x(), hydro.resolution.y(), hydro.resolution.z() };

    domain_parameters_struct dom = {
        hydro.global_min_coord.x(),
        hydro.global_min_coord.y(),
        hydro.global_min_coord.z(),
        hydro.global_max_coord.x(),
        hydro.global_max_coord.y(),
        hydro.global_max_coord.z(),
        0.0,
        0.0,
        0.0,
        127,
        127,
        127,
        resol,
        hydro.cell_length
    };

    int time_step = 0;

    begin_wrapper_async_io();
    //for(int i=0;i<20;i++)
    for(int i=0;i<2;i++)  // v1 テスト: 2ステップ
    {
        if(mpi_rank==0) hydro.show();

        ensemble_generate_particles( time_step, ens_num, dom, hydro.values, hydro.nvariables );
        time_step++;
    }
    end_wrapper_async_io();

#ifndef CPU_VER
    MPI_Finalize();
#endif

    return 0;
}
