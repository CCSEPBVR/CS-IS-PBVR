/*****************************************************************************/
/**
 *  @file   SquareCell.cpp
 *  @author Takuma KAWAMURA
 *  @brief  Quadratic tetrahedral cell class.
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: QuadraticSquareCell.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "QuadraticSquareCell.h"


namespace pbvr
{

// template instantiation
template class QuadraticSquareCell<char>;
template class QuadraticSquareCell<unsigned char>;
template class QuadraticSquareCell<short>;
template class QuadraticSquareCell<unsigned short>;
template class QuadraticSquareCell<int>;
template class QuadraticSquareCell<unsigned int>;
template class QuadraticSquareCell<long>;
template class QuadraticSquareCell<unsigned long>;
template class QuadraticSquareCell<float>;
template class QuadraticSquareCell<double>;

} // end of namespace pbvr
