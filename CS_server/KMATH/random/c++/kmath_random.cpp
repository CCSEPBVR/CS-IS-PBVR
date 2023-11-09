
/**
 *
 * \file     kmath_random.cpp
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


KMATH_Random::KMATH_Random()
{
  _isInit    = false;
  _comm      = 0;
  _comm_rank = 0;
  _comm_size = 0;
  _dsfmt     = new dsfmt_t;
}

KMATH_Random::~KMATH_Random()
{
  finalize();

  delete (dsfmt_t*)_dsfmt;
}

bool
KMATH_Random::init(MPI_Comm comm)
{
  if (_isInit)
    return false;

  km_dsfmt_jump_init_();

  if (MPI_Comm_dup(comm, &_comm) != 0)
    return false;

  if (MPI_Comm_rank(_comm, &_comm_rank) != 0 ||
      MPI_Comm_size(_comm, &_comm_size) != 0)
    {
      MPI_Comm_free(&_comm);

      return false;
    }

  _isInit = true;
  return true;
}

bool
KMATH_Random::finalize()
{
  if (!_isInit)
    return false;

  MPI_Comm_free(&_comm);

  _isInit = false;
  return true;
}

bool
KMATH_Random::seed(int seed)
{
  if (!_isInit)
    return false;

  dsfmt_init_gen_rand((dsfmt_t*)_dsfmt, (uint32_t)seed);

  km_dsfmt_jump_do_((dsfmt_t*)_dsfmt, &_comm_rank, &_comm_size);

  return true;
}

bool
KMATH_Random::get(double& value) const
{
  if (!_isInit)
    return false;

  value = dsfmt_genrand_close1_open2((dsfmt_t*)_dsfmt);
  return true;
}

bool
KMATH_Random::get(double* value, int size) const
{
  if (!_isInit)
    return false;

  if (size < 384 || size % 2 != 0)
    return false;

  dsfmt_fill_array_close1_open2((dsfmt_t*)_dsfmt, value, size);
  return true;
}
