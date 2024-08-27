
/**
 *
 * \file     test2.cpp
 * \brief    test of KMATH random C++ module
 * \authoers Toshiyuki Imamura (TI)
 * \date     2013/02/06 (NT)
 * 
 * (c) Copyright 2013 RIKEN. All rights reserved.
 */

#include "kmath_random.h"
#include <stdio.h>

int main(int argc, char** argv)
{
  KMATH_Random rand;
  int comm_rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

  rand.init(MPI_COMM_WORLD);


  printf("sequential\n");

  rand.seed(1);

  for(int i = 0; i < 10; ++i)
    {
      double v;
      rand.get(v);
      printf("Rank:%d I:%d V:%f\n", comm_rank, i, v);
    }

  printf("vector\n");

  double va[1000];
  rand.seed(1);
  rand.get(va, 384);

  for(int i = 0; i < 10; ++i)
    {
      printf("Rank:%d I:%d V:%f\n", comm_rank, i, va[i]);
    }

  rand.finalize();

  MPI_Finalize();

  return 0;
}
