
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
  int comm_rank;
  void *h;
  long long ll;
  double v;
  

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

  h = KMATH_Random_init(MPI_COMM_WORLD);
  KMATH_Random_seed(h, 1);


  printf("10,000,000,000 loop\n");

  for(ll = 0LL ; ll < 10000000000LL; ++ll)
    KMATH_Random_get(h, &v);

  printf("  last value = %f\n", v);


  KMATH_Random_finalize(h);

  MPI_Finalize();

  return 0;
}
