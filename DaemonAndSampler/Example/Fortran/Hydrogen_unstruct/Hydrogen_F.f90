module kvs_wrapper_unstruct
  implicit none
  type domain_parameters 
     real*4 global_min_coord_x
     real*4 global_min_coord_y
     real*4 global_min_coord_z
     real*4 global_max_coord_x
     real*4 global_max_coord_y
     real*4 global_max_coord_z
  end type domain_parameters
contains
  subroutine generate_particles(&
       time_step, dom, &
       values, nvariables, &
       coords, nnodes, &
       connections, ncells )

    implicit none
    integer time_step
    type(domain_parameters) :: dom 
    real*4 values(:,:)
    integer nvariables 
    real*4 coords(:)
    integer nnodes
    integer connections(:) 
    integer ncells
    integer num_element_node
    num_element_node = size(connections)/ncells
    call generate_particles_wrap( &
         time_step,  &
         dom%global_min_coord_x, &
         dom%global_min_coord_y, &
         dom%global_min_coord_z, &
         dom%global_max_coord_x, &
         dom%global_max_coord_y, &
         dom%global_max_coord_z, &
         values  , nvariables, &
         coords  , nnodes, &
         connections, ncells, &
         num_element_node)

  end subroutine generate_particles
end module kvs_wrapper_unstruct

module Hydrogen
  use kvs_wrapper_unstruct
  implicit none

  ! const kvs::Int32 num_dimentions = 3;
  ! const kvs::Int32 num_element_node = 8;
  integer,parameter :: num_dimentions=3
  integer,parameter :: num_element_node=8

  ! kvs::Vector3i resolution;
  integer resolution(3)
  
  ! kvs::Vector2f  global_region[4];
  real*4 global_region(2,4)

  ! kvs::Vector3f  global_min_coord;
  real*4 global_min_coord(3)

  ! kvs::Vector3f  global_max_coord;
  real*4 global_max_coord(3)

  ! float          cell_length;
  real*4 cell_length

  ! int            mpi_rank;
  integer mpi_rank

  ! float**        values;//[nvariables][nnodes]
  real*4,allocatable :: values(:,:)
  ! unsigned int*  connections; -->  符号なし整数がサポートされていないため32bit整数で代用
  integer,allocatable :: connections(:) 
  ! float*         coords;
  real*4,allocatable :: coords(:)
  ! int            ncells;
  integer ncells
  ! int            nnodes;
  integer nnodes
  ! int            nvariables;
  integer nvariables 

  type(domain_parameters) :: dom 

contains
  subroutine generate_volume()
    implicit none
    ! kvs::UInt64 --> 符号なし整数がサポートされていないため64bit整数で代用
    ! kvs::Real64 --> real*8

    integer*8 dim1
    integer*8 dim2
    integer*8 dim3
    integer*8 k,j,i

    integer*8 index
    integer*8 coords_index 

    real*8 dim
    real*8 kr 
    real*8 kd 

    real*4 x,y,z
    real*8 dx,dy,dz

    real*8 r
    real*8 cos_theta
    real*8 phi
    real*8 c

    integer*8 line_size
    integer*8 slice_size
    integer*8 vertex_index
    integer*8 connection_index

    ! size_t --> dim[0-3]がinteger*8型なのでinteger*8として扱う
    integer*8 ix,iy,iz
    integer*8 local_vertex_index(8)
    dim1 = resolution(1)
    dim2 = resolution(2)
    dim3 = resolution(3)

    dim= 128.0d0
    kr = 32.0d0 / dim
    kd = 6.0d0
    ncells = ( dim1 - 1 ) * ( dim2 - 1 )  * ( dim3 - 1 )
    nnodes = dim1 *  dim2  * dim3

!     connections = new unsigned int[ ncells * num_element_node ];
    allocate(connections( ncells * num_element_node))
!     coords      = new float[ nnodes * num_dimentions ];
    allocate(coords( nnodes * num_dimentions ))
!     values      = new float*[ nvariables ];
!     for(int i=0; i<nvariables; i++)
!     {
!         values[i] = new float[nnodes];
!     }
    allocate(values(nvariables,nnodes))

    index = 0
    coords_index = 0

!     for ( kvs::UInt64 k = 0; k < dim3; ++k )
!     {
!         for ( kvs::UInt64 j = 0; j < dim2; ++j )
!         {
!             for ( kvs::UInt64 i = 0; i < dim1; ++i )
!             {

    do k = 0,dim3-1
       do j = 0,dim2-1
          do i = 0,dim1-1
             x = real(i) * cell_length + global_region(1,mpi_rank+1)
             y = real(j) * cell_length + global_region(2,mpi_rank+1)
             z = real(k) * cell_length

             dx = kr * ( x - ( dim / 2.0d0 ) )
             dy = kr * ( y - ( dim / 2.0d0 ) )
             dz = kr * ( z - ( dim / 2.0d0 ) )

             r   = sqrt( dx * dx + dy * dy + dz * dz ) + 0.01d0
             cos_theta = dz / r
             phi = kd * ( r*r ) * exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 )
             ! c = (phi * phi) > 255 ? 255 : (phi * phi);
             c =min((phi * phi),255.0d0)

             values(0+1,index+1) = real( c )
             values(1+1,index+1) = real( c )
             values(2+1,index+1) = real( c )
             values(3+1,index+1) = real( c )
!                 index++;
             index=index+1

!                 coords[ coords_index++ ] = x;
!                 coords[ coords_index++ ] = y;
!                 coords[ coords_index++ ] = z;
             coords( coords_index + 1 ) = x
             coords_index=coords_index+1
             coords( coords_index + 1 ) = y
             coords_index=coords_index+1
             coords( coords_index + 1 ) = z
             coords_index=coords_index+1
          enddo
       enddo
    enddo
    line_size  = int( dim1 )
    slice_size = int( dim1 * dim2 )
    vertex_index = 0
    connection_index = 0

!     for ( size_t z = 0; z < dim3-1; ++z )
!     {
!         for ( size_t y = 0; y < dim2-1; ++y )
!         {
!             for ( size_t x = 0; x < dim1-1; ++x )
!             {
    do iz=0,dim3-1 -1
       do iy=0,dim2-1 -1
          do ix=0,dim2-1 -1

!                 // Same Connection order to Structured grid
!                 const kvs::UInt64 local_vertex_index[8] =
!                 {
!                     vertex_index,
!                     vertex_index + 1,
!                     vertex_index + line_size + 1,
!                     vertex_index + line_size,
!                     vertex_index                 + slice_size,
!                     vertex_index + 1             + slice_size,
!                     vertex_index + line_size + 1 + slice_size,
!                     vertex_index + line_size     + slice_size
!                 };             
             local_vertex_index(1)=vertex_index
             local_vertex_index(2)=vertex_index + 1
             local_vertex_index(3)=vertex_index + line_size + 1
             local_vertex_index(4)=vertex_index + line_size
             local_vertex_index(5)=vertex_index                 + slice_size
             local_vertex_index(6)=vertex_index + 1             + slice_size
             local_vertex_index(7)=vertex_index + line_size + 1 + slice_size
             local_vertex_index(8)=vertex_index + line_size     + slice_size

!                 // Unstructued Hexahedral Connection order
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );

             connections( connection_index + 1 ) = int( local_vertex_index( 4 +1 ) )
             connection_index=connection_index+1
             connections( connection_index + 1 ) = int( local_vertex_index( 5 +1 ) )
             connection_index=connection_index+1
             connections( connection_index + 1 ) = int( local_vertex_index( 6 +1 ) )
             connection_index=connection_index+1
             connections( connection_index + 1 ) = int( local_vertex_index( 7 +1 ) )
             connection_index=connection_index+1
             connections( connection_index + 1 ) = int( local_vertex_index( 0 +1 ) )
             connection_index=connection_index+1
             connections( connection_index + 1 ) = int( local_vertex_index( 1 +1 ) )
             connection_index=connection_index+1
             connections( connection_index + 1 ) = int( local_vertex_index( 2 +1 ) )
             connection_index=connection_index+1
             connections( connection_index + 1 ) = int( local_vertex_index( 3 +1 ) )
             connection_index=connection_index+1
!                 vertex_index++;
!             }
             vertex_index=vertex_index+1
          enddo
!             vertex_index++;
!         }
          vertex_index=vertex_index+1
       enddo

!         vertex_index += line_size;
!     }
       vertex_index = vertex_index+line_size
    enddo
!     return( 1 );

  end subroutine generate_volume

  subroutine Hydrogen_init
    use mpi
    implicit none
    integer ierr
    nvariables = 4

    !// Including halo region in x-y boundary
    !    resolution = kvs::Vector3i( 65, 65, 128 )
    resolution(1) = 65
    resolution(2) = 65
    resolution(3) = 128

    cell_length = 1.0

!   global_region[ 0 ] = kvs::Vector2f(  0,  0 );
    global_region(1,1) = 0
    global_region(2,1) = 0

!   global_region[ 1 ] = kvs::Vector2f( 64,  0 );
    global_region(1,2) = 64
    global_region(2,2) = 0

!   global_region[ 2 ] = kvs::Vector2f( 64, 64 );
    global_region(1,3) = 64
    global_region(2,3) = 64

!   global_region[ 3 ] = kvs::Vector2f(  0, 64 );
    global_region(1,4) = 0
    global_region(2,4) = 64

!   global_min_coord = kvs::Vector3f( 0, 0, 0 );
    global_min_coord(1) = 0.0
    global_min_coord(2) = 0.0
    global_min_coord(3) = 0.0

!   global_max_coord = kvs::Vector3f( 128, 128, 127 );
    global_max_coord(1) = 128.0
    global_max_coord(2) = 128.0
    global_max_coord(3) = 127.0

    call MPI_Comm_rank(MPI_COMM_WORLD, mpi_rank,ierr)
    call generate_volume()
    dom%global_min_coord_x = global_min_coord(1)
    dom%global_min_coord_y = global_min_coord(2)
    dom%global_min_coord_z = global_min_coord(3)
    dom%global_max_coord_x = global_max_coord(1)
    dom%global_max_coord_y = global_max_coord(2)
    dom%global_max_coord_z = global_max_coord(3)

  end subroutine Hydrogen_init

  subroutine hydro_show
    implicit none
    integer con_min,con_max
    real*4 coord_min,coord_max
    real*4 value_min,value_max
    integer i
    write(*,*)"-------show volume data------"
    write(*,*)"mpi_rank=",mpi_rank
    write(*,*)"resolution=",resolution
    write(*,*)"nvariables=",nvariables
    write(*,*)"global_region=",&
         global_region(1,mpi_rank+1),global_region(2,mpi_rank+1)
    write(*,*)"ncells=",ncells
    write(*,*)"nnodes=",nnodes

    con_min = 17
    con_max = 17
    ! for(int i=0; i<ncells * num_element_node; i++)
    ! {
    !     con_min = con_min < connections[i] ? con_min : connections[i];
    !     con_max = con_max > connections[i] ? con_max : connections[i];
    ! }
    do i=0,ncells * num_element_node-1
       con_min = min(con_min,connections(i+1))
       con_max = max(con_max,connections(i+1))
    enddo
    write(*,*)"connection min = ",con_min
    write(*,*)"connection max = ",con_max

    coord_min = 17
    coord_max = 17
    ! for(int i=0; i<nnodes * num_dimentions; i++)
    ! {
    !     coord_min = coord_min < coords[i] ? coord_min : coords[i];
    !     coord_max = coord_max > coords[i] ? coord_max : coords[i];
    ! }
    do  i=0,nnodes * num_dimentions-1
       coord_min = min(coord_min,coords(i+1))
       coord_max = max(coord_max,coords(i+1))
    enddo
    write(*,*)"coord min = ",coord_min
    write(*,*)"coord max = ",coord_max

    value_min = 17
    value_max = 17
    ! for(int i=0; i< nnodes; i++)
    ! {
    !     value_min = value_min < values[0][i] ? value_min : values[0][i];
    !     value_max = value_max > values[0][i] ? value_max : values[0][i];
    ! }
    do i=0,nnodes-1
       value_min =min( value_min , values(0+1,i+1))
       value_max =max( value_max , values(0+1,i+1))  
    enddo

    write(*,*)"value min = ",value_min
    write(*,*)"value max = ",value_max
    write(*,*)"-------end show volume data------"

  end subroutine hydro_show

  subroutine Hydrogen_free
    implicit none
    deallocate(values)
    deallocate(connections)
    deallocate(coords)
  end subroutine Hydrogen_free

end module Hydrogen

! /*****************************************************************************/
! /**
!  *  @file   Hydrogen.cpp
!  *  @author Naohisa Sakamoto
!  */
! /*----------------------------------------------------------------------------
!  *
!  *  Copyright (c) Visualization Laboratory, Kyoto University.
!  *  All rights reserved.
!  *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
!  *
!  *  $Id: Hydrogen.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
!  */
! /*****************************************************************************/
! #include "Hydrogen.h"
! #include <kvs/AnyValueArray>
! #include <kvs/ValueArray>
! #include <kvs/Vector3>



! const kvs::Int32 num_dimentions = 3;
! const kvs::Int32 num_element_node = 8;

! /*===========================================================================*/
! /**
!  *  @brief  Generates hydrogen volume.
!  *  @param  resolution [in] volume resolution
!  *  @return value array of the hydrogen volume
!  */
! /*===========================================================================*/
! int Hydrogen::generate_volume( void )
! {
!     const kvs::UInt64 dim1 = resolution.x();
!     const kvs::UInt64 dim2 = resolution.y();
!     const kvs::UInt64 dim3 = resolution.z();

!     const kvs::Real64 dim= 128.0;
!     const kvs::Real64 kr = 32.0 / dim;
!     const kvs::Real64 kd = 6.0;

!     ncells = ( dim1 - 1 ) * ( dim2 - 1 )  * ( dim3 - 1 );
!     nnodes = dim1 *  dim2  * dim3;

!     connections = new unsigned int[ ncells * num_element_node ];
!     coords      = new float[ nnodes * num_dimentions ];
!     values      = new float*[ nvariables ];

!     for(int i=0; i<nvariables; i++)
!     {
!         values[i] = new float[nnodes];
!     }

!     kvs::UInt64 index = 0;
!     kvs::UInt64 coords_index = 0;

!     for ( kvs::UInt64 k = 0; k < dim3; ++k )
!     {
!         for ( kvs::UInt64 j = 0; j < dim2; ++j )
!         {
!             for ( kvs::UInt64 i = 0; i < dim1; ++i )
!             {
!                 const float x = (float)i * cell_length + global_region[mpi_rank].x();
!                 const float y = (float)j * cell_length + global_region[mpi_rank].y();
!                 const float z = (float)k * cell_length;

!                 const kvs::Real64 dx = kr * ( x - ( dim / 2.0 ) );
!                 const kvs::Real64 dy = kr * ( y - ( dim / 2.0 ) );
!                 const kvs::Real64 dz = kr * ( z - ( dim / 2.0 ) );


!                 const kvs::Real64 r = std::sqrt( dx * dx + dy * dy + dz * dz ) + 0.01;
!                 const kvs::Real64 cos_theta = dz / r;
!                 const kvs::Real64 phi = kd * ( r*r ) * std::exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 );
!                 const kvs::Real64  c = (phi * phi) > 255 ? 255 : (phi * phi);

!                 values[0][index] = static_cast<float>( c );
!                 values[1][index] = static_cast<float>( c );
!                 values[2][index] = static_cast<float>( c );
!                 values[3][index] = static_cast<float>( c );
!                 index++;
!                 coords[ coords_index++ ] = x;
!                 coords[ coords_index++ ] = y;
!                 coords[ coords_index++ ] = z;
!             }
!         }
!     }

!     kvs::UInt64 line_size  = static_cast<kvs::UInt32>( dim1 );
!     kvs::UInt64 slice_size = static_cast<kvs::UInt32>( dim1 * dim2 );
!     kvs::UInt64 vertex_index = 0;
!     kvs::UInt64 connection_index = 0;
!     for ( size_t z = 0; z < dim3-1; ++z )
!     {
!         for ( size_t y = 0; y < dim2-1; ++y )
!         {
!             for ( size_t x = 0; x < dim1-1; ++x )
!             {
!                 // Same Connection order to Structured grid
!                 const kvs::UInt64 local_vertex_index[8] =
!                 {
!                     vertex_index,
!                     vertex_index + 1,
!                     vertex_index + line_size + 1,
!                     vertex_index + line_size,
!                     vertex_index                 + slice_size,
!                     vertex_index + 1             + slice_size,
!                     vertex_index + line_size + 1 + slice_size,
!                     vertex_index + line_size     + slice_size
!                 };

!                 // Unstructued Hexahedral Connection order
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 4 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 5 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 6 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 7 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 0 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 1 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 2 ] );
!                 connections[ connection_index++ ] = static_cast<unsigned int>( local_vertex_index[ 3 ] );

!                 vertex_index++;
!             }
!             vertex_index++;
!         }
!         vertex_index += line_size;
!     }
!     return( 1 );
! }


! /*===========================================================================*/
! /**
!  *  @brief  Constructs a new hydrogen class.
!  *  @param  resolution [in] volume resolution
!  */
! /*===========================================================================*/
! Hydrogen::Hydrogen( void )
! {
!     nvariables = 4;

!     // Including halo region in x-y boundary
!     resolution = kvs::Vector3ui( 65, 65, 128 );

!     cell_length = 1.0;

!     global_region[ 0 ] = kvs::Vector2f(  0,  0 );
!     global_region[ 1 ] = kvs::Vector2f( 64,  0 );
!     global_region[ 2 ] = kvs::Vector2f( 64, 64 );
!     global_region[ 3 ] = kvs::Vector2f(  0, 64 );

!     global_min_coord = kvs::Vector3f( 0, 0, 0 );
!     global_max_coord = kvs::Vector3f( 128, 128, 127 );

!     MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

!     this->generate_volume();
! }

! /*===========================================================================*/
! /**
!  *  @brief  Constructs a new hydrogen class.
!  *  @param  resolution [in] volume resolution
!  */
! /*===========================================================================*/
! void Hydrogen::show( void )
! {
!     std::cout<<"-------show volume data------"<<std::endl;
!     std::cout<<"mpi_rank="<<mpi_rank<<std::endl;
!     std::cout<<"resolution="<<resolution<<std::endl;
!     std::cout<<"nvariables="<<nvariables<<std::endl;
!     std::cout<<"global_region="<<global_region[mpi_rank]<<std::endl;
!     std::cout<<"ncells="<<ncells<<std::endl;
!     std::cout<<"nnodes="<<nnodes<<std::endl;

!     unsigned int con_min = 17;
!     unsigned int con_max = 17;
!     for(int i=0; i<ncells * num_element_node; i++)
!     {
!         con_min = con_min < connections[i] ? con_min : connections[i];
!         con_max = con_max > connections[i] ? con_max : connections[i];
!     }
!     std::cout<<"connection min = "<<con_min<<std::endl;
!     std::cout<<"connection max = "<<con_max<<std::endl;

!     float coord_min = 17;
!     float coord_max = 17;
!     for(int i=0; i<nnodes * num_dimentions; i++)
!     {
!         coord_min = coord_min < coords[i] ? coord_min : coords[i];
!         coord_max = coord_max > coords[i] ? coord_max : coords[i];
!     }
!     std::cout<<"coord min = "<<coord_min<<std::endl;
!     std::cout<<"coord max = "<<coord_max<<std::endl;

!     float value_min = 17;
!     float value_max = 17;
!     for(int i=0; i< nnodes; i++)
!     {
!         value_min = value_min < values[0][i] ? value_min : values[0][i];
!         value_max = value_max > values[0][i] ? value_max : values[0][i];
!     }
!     std::cout<<"value min = "<<value_min<<std::endl;
!     std::cout<<"value max = "<<value_max<<std::endl;
!     std::cout<<"-------end show volume data------"<<std::endl;
! }

! /*===========================================================================*/
! /**
!  *  @brief  Destructs the Hydrogen class.
!  */
! /*===========================================================================*/
! Hydrogen::~Hydrogen( void )
! {
!     for(int i=0; i<nvariables; i++)
!     {
!         delete [] values[i];
!     }

!     delete [] values;
!     delete [] connections;
!     delete [] coords;
! }
