
!--------1---------2---------3---------4---------5---------6---------7---------8
!
!  Program  test
!> @brief   test of KMATH random module
!! @authors Toshiyuki Imamura (TI)
!! @date    2013/02/04 (NT)
!
!  (c) Copyright 2013 RIKEN. All rights reserved.
!
!--------1---------2---------3---------4---------5---------6---------7---------8
program test

  use kmath_random_mod
  use mpi

  implicit none


  ! local variables
  type(s_km_rand) :: rand
  integer :: ierr, i
  integer :: comm_rank
  double precision :: v, va(1000)


  call MPI_Init(ierr)
  call MPI_Comm_rank(MPI_COMM_WORLD, comm_rank, ierr)

  call KMATH_Random_Init(rand, MPI_COMM_WORLD, ierr)
  

  write(6,*) 'sequential'

  call KMATH_Random_Seed(rand, 1, ierr);

  do i = 1, 10
    call KMATH_Random_Get(rand, v, ierr)
    write(6,*) 'Rank:', comm_rank, ' I:',i, ' V:', v
  end do


  write(6,*) 'vector'

  call KMATH_Random_Seed(rand, 1, ierr);
  call KMATH_Random_Vector(rand, va, 384, ierr)

  do i = 1, 10
    write(6,*) 'Rank:', comm_rank, ' I:',i, ' V:', va(i)
  end do


  call KMATH_Random_Finalize(rand, ierr)

  call MPI_Finalize(ierr)

  stop

end program test
