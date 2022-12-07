
/**
 *
 * \file     test.c
 * \brief    test of KMATH random C module
 * \authoers Toshiyuki Imamura (TI)
 * \date     2013/02/04 (NT)
 * 
 * (c) Copyright 2013 RIKEN. All rights reserved.
 */

#include "kmath_random.h"
#include <stdio.h>

int main(int argc, char** argv)
{
  int comm_rank, i;
  void* h;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

  h = KMATH_Random_init(MPI_COMM_WORLD);


  printf("sequential\n");

  KMATH_Random_seed(h, 1);

  for(i = 0; i < 10; ++i)
    {
      double v;
      KMATH_Random_get(h, &v);
      printf("Rank:%d I:%d V:%f\n", comm_rank, i, v);
    }

  printf("vector\n");

  double va[1000];
  KMATH_Random_seed(h, 1);
  KMATH_Random_vector(h, va, 384);

  for(i = 0; i < 10; ++i)
    {
      printf("Rank:%d I:%d V:%f\n", comm_rank, i, va[i]);
    }

  KMATH_Random_finalize(h);

  MPI_Finalize();

  return 0;
}
