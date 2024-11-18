/****************************************************************************/
/**
 *  @file HexahedralCell.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: HexahedralCell.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "HexahedralCell.h"


namespace kvs
{

// template instantiation
template class HexahedralCell<char>;
template class HexahedralCell<unsigned char>;
template class HexahedralCell<short>;
template class HexahedralCell<unsigned short>;
template class HexahedralCell<int>;
template class HexahedralCell<unsigned int>;
template class HexahedralCell<long>;
template class HexahedralCell<unsigned long>;
template class HexahedralCell<float>;
template class HexahedralCell<double>;

} // end of namespace kvs
