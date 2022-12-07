
/**
 *
 * \file     kmath_random_if.c
 * \brief    interface of between dSFMT and Fortran code
 * \authoers Toshiyuki Imamura (TI)
 * \date     2013/02/11 (NT)
 * 
 * (c) Copyright 2013 RIKEN. All rights reserved.
 */

#include "../dsfmt/dSFMT.h"

void
km_size_of_dsfmt_t_(int* size)
{
  *size = sizeof(dsfmt_t);
}

void
km_dsfmt_init_gen_rand_(void* dsfmt, int* seed)
{
  dsfmt_init_gen_rand((dsfmt_t*)dsfmt, (uint32_t)*seed);
}

void
km_dsfmt_genrand_close1_open2_(void* dsfmt, double* value)
{
  *value = dsfmt_genrand_close1_open2((dsfmt_t*)dsfmt);
}

void
km_dsfmt_fill_array_close1_open2_(void* dsfmt, double* value, int* size)
{
  dsfmt_fill_array_close1_open2((dsfmt_t*)dsfmt, value, *size);
}
