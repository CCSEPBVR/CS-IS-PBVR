
/**
 *
 * \file     kmath_random.c
 * \brief    generate random number
 * \authoers Toshiyuki Imamura (TI)
 * \date     2013/02/04 (NT)
 * 
 * (c) Copyright 2013 RIKEN. All rights reserved.
 */

#include "kmath_random.h"
#include "../dsfmt/km_dsfmt_jump.h"
#include "../dsfmt/dSFMT.h"

#include <stdlib.h>


typedef struct _Context
{
  MPI_Comm comm;
  int      comm_rank;
  int      comm_size;
  dsfmt_t  dsfmt;

} Context;


inline void*
KMATH_Random_init(MPI_Comm comm)
{
  Context* c = (Context*)malloc(sizeof(Context));

  km_dsfmt_jump_init_();

  if (MPI_Comm_dup(comm, &c->comm) != 0)
    {
      free(c);
      return NULL;
    }

  if (MPI_Comm_rank(c->comm, &c->comm_rank) != 0 ||
      MPI_Comm_size(c->comm, &c->comm_size) != 0)
    {
      MPI_Comm_free(&c->comm);

      free(c);
      return NULL;
    }

  return (void*)c;
}


inline int
KMATH_Random_finalize(void* handle)
{
  if (handle == NULL)
    return -1;

  Context* c = (Context *)handle;

  MPI_Comm_free(&c->comm);

  free(c);

  return 0;
}


inline int
KMATH_Random_seed(void* handle, int seed)
{
  if (handle == NULL)
    return -1;

  Context* c = (Context *)handle;

  dsfmt_init_gen_rand(&c->dsfmt, (uint32_t)seed);

  km_dsfmt_jump_do_(&c->dsfmt, &c->comm_rank, &c->comm_size);

  return 0;
}


inline int
KMATH_Random_get(void* handle, double* value)
{
  if (handle == NULL)
    return -1;

  Context* c = (Context *)handle;

  *value = dsfmt_genrand_close1_open2(&c->dsfmt);
}


inline int
KMATH_Random_vector(void* handle, double* value, int size)
{
  if (handle == NULL)
    return -1;

  if (size < 384 || size % 2 != 0)
    return -2;

  Context* c = (Context *)handle;

  dsfmt_fill_array_close1_open2(&c->dsfmt, value, size);
}
