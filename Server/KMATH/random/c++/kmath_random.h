
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

/**
 *
 * Class    KMATH_Random
 * \brief   Random number class
 * \authors NT
 * \date    2013/02/04 (NT)
 *
 */
class KMATH_Random
{
 public:
  /**
   *
   * Function    Constructor
   * \authors    NT
   * \date       2013/02/04 (NT)
   *
   */
  KMATH_Random();

  /**
   *
   * Function    Destructor
   * \authors    NT
   * \date       2013/02/04 (NT)
   *
   */
  ~KMATH_Random();

  /**
   *
   * Function    init
   * \brief      Initialize random number system
   * \authors    NT
   * \param[in]  comm      communicator
   * \return     error information (true:No error)
   * \date       2013/02/04 (NT)
   *
   */
  bool        init(MPI_Comm comm);

  /**
   *
   * Function    finalize
   * \brief      Finalize random number system
   * \authors    NT
   * \return     error information (true:No error)
   * \date       2013/02/04 (NT)
   *
   */
  bool        finalize();

  /**
   *
   * Function    seed
   * \brief      Setup random seed
   * \authors    NT
   * \param[in]  seed     seed value
   * \return     error information (true:No error)
   * \date       2013/02/04 (NT)
   *
   */
  bool        seed(int seed);

  /**
   *
   * Function    get
   * \brief      Get a random value
   * \authors    NT
   * \param[out] value    random value
   * \return     error information (true:No error)
   * \date       2013/02/04 (NT)
   *
   */
  bool        get(double& value) const;

  /**
   *
   * Function    get
   * \brief      Get random values
   * \authors    NT
   * \param[out] values   random values
   * \param[in]  nvalue   number of values
   * \return     error information (true:No error)
   * \date       2013/02/04 (NT)
   *
   */
  bool        get(double* value, int size) const;

 private:
  bool        _isInit;
  MPI_Comm    _comm;
  int         _comm_rank;
  int         _comm_size;
  void*       _dsfmt;
};

#endif // !__kmath_random_h__
