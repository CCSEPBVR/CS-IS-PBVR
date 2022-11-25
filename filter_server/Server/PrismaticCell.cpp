/****************************************************************************/
/**
 *  @file   PrismaticCell.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id$
 */
/****************************************************************************/
#include "PrismaticCell.h"


namespace pbvr
{
// template instantiation
template class PrismaticCell<char>;
template class PrismaticCell<unsigned char>;
template class PrismaticCell<short>;
template class PrismaticCell<unsigned short>;
template class PrismaticCell<int>;
template class PrismaticCell<unsigned int>;
template class PrismaticCell<long>;
template class PrismaticCell<unsigned long>;
template class PrismaticCell<float>;
template class PrismaticCell<double>;
} // end of namespace kvs
