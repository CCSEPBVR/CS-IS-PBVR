/****************************************************************************/
/**
 *  @file SquareCell.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: SquareCell.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "SquareCell.h"


namespace pbvr
{

// template instantiation
template class SquareCell<char>;
template class SquareCell<unsigned char>;
template class SquareCell<short>;
template class SquareCell<unsigned short>;
template class SquareCell<int>;
template class SquareCell<unsigned int>;
template class SquareCell<long>;
template class SquareCell<unsigned long>;
template class SquareCell<float>;
template class SquareCell<double>;

} // end of namespace pbvr
