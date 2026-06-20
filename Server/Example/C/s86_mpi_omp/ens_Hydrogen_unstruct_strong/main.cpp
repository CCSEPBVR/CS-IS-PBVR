#include <string>
#include <iostream>
#include <cstdlib>
#include "Hydrogen.h"
#include "kvs_wrapper.h"
#include "kvs_wrapper_common.h"
#ifndef CPU_VER
#include <mpi.h>
#endif

#define RANK 1

/*===========================================================================*/
/**
 *  @brief  Main function (strong-scaling driver).
 */
/*===========================================================================*/
int main( int argc, char** argv )
{
    MPI_Init( &argc, &argv );
    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &(mpi_rank) );

    // Generate hydrogen volume (fixed global problem, runtime z-slab split).
    Hydrogen hydro;

    domain_parameters_unstruct dom = {
        hydro.global_min_coord.x(),
        hydro.global_min_coord.y(),
        hydro.global_min_coord.z(),
        hydro.global_max_coord.x(),
        hydro.global_max_coord.y(),
        hydro.global_max_coord.z()
    };

    int time_step = 0;
    bool result = false;

    // Number of ensembles = mpi_size / mpi_per_ens (fixed, from the class).
    int mpi_size = 1;
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
    int ens_num = mpi_size / hydro.mpi_per_ens;
    if ( ens_num < 1 ) ens_num = 1;

    if ( mpi_rank == 0 )
        std::cout << "ens_num =  " << ens_num << std::endl;

    begin_wrapper_async_io();
    for(int i =0;i<1;i ++ )
    {
        if(mpi_rank==RANK) hydro.show();

        result = ensemble_generate_particles( time_step, ens_num, dom,
                            hydro.values, hydro.nvariables,
                            hydro.coords, hydro.nnodes,
                            hydro.connections, hydro.ncells, vismodule::VolumeObjectBase::CellType::Hexahedra );
        if ( !result ) return -1;
        time_step++;
    }
    end_wrapper_async_io();

#ifndef CPU_VER
    MPI_Finalize();
#endif

    return 0;
}
