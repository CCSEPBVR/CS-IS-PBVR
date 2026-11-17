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

    bool result = false;

    // Each ensemble member (kd = 1 + member) is emitted as a separate time step,
    // so OutputParticles writes one particle file per member (member index in the file name).
    const int num_members = 4;

    begin_wrapper_async_io();
    for( int member = 0; member < num_members; member++ )
    {
        hydro.set_member( member );            // regenerate 256^3 field with kd = 1 + member
        if( mpi_rank == 0 ) hydro.show();

        result = generate_particles( member, dom,
                            hydro.values, hydro.nvariables,
                            hydro.coords, hydro.nnodes,
                            hydro.connections, hydro.ncells, vismodule::VolumeObjectBase::CellType::Hexahedra );
        if ( !result ) return -1;
    }
    end_wrapper_async_io();

#ifndef CPU_VER
    MPI_Finalize();
#endif

    return 0;
}
