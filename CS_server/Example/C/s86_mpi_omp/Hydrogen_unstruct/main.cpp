#include "Hydrogen.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include "kvs_wrapper.h"
#include "kvs_wrapper_common.h"
#ifndef CPU_VER
#include <mpi.h>
#endif

#define RANK 1

/*===========================================================================*/
/**
 *  @brief  Main function.
 *  @param  argc [i] argument counter
 *  @param  argv [i] argument values
 */
/*===========================================================================*/
int main( int argc, char** argv )
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &(mpi_rank) );
#else
    mpi_rank = 0;
#endif

    // Generate hydrogen volume.
    Hydrogen hydro;

    //全体の座標
    //domain_parameters dom = {
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

    begin_wrapper_async_io();
    for(;;)
    {
        if(mpi_rank==RANK) hydro.show();

        result = generate_particles( time_step, dom,
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
