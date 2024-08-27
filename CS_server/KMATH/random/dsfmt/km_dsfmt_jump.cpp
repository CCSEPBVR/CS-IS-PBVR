
/**
 *
 * \file     km_dsfmt_jump.cpp
 * \brief    interface between dSFMT-jump and external codes (C,C++,FORTRAN)
 * \authoers Toshiyuki Imamura (TI)
 * \date     2013/02/11 (NT)
 * 
 * (c) Copyright 2013 RIKEN. All rights reserved.
 */

#include "km_dsfmt_jump.h"
#include "km_dsfmt_poly.h"

#include "dSFMT-calc-jump.hpp"
#include "dSFMT-jump.h"
#include "dSFMT.h"

#include <NTL/GF2X.h>
#include <NTL/ZZ.h>
#include <stdlib.h>
#include <stdio.h>


using namespace dsfmt;
using namespace NTL;
using namespace std;


#define DEF_RAND_RANGE  1000
#define MIN_RAND_RANGE  1
#define MAX_RAND_RANGE  km_dsfmt_period

ZZ  g_range;


extern "C" void
km_dsfmt_jump_init_()
{
  if (g_range == 0)
    {
      int range = DEF_RAND_RANGE;

      char* rangeStr = getenv("KMATH_RAND_RANGE");
      if (rangeStr != NULL)
	sscanf(rangeStr, "%d", &range);

      if (range < MIN_RAND_RANGE)
	range = MIN_RAND_RANGE;
      else if (range > MAX_RAND_RANGE)
	range = MAX_RAND_RANGE;

      g_range = 1;

      int i = 0;
      for(i = 0; i < range; ++i)
	g_range *= 2;
      if (range == MAX_RAND_RANGE)
	g_range --;
    }
}


extern "C" void
km_dsfmt_jump_do_(dsfmt_t* dsfmt, const int* iproc, const int* nproc)
{
  assert(dsfmt != NULL);
  
  GF2X poly;
  stringtopoly(poly, km_dsfmt_poly);

  ZZ range;
  range = g_range / long(*nproc);

  ZZ count;
  count = range * long(*iproc);

  string countStr;
  calc_jump(countStr, count, poly);

  dSFMT_jump(dsfmt, countStr.c_str());
}
