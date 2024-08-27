#include <string>
#include <iostream>
#include <cstdlib>
#include "Hydrogen.h"
#include "kvs_wrapper.h"
#include <mpi.h>

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
     MPI_Init( &argc, &argv );
    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &(mpi_rank) );

    // Generate hydrogen volume.
    Hydrogen hydro;

    //全体の座標
    domain_parameters dom = {
        hydro.global_min_coord.x(),
        hydro.global_min_coord.y(),
        hydro.global_min_coord.z(),
        hydro.global_max_coord.x(),
        hydro.global_max_coord.y(),
        hydro.global_max_coord.z()
    };

    int time_step = 0;

    begin_wrapper_async_io();
    for(;;)
    {
        if(mpi_rank==RANK) hydro.show();

        generate_particles( time_step, dom,
                            hydro.values, hydro.nvariables,
                            hydro.coords, hydro.nnodes,
                            hydro.connections, hydro.ncells, pbvr::VolumeObjectBase::CellType::Hexahedra );
        time_step++;
    }
    end_wrapper_async_io();

    MPI_Finalize();

    return 0;
}
