module kvs_wrapper_AMR
  implicit none
  type domain_parameters 
     real*4 global_min_coord_x
     real*4 global_min_coord_y
     real*4 global_min_coord_z
     real*4 global_max_coord_x
     real*4 global_max_coord_y
     real*4 global_max_coord_z
     integer resolution(4)
  end type domain_parameters
contains
  subroutine generate_particles( &
       time_step, dom, &
       cell_length, leaf_min_coord, &
       nvariables,values )
    implicit none
    integer time_step
    type(domain_parameters) :: dom 
    real*4 cell_length(:)
    real*4 leaf_min_coord(:)
    integer nvariables 
    real*4 values(:,:)

    integer nl
    integer values_size

    nl=size(cell_length)
    values_size=size(values)/nvariables
    call generate_particles_wrap( &
         time_step,&
         ! ------
         dom%global_min_coord_x, &
         dom%global_min_coord_y, &
         dom%global_min_coord_z, &
         dom%global_max_coord_x, &
         dom%global_max_coord_y, &
         dom%global_max_coord_z, &
         dom%resolution, &
         ! ------
         cell_length, leaf_min_coord, &
         nvariables,values,&
         nl,values_size)
! -----
! cell_length    --> nl
! leaf_min_coord --> nl*ndim
! values         --> values_size 

  end subroutine generate_particles

end module kvs_wrapper_AMR

module Hydrogen
  use kvs_wrapper_AMR
  implicit none
!    kvs::Vector3f global_min_coord;
  real*4 global_min_coord(3)
!    kvs::Vector3f global_max_coord;
  real*4 global_max_coord(3)

    ! // num. of leafs in each MPI region
    integer nleafs

    ! // x resolution of each leaf, 4x4x4
    integer resol_x

    ! // Leaf topology at each process
    ! //  Lv.1; 8x8x8; resol_lv1
    ! //  Lv.2; 16x16x16; resol_lv2
    ! //  Lv.2 on Lv.1
    integer resol_lv1
    integer resol_lv2
    real*4 leaf_length_lv1
    real*4 leaf_length_lv2
    real*4 cell_length_lv1
    real*4 cell_length_lv2

    ! // global x-y coordinates of each MPI region
    ! kvs::Vector2f global_region[4];
    real*4 global_region(2,4)
    integer mpi_rank

!    type(domain_parameters) :: dom 

contains
  subroutine Hydrogen_init
    use mpi
    implicit none
    integer ierr
    ! global_min_coord = kvs::Vector3f( 0.0, 0.0, 0.0 );
    global_min_coord(1)=0.0
    global_min_coord(2)=0.0
    global_min_coord(3)=0.0
    ! global_max_coord = kvs::Vector3f( 128.0, 128.0, 128.0 );
    global_max_coord(1) = 128.0
    global_max_coord(2) = 128.0
    global_max_coord(3) = 128.0
    resol_x = 5
    resol_lv1 = 8
    resol_lv2 = 16
    leaf_length_lv1 = 8.0
    leaf_length_lv2 = 4.0
    cell_length_lv1 = leaf_length_lv1 / (resol_x-1)
    cell_length_lv2 = leaf_length_lv2 / (resol_x-1)   
    nleafs = &
         resol_lv1 * resol_lv1 * resol_lv1 + & 
         resol_lv2 * resol_lv2 * resol_lv2

    ! global_region[ 0 ] = kvs::Vector2f(  0,  0 );
    global_region(1,0+1)=0.0
    global_region(2,0+1)=0.0
    ! global_region[ 1 ] = kvs::Vector2f( 64,  0 );
    global_region(1,1+1)=64.0
    global_region(2,1+1)=0.0
    ! global_region[ 2 ] = kvs::Vector2f( 64, 64 );
    global_region(1,2+1)=64.0
    global_region(2,2+1)=64.0
    ! global_region[ 3 ] = kvs::Vector2f(  0, 64 );
    global_region(1,3+1)=0.0
    global_region(2,3+1)=64.0
    
    ! MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    call MPI_Comm_rank(MPI_COMM_WORLD, mpi_rank,ierr)

    ! dom%global_min_coord_x = global_min_coord(1)
    ! dom%global_min_coord_y = global_min_coord(2)
    ! dom%global_min_coord_z = global_min_coord(3)
    ! dom%global_max_coord_x = global_max_coord(1)
    ! dom%global_max_coord_y = global_max_coord(2)
    ! dom%global_max_coord_z = global_max_coord(3)
    ! dom%resolution(1)=resol_x
    ! dom%resolution(2)=resol_x
    ! dom%resolution(3)=resol_x
    ! dom%resolution(4)=nleafs

  end subroutine Hydrogen_init

  subroutine hydro_cellLength(leaf_index,ret_cellLength)
    implicit none
    integer leaf_index
    real*4 ret_cellLength ! cellLengthの戻り値
    integer slice_lv1
    integer volum_lv1
    integer slice_lv2
    integer volum_lv2

    slice_lv1 = resol_lv1 * resol_lv1
    volum_lv1 = slice_lv1 * resol_lv1
    slice_lv2 = resol_lv2 * resol_lv2
    volum_lv2 = slice_lv2 * resol_lv2

 !    if( 0 <= leaf_index && leaf_index < volum_lv1 )
 !    {
 !    return cell_length_lv1;
 !    }
 ! else if( volum_lv1 <= leaf_index && leaf_index < (volum_lv1 + volum_lv2) )
 !    {
 !    return cell_length_lv2;
 !    }
 ! else
 !    {
 !    std::cout<<"cellLength error, leaf_index="<<leaf_index<<std::endl;
 !    return 0.0;
 !    }
    if( (0 <= leaf_index) .and. (leaf_index < volum_lv1) )then
       ret_cellLength=cell_length_lv1
    else if( (volum_lv1 <= leaf_index) .and. &
         (leaf_index < (volum_lv1 + volum_lv2) ) )then
       ret_cellLength=cell_length_lv2
    else
       write(*,*)"cellLength error, leaf_index=",leaf_index
       ret_cellLength=0.0
    endif
  end subroutine hydro_cellLength

  ! const kvs::Vector3f leafMinCoord( const int leaf_index )
  subroutine hydro_leafMinCoord(leaf_index,ret_leafMinCoord)
    implicit none
    integer leaf_index
    real*4 ret_leafMinCoord(3) ! leafMinCoordの戻り値
    integer slice_lv1
    integer volum_lv1             
    integer slice_lv2
    integer volum_lv2
    integer i,j,k
    real*4 x,y,z
    integer l_index

    slice_lv1 = resol_lv1 * resol_lv1
    volum_lv1 = slice_lv1 * resol_lv1

    slice_lv2 = resol_lv2 * resol_lv2
    volum_lv2 = slice_lv2 * resol_lv2

    ! if( 0 <= leaf_index && leaf_index < volum_lv1 )
    ! {
    !     k = leaf_index / slice_lv1;
    !     j = (leaf_index - k * slice_lv1) / resol_lv1;
    !     i = leaf_index - j * resol_lv1 - k * slice_lv1;

    !     x = (float)i * leaf_length_lv1 + global_region[mpi_rank].x();
    !     y = (float)j * leaf_length_lv1 + global_region[mpi_rank].y();
    !     z = (float)k * leaf_length_lv1;
    ! }
    ! else if( volum_lv1 <= leaf_index && leaf_index < (volum_lv1 + volum_lv2) )
    ! {
    !     int l_index = leaf_index - volum_lv1;
    !     k = l_index / slice_lv2;
    !     j = (l_index - k * slice_lv2) / resol_lv2;
    !     i = l_index - j * resol_lv2 - k * slice_lv2;

    !     x = (float)i * leaf_length_lv2 + global_region[mpi_rank].x();
    !     y = (float)j * leaf_length_lv2 + global_region[mpi_rank].y();
    !     z = (float)k * leaf_length_lv2;
    !     z += (float)resol_lv1  * leaf_length_lv1;
    ! }
    ! else
    ! {
    !     std::cout<<"leafMinCoord error, leaf_index="<<leaf_index<<std::endl;
    ! }
    if( (0 <= leaf_index) .and. (leaf_index < volum_lv1) )then
        k = leaf_index / slice_lv1
        j = (leaf_index - k * slice_lv1) / resol_lv1
        i = leaf_index - j * resol_lv1 - k * slice_lv1

        x = real(i) * leaf_length_lv1 + global_region(1,mpi_rank+1)
        y = real(j) * leaf_length_lv1 + global_region(2,mpi_rank+1)
        z = real(k) * leaf_length_lv1
    else if( ( volum_lv1 <= leaf_index) .and. & 
             ( leaf_index < (volum_lv1 + volum_lv2) ) )then
        l_index = leaf_index - volum_lv1
        k = l_index / slice_lv2
        j = (l_index - k * slice_lv2) / resol_lv2
        i = l_index - j * resol_lv2 - k * slice_lv2

        x = real(i) * leaf_length_lv2 + global_region(1,mpi_rank+1)
        y = real(j) * leaf_length_lv2 + global_region(2,mpi_rank+1)
        z = real(k) * leaf_length_lv2
        z = z + real(resol_lv1)  * leaf_length_lv1

    else
       write(*,*)"leafMinCoord error, leaf_index=",leaf_index
    endif

    ! return kvs::Vector3f( x, y, z);
    !     }
    ret_leafMinCoord(1)=x
    ret_leafMinCoord(2)=y
    ret_leafMinCoord(3)=z

  end subroutine hydro_leafMinCoord

  subroutine hydro_gridCoord(i,j,k,l,ret_gridCoord)
    implicit none
    integer i,j,k,l 
    real*4 ret_gridCoord(3)
    real*4 dx
    real*4 lm(3)
    real*4 x,y,z  

    ! const float dx = this->cellLength( l );
    call hydro_cellLength(l,dx)

    ! const kvs::Vector3f lm = this->leafMinCoord( l );
    call hydro_leafMinCoord(l,lm)

    ! const float x = i * dx + lm.x();
    x = i * dx + lm(1)

    ! const float y = j * dx + lm.y();
    y = j * dx + lm(2)

    ! const float z = k * dx + lm.z();   
    z = k * dx + lm(3)

    ! return kvs::Vector3f( x, y, z );
    ret_gridCoord(1)=x
    ret_gridCoord(2)=y
    ret_gridCoord(3)=z

  end subroutine hydro_gridCoord

  ! const float value( const kvs::Vector3f coord )
  subroutine hydro_value(coord,ret_value)
    implicit none
    real*4 coord(3)
    real*4 ret_value 
    real*4 x
    real*4 y
    real*4 z
    real*4 dim
    real*4 kr
    real*4 kd
    real*4 dx
    real*4 dy
    real*4 dz
    real*4 r
    real*4 cos_theta
    real*4 phi
    real*4 c

    x   = coord(1)
    y   = coord(2)
    z   = coord(3)

    dim = 128.0
    kr  = 32.0 / dim
    kd  = 6.0
    
    dx  = kr * ( x - ( dim / 2.0 ))
    dy  = kr * ( y - ( dim / 2.0 ))
    dz  = kr * ( z - ( dim / 2.0 ))
    
    r         = sqrt( dx * dx + dy * dy + dz * dz ) + 0.01
    cos_theta = dz / r
    phi       = kd * ( r*r ) * exp( -r/2 ) * ( 3*cos_theta*cos_theta-1 )
    ! c         = (phi * phi) > 255 ? 255 : (phi * phi)
    c         = min((phi * phi),255.0)
    
    ret_value=c

  end subroutine hydro_value


end module Hydrogen
