program main
  use mpi
  use Hydrogen,only : &
       values,nvariables, &
       coords,nnodes, &
       connections,ncells, &
       global_min_coord, &
       global_max_coord, &
       dom
  use Hydrogen,only : &
       Hydrogen_init, &
       hydro_show, &
       Hydrogen_free

  use kvs_wrapper_unstruct,only : &
       generate_particles

  implicit none

  integer ierr
  integer mpi_rank0
  character*256 filename
  integer time_step 
  integer i
  integer,parameter ::  RANK=1
  call MPI_Init(ierr)
  call MPI_Comm_Rank(MPI_COMM_WORLD,mpi_rank0,ierr)
  call Hydrogen_init()
!     //全体の最大最小値を示すpfiファイルを生成
!     char filename[256];
!     sprintf(filename,"./jupiter_particle_out/t_pfi_coords_minmax.txt");
!     FILE* fp = fopen( filename, "w" );
!     if( fp )
!     {
!         fprintf( fp, "%f %f %f %f %f %f\n",
!                  hydro.global_min_coord.x(),
!                  hydro.global_min_coord.y(),
!                  hydro.global_min_coord.z(),
!                  hydro.global_max_coord.x(),
!                  hydro.global_max_coord.y(),
!                  hydro.global_max_coord.z() );
!         fclose( fp );
!     }
! -----
! if(mpi_rank0.eq.0)then
!    filename="./jupiter_particle_out/t_pfi_coords_minmax.txt"
!    open(55,file=filename,status='replace')
!    write(55,*) &
!         global_min_coord(1), &
!         global_min_coord(2), &
!         global_min_coord(3), &
!         global_max_coord(1), &
!         global_max_coord(2), &
!         global_max_coord(3)
!    close(55)
! endif

  time_step=0
!  do i=1,10
  do 
     if(mpi_rank0.eq.RANK)then
        call hydro_show()
     endif
     call generate_particles( &
          time_step, dom, &
          values, nvariables, &
          coords, nnodes, &
          connections, ncells )
     time_step=time_step+1
  enddo
  call Hydrogen_free()
  call MPI_Finalize(ierr)

end program main

! #include <string>
! #include <iostream>
! #include "Hydrogen.h"
! #include "kvs_wrapper.h"
! #include <mpi.h>
! #define RANK 1
! /*===========================================================================*/
! /**
!  *  @brief  Main function.
!  *  @param  argc [i] argument counter
!  *  @param  argv [i] argument values
!  */
! /*===========================================================================*/
! int main( int argc, char** argv )
! {
!     MPI_Init( &argc, &argv );
!     int mpi_rank;
!     MPI_Comm_rank( MPI_COMM_WORLD, &(mpi_rank) );

!     // Generate hydrogen volume.
!     Hydrogen hydro;

!     //全体の最大最小値を示すpfiファイルを生成
!     char filename[256];
!     sprintf(filename,"./jupiter_particle_out/t_pfi_coords_minmax.txt");
!     FILE* fp = fopen( filename, "w" );
!     if( fp )
!     {
!         fprintf( fp, "%f %f %f %f %f %f\n",
!                  hydro.global_min_coord.x(),
!                  hydro.global_min_coord.y(),
!                  hydro.global_min_coord.z(),
!                  hydro.global_max_coord.x(),
!                  hydro.global_max_coord.y(),
!                  hydro.global_max_coord.z() );
!         fclose( fp );
!     }

!     //全体の座標
!     domain_parameters dom = {
!         hydro.global_min_coord.x(),
!         hydro.global_min_coord.y(),
!         hydro.global_min_coord.z(),
!         hydro.global_max_coord.x(),
!         hydro.global_max_coord.y(),
!         hydro.global_max_coord.z()
!     };

!     int time_step = 0;

!     for(;;)
!     {
!         if(mpi_rank==RANK) hydro.show();

!         generate_particles( time_step, dom,
!                             hydro.values, hydro.nvariables,
!                             hydro.coords, hydro.nnodes,
!                             hydro.connections, hydro.ncells );

!         time_step++;
!     }

!     MPI_Finalize();

!     return 0;
! }
