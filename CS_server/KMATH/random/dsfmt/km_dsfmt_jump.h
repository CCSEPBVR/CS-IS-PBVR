
/**
 *
 * \file     km_dsfmt_jump.h
 * \brief    interface between dSFMT-jump and external codes (C,C++,FORTRAN)
 * \authoers Toshiyuki Imamura (TI)
 * \date     2013/02/11 (NT)
 * 
 * (c) Copyright 2013 RIKEN. All rights reserved.
 */

#ifndef __km_dsfmt_jump_h__
#define __km_dsfmt_jump_h__

#include "dSFMT.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

  /**
   *
   * Function    km_dsfmt_jump_init_
   * \brief      Initialize dSFMT-jump system
   * \authors    NT
   * \param[out] handle    handle for this random number
   * \date       2013/02/11 (NT)
   *
   */
  extern void km_dsfmt_jump_init_();

  /**
   *
   * Function    km_dsfmt_jump_do_
   * \brief      jump dSFMT random number using process informations
   * \authors    NT
   * \param[in]  handle    handle for this random number
   * \param[in]  iproc     process rank number
   * \param[in]  nproc     number of processes in group
   * \date       2013/02/11 (NT)
   *
   */
  extern void km_dsfmt_jump_do_(dsfmt_t* dsfmt, 
				const int* iproc, const int* nproc);

  
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif // !__km_dsfmt_jump_h__
