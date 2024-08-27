/****************************************************************************/
/**
 *  @file QuadraticTriangleCell.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: QuadraticTriangleCell.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "QuadraticTriangleCell.h"


namespace pbvr
{

// template instantiation
template class QuadraticTriangleCell<char>;
template class QuadraticTriangleCell<unsigned char>;
template class QuadraticTriangleCell<short>;
template class QuadraticTriangleCell<unsigned short>;
template class QuadraticTriangleCell<int>;
template class QuadraticTriangleCell<unsigned int>;
template class QuadraticTriangleCell<long>;
template class QuadraticTriangleCell<unsigned long>;
template class QuadraticTriangleCell<float>;
template class QuadraticTriangleCell<double>;

} // end of namespace pbvr
