module kvs_wrapper_struct
  implicit none
  type domain_parameters 
     real*4 global_min_coord_x
     real*4 global_min_coord_y
     real*4 global_min_coord_z
     real*4 global_max_coord_x
     real*4 global_max_coord_y
     real*4 global_max_coord_z
     real*4 global_region_x
     real*4 global_region_y
     real*4 global_region_z
     integer resolution_x
     integer resolution_y
     integer resolution_z
     real*4 cell_length
  end type domain_parameters
contains

  subroutine generate_particles(time_step,dom,values,nvariables)
    implicit none
    integer time_step
    type(domain_parameters) :: dom 
    real*4 values(:,:)
    integer nvariables
    call generate_particles_wrap( &
         time_step, &
         dom%global_min_coord_x, &
         dom%global_min_coord_y, &
         dom%global_min_coord_z, &
         dom%global_max_coord_x, &
         dom%global_max_coord_y, &
         dom%global_max_coord_z, &
         dom%global_region_x, &
         dom%global_region_y, &
         dom%global_region_z, &
         dom%resolution_x, &
         dom%resolution_y, &
         dom%resolution_z, &
         dom%cell_length, &
         values, &
         nvariables)

  end subroutine generate_particles

end module kvs_wrapper_struct

module Hydrogen
  use kvs_wrapper_struct
  implicit none

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

    real*8 dim
    real*8 kr 
    real*8 kd 

    real*4 x,y,z
    real*8 dx,dy,dz

    real*8 r
    real*8 cos_theta
    real*8 phi
    real*8 c

    dim1 = resolution(1)
    dim2 = resolution(2)
    dim3 = resolution(3)

    dim= 128.0d0
    kr = 32.0d0 / dim
    kd = 6.0d0
    
    ! values = new float*[ nvariables ];
    ! for(int i=0; i<nvariables; i++)
    ! {
    !     values[i] = new float[dim1*dim2*dim3];
    ! }
    ! -----
    allocate(values(nvariables,dim1*dim2*dim3))

    !    kvs::UInt64 index = 0;
    index = 0

    ! for ( k = 0; k < dim3; ++k )
    ! {
    !     for ( j = 0; j < dim2; ++j )
    !     {
    !         for ( i = 0; i < dim1; ++i )
    !         {
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
             !                 c = (phi * phi) > 255 ? 255 : (phi * phi);
             c =min((phi * phi),255.0d0)
             
             values(0+1,index+1) = real( c )
             values(1+1,index+1) = real( c )
             values(2+1,index+1) = real( c )
             values(3+1,index+1) = real( c )
             !                index++;
             index=index+1
          enddo
       enddo
    enddo

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
    dom%global_region_x    = global_region(1,mpi_rank+1)
    dom%global_region_y    = global_region(2,mpi_rank+1)
    dom%global_region_z    = 0.0
    dom%resolution_x       = resolution(1)
    dom%resolution_y       = resolution(2)
    dom%resolution_z       = resolution(3)
    dom%cell_length        = cell_length

  end subroutine Hydrogen_init

  subroutine Hydrogen_free()
    implicit none
    deallocate(values)
  end subroutine Hydrogen_free

  subroutine Hydro_show
    implicit none

    ! std::cout<<"-------show volume data------"<<std::endl;
    ! std::cout<<"mpi_rank="<<mpi_rank<<std::endl;
    ! std::cout<<"resolution="<<resolution<<std::endl;
    ! std::cout<<"nvariables="<<nvariables<<std::endl;
    ! std::cout<<"global_region="<<global_region[mpi_rank]<<std::endl;

    ! float value_min = 17;
    ! float value_max = 17;
    ! for(int i=0; i< resolution.x()*resolution.y()*resolution.z(); i++)
    ! {
    !     value_min = value_min < values[0][i] ? value_min : values[0][i];
    !     value_max = value_max > values[0][i] ? value_max : values[0][i];
    ! }
    ! std::cout<<"value min = "<<value_min<<std::endl;
    ! std::cout<<"value max = "<<value_max<<std::endl;
    ! std::cout<<"-------end show volume data------"<<std::endl;
    ! -----

    real value_min
    real value_max
    integer i
    write(*,*)"-------show volume data------"
    write(*,*)"mpi_rank=",mpi_rank
    write(*,*)"resolution=",resolution
    write(*,*)"nvariables=",nvariables
    write(*,*)"global_region=",&
         global_region(1,mpi_rank+1),global_region(2,mpi_rank+1)

    value_min=17
    value_max=17
    do i=0,resolution(1)*resolution(2)*resolution(3)-1
       value_min = min(value_min,values(0+1,i+1))
       value_max = max(value_max,values(0+1,i+1))
    enddo
    write(*,*)"value min = ",value_min
    write(*,*)"value max = ",value_max
    write(*,*)"-------end show volume data------"

  end subroutine Hydro_show

end module Hydrogen
