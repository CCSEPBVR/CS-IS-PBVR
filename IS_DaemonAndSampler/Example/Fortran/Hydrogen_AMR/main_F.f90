subroutine Output(time_step)
  use kvs_wrapper_AMR,only:domain_parameters
  use kvs_wrapper_AMR,only:generate_particles
  use Hydrogen,only:&
       global_min_coord, &
       global_max_coord, &
       resol_x,nleafs
  use Hydrogen,only:&
       Hydrogen_init, &
       hydro_cellLength, &
       hydro_leafMinCoord, &
       hydro_gridCoord, &
       hydro_value

  use mpi
  implicit none
  integer mpi_rank0
  integer ierr
  character*256 filename
  integer time_step 

  integer nx,ny,nz,nl
  integer resolution(4)
  type(domain_parameters) :: dom 

  real*4,allocatable :: cell_length(:)
  real*4,allocatable :: leaf_min_coord(:)
  integer,parameter :: nvariables = 4
  real*4,allocatable :: values(:,:)
  real*4 val_min
  real*4 val_max
  integer cnt 
  integer values_size
  integer i,j,k,l
  real*4 lm(3)
  real*4 gridCoord(3)
  real*4 tmp

  call MPI_Comm_Rank(MPI_COMM_WORLD,mpi_rank0,ierr)
  write(*,*)"mpi_rank=",mpi_rank0
  call Hydrogen_init()

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

    nx = resol_x
    ny = resol_x
    nz = resol_x
    nl = nleafs
    resolution(1) = nx
    resolution(2) = ny
    resolution(3) = nz
    resolution(4) = nl 

    dom%global_min_coord_x = global_min_coord(1)
    dom%global_min_coord_y = global_min_coord(2)
    dom%global_min_coord_z = global_min_coord(3)
    dom%global_max_coord_x = global_max_coord(1)
    dom%global_max_coord_y = global_max_coord(2)
    dom%global_max_coord_z = global_max_coord(3)
    dom%resolution(1)=resolution(1)
    dom%resolution(2)=resolution(2)
    dom%resolution(3)=resolution(3)
    dom%resolution(4)=resolution(4)

    if(mpi_rank0.eq.0)then   
       write(*,*)"nx=",nx,",ny=",ny,",nz=",nz,",nl=",nl
       write(*,*)"value_size=",nx * ny * nz * nl
    endif

    ! ----
  ! //入力用の多変量データを生成
  values_size=nx * ny * nz * nl
  allocate(values(nvariables,values_size))
  values=0.0

  val_min = 10
  val_max = 10
  cnt = 0
  allocate(cell_length(nl))
  allocate(leaf_min_coord(nl*3))
  do l=0,nl-1
     call hydro_cellLength(l,cell_length(l+1))
     call hydro_leafMinCoord(l,leaf_min_coord(l*3+1))
     do k=0,nz-1
        do j=0,ny-1
           do i=0,nx-1
              call hydro_gridCoord(i,j,k,l,gridCoord)
              call hydro_value( gridCoord,values(0+1,cnt+1) )
              call hydro_value( gridCoord,values(1+1,cnt+1) )
              call hydro_value( gridCoord,values(2+1,cnt+1) )
              call hydro_value( gridCoord,values(3+1,cnt+1) )
              cnt=cnt+1
              call hydro_value( gridCoord,tmp )

              ! min = min < tmp ? min : tmp;
              ! max = max > tmp ? max : tmp;
              val_min = min(val_min , tmp)
              val_max = max(val_max , tmp)

           enddo
        enddo
     enddo
  enddo
  if(mpi_rank0.eq.0)then
     write(*,*)
     write(*,*)"min=",val_min,",max=",val_max
  endif
  call generate_particles( &
       time_step, dom, &
       cell_length, leaf_min_coord, &
       nvariables, values )

  deallocate(leaf_min_coord)
  deallocate(cell_length)
  deallocate(values)

end subroutine Output

program main
  implicit none
  integer ierr
  integer i
  integer time_step 
  call MPI_Init(ierr)

  time_step=0
  do
!   do i=1,3
     call Output(time_step)
     time_step =time_step +1
  enddo

  call MPI_Finalize(ierr)

end program main
! /*****************************************************************************/
! /**
!  *  @file   main.cpp
!  *  @brief  Hydrogen volume data generator.
!  *  @author Naohisa Sakamoto
!  *
!  *  Compile:\
!  *  \code
!  *  % kvsmake -G
!  *  % kvsmake
!  *  \endcode
!  *
!  *  Usage:\n
!  *  \code
!  *  % ./Hydrogen [-h|--help] <Output filename>
!  *  \endcode
!  */
! /*----------------------------------------------------------------------------
!  *
!  *  Copyright (c) Visualization Laboratory, Kyoto University.
!  *  All rights reserved.
!  *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
!  *
!  *  $Id: main.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
!  */
! /*****************************************************************************/
! #include <string>
! #include <iostream>
! #include <vector>
! #include <kvs/Vector2>
! #include <kvs/Vector3>
! #include <kvs/Message>
! #include <mpi.h>
! #include "kvs_wrapper.h"
! #include "Hydrogen.h"


! void Output( void )
! {
!     int mpi_rank;
!     MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
!     std::cout<<"mpi_rank="<<mpi_rank<<std::endl;

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

!     //各MPIプロセスにおける、gridの解像度を取得
!     int nx = hydro.resol_x;
!     int ny = hydro.resol_x;
!     int nz = hydro.resol_x;
!     int nl = hydro.nleafs;
!     int resolution[4] = { nx, ny, nz, nl };
!     domain_parameters dom = {
!         hydro.global_min_coord.x(),
!         hydro.global_min_coord.y(),
!         hydro.global_min_coord.z(),
!         hydro.global_max_coord.x(),
!         hydro.global_max_coord.y(),
!         hydro.global_max_coord.z(),
!         resolution
!     };

!     if(mpi_rank==0)
!     {
!         std::cout<<"nx="<<nx<<",ny="<<ny<<",nz="<<nz<<",nl="<<nl<<std::endl;
!         std::cout<<"value_size="<<nx * ny * nz * nl<<std::endl;
!     }

!     //入力用の多変量データを生成
!     static int time_step = 0;
!     std::vector<float> cell_length;
!     std::vector<float> leaf_min_coord;
!     const int nvariables = 4;
!     float** values = new float*[nvariables];
!     for( int i=0; i<nvariables; i++ )
!     {
!         const int values_size = nx * ny * nz * nl;
!         values[i] = new float[ values_size ];
!         std::fill(values[i], values[i] + values_size , 0.0 );
!     }

!     float min = 10;
!     float max = 10;

!     int cnt = 0;
!     for (int l=0; l<nl; l++)
!     {
!         cell_length.push_back( hydro.cellLength( l ) );

!         const kvs::Vector3f lm( hydro.leafMinCoord( l ) );
!         leaf_min_coord.push_back( lm.x() );
!         leaf_min_coord.push_back( lm.y() );
!         leaf_min_coord.push_back( lm.z() );
!         //if(mpi_rank==0) std::cout<<"("<<hydro.cellLength( l )<<","<<lm<<")";

!         for (int k=0; k<nz; k++ )
!         {
!             for (int j=0; j<ny; j++ )
!             {
!                 for (int i=0; i<nx; i++)
!                 {
!                     values[0][cnt] = hydro.value( hydro.gridCoord( i, j, k, l ) );
!                     values[1][cnt] = hydro.value( hydro.gridCoord( i, j, k, l ) );
!                     values[2][cnt] = hydro.value( hydro.gridCoord( i, j, k, l ) );
!                     values[3][cnt] = hydro.value( hydro.gridCoord( i, j, k, l ) );
!                     cnt++;

!                     float tmp = hydro.value( hydro.gridCoord( i, j, k, l ) );
!                     min = min < tmp ? min : tmp;
!                     max = max > tmp ? max : tmp;
!                 }//end of for i
!             }//end of for j
!         }//end of for k
!     }//end of for l

!     if(mpi_rank==0) std::cout<<std::endl<<"min="<<min<<",max="<<max<<std::endl;
!     generate_particles( time_step, dom,
!                         cell_length, leaf_min_coord,
!                         nvariables, (const float**)values );

!     time_step++;

!     for( int i=0; i<nvariables; i++ )
!     {
!         delete [] values[i];
!     }
!     delete [] values;
! }

! /*===========================================================================*/
! /**
!  *  @brief  Main function
!  *  @param  argc [in] argument count
!  *  @param  argv [in] argument values
!  */
! /*===========================================================================*/
! int main( int argc, char** argv )
! {
!     MPI_Init( &argc, &argv );

!     // Simulation loop
!     while( 1 )
!     {
!         //Simulation();
!         Output();
!     }

!     MPI_Finalize();

!     return 0 ;
! }
