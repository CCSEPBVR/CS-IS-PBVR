
/**
 *
 * \file     kmath_random.h
 * \brief    generate random number
 * \authoers Toshiyuki Imamura (TI)
 * \date     2013/02/04 (NT)
 * 
 * (c) Copyright 2013 RIKEN. All rights reserved.
 */

#ifndef __kmath_random_h__
#define __kmath_random_h__

#include <mpi.h>

#if defined(__cplusplus)
extern "C" {
#endif

  /**
   *
   * Function    KMATH_Random_Init
   * \brief      Initialize random number system
   * \authors    NT
   * \param[in]  comm      communicator
   * \return     handle of this random number (0: initialization was failed)
   * \date       2013/02/04 (NT)
   *
   */
  extern void* KMATH_Random_init(MPI_Comm comm);

  /**
   *
   * Function    KMATH_Random_Finalize
   * \brief      Finalize random number system
   * \authors    NT
   * \param[in]  handle   handle for this random number
   * \return     error information (0:No error)
   * \date       2013/02/04 (NT)
   *
   */
  extern int KMATH_Random_finalize(void* handle);

  /**
   *
   * Function    KMATH_Random_Seed
   * \brief      Setup random seed
   * \authors    NT
   * \param[in]  handle   handle for this random number
   * \param[in]  seed     seed value
   * \return     error information (0:No error)
   * \date       2013/02/04 (NT)
   *
   */
  extern int KMATH_Random_seed(void* handle, int seed);

  /**
   *
   * Function    KMATH_Random_Get
   * \brief      Get a random value
   * \authors    NT
   * \param[in]  handle   handle for this random number
   * \param[out] value    random value
   * \return     error information (0:No error)
   * \date       2013/02/04 (NT)
   *
   */
  extern int KMATH_Random_get(void* handle, double* value);

  /**
   *
   * Function    KMATH_Random_Vector
   * \brief      Get random values
   * \authors    NT
   * \param[in]  handle   handle for this random number
   * \param[out] values   random values
   * \param[in]  nvalue   number of values
   * \return     error information (0:No error)
   * \date       2013/02/04 (NT)
   *
   */
  extern int KMATH_Random_vector(void* handle, double* value, int size);

#if defined(__cplusplus)
}
#endif

#endif // !__kmath_random_h__
