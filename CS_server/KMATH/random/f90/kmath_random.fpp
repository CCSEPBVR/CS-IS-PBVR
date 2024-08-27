
!--------1---------2---------3---------4---------5---------6---------7---------8
!
!  Module   kmath_random_mod
!> @brief   random number module
!! @authors Toshiyuki Imamura (TI)
!! @date    2013/02/06 (NT)
!
!  (c) Copyright 2013 RIKEN. All rights reserved.
!
!--------1---------2---------3---------4---------5---------6---------7---------8

module kmath_random_mod

  use mpi

  implicit none
  private


  ! public procedures
  !

  public :: KMATH_Random_Init
  public :: KMATH_Random_Finalize
  public :: KMATH_Random_Seed
  public :: KMATH_Random_Get
  public :: KMATH_Random_Vector


  ! global variables
  !

  type, public:: s_km_rand

    logical            :: init = .false.
    integer            :: comm
    integer            :: comm_rank
    integer            :: comm_size
    character, pointer :: dsfmt(:)

  end type s_km_rand

contains

  !======1=========2=========3=========4=========5=========6=========7=========8
  !
  !  Subroutine  KMATH_Random_Init
  !> @brief      Initialize random number system
  !! @authors    NT
  !! @param[out] handle    handle for this random number
  !! @param[in]  comm      communicator
  !! @param[out] ierr      error information
  !! @date       2013/02/06 (NT)
  !
  !======1=========2=========3=========4=========5=========6=========7=========8

  subroutine KMATH_Random_Init(handle, comm, ierr)

    ! formal arguments
    type(s_km_rand),  target, intent(inout) :: handle
    integer,                  intent(in)    :: comm
    integer,                  intent(out)   :: ierr

    ! local variables
    type(s_km_rand),  pointer :: h
    integer                   :: ierr2, isize


    h => handle

    ! check handle initialization
    !
    if (h%init) then
      ierr = -1
      return 
    end if


    ! initialize dSFMT-jump
    !
    call km_dsfmt_jump_init


    ! duplicate main communicator
    !
    call MPI_Comm_dup(comm, h%comm, ierr)
    if (ierr /= 0) &
      return 


    ! check the nprocs
    !
    call MPI_Comm_size(h%comm, h%comm_size, ierr)
    if (ierr /= 0) then
      call MPI_Comm_free(h%comm, ierr2)
      return 
    end if

    call MPI_Comm_rank(h%comm, h%comm_rank, ierr)
    if (ierr /= 0) then
      call MPI_Comm_free(h%comm, ierr2)
      return
    end if


    ! allocate dSFMT working memory
    !
    call km_size_of_dsfmt_t(isize)

    allocate(h%dsfmt(isize))


    h%init = .true.

    return

  end subroutine KMATH_Random_Init


  !======1=========2=========3=========4=========5=========6=========7=========8
  !
  !  Subroutine  KMATH_Random_Finalize
  !> @brief      Finalize random number system
  !! @authors    NT
  !! @param[in]  handle   handle for this random number
  !! @param[out] ierr     error information
  !! @date       2013/02/06 (NT)
  !
  !======1=========2=========3=========4=========5=========6=========7=========8

  subroutine KMATH_Random_Finalize(handle, ierr)

    ! formal arguments
    type(s_km_rand),  target, intent(inout) :: handle
    integer,                  intent(out)   :: ierr

    ! local variables
    type(s_km_rand),  pointer :: h


    h => handle

    if (.not. h%init) then
      ierr = -1
      return 
    end if


    ! deallocate dSFMT working memory
    !
    deallocate(h%dsfmt)


    ! free MPI communicator
    !
    call MPI_Comm_free(h%comm, ierr)


    h%init = .false.

    return

  end subroutine KMATH_Random_Finalize


  !======1=========2=========3=========4=========5=========6=========7=========8
  !
  !  Subroutine  KMATH_Random_Seed
  !> @brief      Setup random seed
  !! @authors    NT
  !! @param[in]  handle   handle for this random number
  !! @param[in]  seed     seed value
  !! @param[out] ierr     error information
  !! @date       2013/02/06 (NT)
  !
  !======1=========2=========3=========4=========5=========6=========7=========8

  subroutine KMATH_Random_Seed(handle, seed, ierr)

    ! formal arguments
    type(s_km_rand),  target, intent(in)  :: handle
    integer,                  intent(in)  :: seed
    integer,                  intent(out) :: ierr

    ! local variables
    type(s_km_rand),  pointer :: h


    h => handle

    if (.not. h%init) then
      ierr = -1
      return
    end if

    call km_dsfmt_init_gen_rand(h%dsfmt, seed)

    call km_dsfmt_jump_do      (h%dsfmt, h%comm_rank, h%comm_size)

    ierr = 0
    return

  end subroutine KMATH_Random_Seed


  !======1=========2=========3=========4=========5=========6=========7=========8
  !
  !  Subroutine  KMATH_Random_Get
  !> @brief      Get a random value
  !! @authors    NT
  !! @param[in]  handle   handle for this random number
  !! @param[out] value    random value
  !! @param[out] ierr     error information
  !! @date       2013/02/06 (NT)
  !
  !======1=========2=========3=========4=========5=========6=========7=========8

  subroutine KMATH_Random_Get(handle, value, ierr)

    ! formal arguments
    type(s_km_rand),  intent(in)  :: handle
    double precision, intent(out) :: value
    integer,          intent(out) :: ierr


    if (.not. handle%init) then
      ierr = -1
      return 
    end if

    call km_dsfmt_genrand_close1_open2(handle%dsfmt, value)

    ierr = 0
    return

  end subroutine KMATH_Random_Get


  !======1=========2=========3=========4=========5=========6=========7=========8
  !
  !  Subroutine  KMATH_Random_Vector
  !> @brief      Get random values
  !! @authors    NT
  !! @param[in]  handle   handle for this random number
  !! @param[out] values   random values
  !! @param[in]  nvalue   number of values
  !! @param[out] ierr     error information
  !! @date       2013/02/06 (NT)
  !
  !======1=========2=========3=========4=========5=========6=========7=========8

  subroutine KMATH_Random_Vector(handle, value, nvalue, ierr)

    ! formal arguments
    type(s_km_rand),  intent(in)    :: handle
    double precision, intent(inout) :: value(:)
    integer,          intent(in)    :: nvalue
    integer,          intent(out)   :: ierr


    if (.not. handle%init) then
      ierr = -1
      return 
    end if

    call km_dsfmt_fill_array_close1_open2(handle%dsfmt, value, nvalue)

    ierr = 0
    return

  end subroutine KMATH_Random_Vector


end module kmath_random_mod
