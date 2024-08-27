/****************************************************************************/
/**
 *  @file TriangleCell.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: TriangleCell.cpp 608 2010-09-13 06:55:48Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "TriangleCell.h"


namespace pbvr
{

// template instantiation
template class TriangleCell<char>;
template class TriangleCell<unsigned char>;
template class TriangleCell<short>;
template class TriangleCell<unsigned short>;
template class TriangleCell<int>;
template class TriangleCell<unsigned int>;
template class TriangleCell<long>;
template class TriangleCell<unsigned long>;
template class TriangleCell<float>;
template class TriangleCell<double>;

} // end of namespace pbvr
