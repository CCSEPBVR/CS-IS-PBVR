/****************************************************************************/
/**
 *  @file Quaternion.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Quaternion.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Quaternion.h"


namespace kvs
{

// template instantiation
template class Quaternion<int>;
template class Quaternion<float>;
template class Quaternion<double>;

} // end of namespace kvs
