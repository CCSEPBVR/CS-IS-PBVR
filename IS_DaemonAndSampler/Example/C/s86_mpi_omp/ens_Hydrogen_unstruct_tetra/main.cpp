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
#if 1
    begin_wrapper_async_io();
    for(int i =0; i< 1; i++ )
    //for(;;)
    {
        if(mpi_rank==0) hydro.show();

        ensemble_generate_particles( time_step, dom,
                            hydro.values, hydro.nvariables,
                            hydro.coords, hydro.nnodes,
                            hydro.connections, hydro.ncells, pbvr::VolumeObjectBase::CellType::Tetrahedra  );

        time_step++;
    }
    end_wrapper_async_io();
#else
    // 平均値計算
    Hydrogen hydro_average; 
    int mpi_size = 8;
    //MPI_Comm_rank( MPI_COMM_WORLD, &(mpi_size) );
    hydro.calc_average(mpi_size);
    //hydro.calc_each_ens(mpi_size);

    for(int i =0; i< 1; i++ )
    //for(;;)
    {
        if(mpi_rank==0) hydro.show();

//        ensemble_generate_particles( time_step, dom,
//                            hydro_average.values, hydro_average.nvariables,
//                            hydro_average.coords, hydro_average.nnodes,
//                            hydro_average.connections, hydro_average.ncells, pbvr::VolumeObjectBase::CellType::Tetrahedra  );
        ensemble_generate_particles( time_step, dom,
                            hydro.values, hydro.nvariables,
                            hydro.coords, hydro.nnodes,
                            hydro.connections, hydro.ncells, pbvr::VolumeObjectBase::CellType::Tetrahedra  );


        time_step++;
    }
#endif


    MPI_Finalize();

    return 0;
}
