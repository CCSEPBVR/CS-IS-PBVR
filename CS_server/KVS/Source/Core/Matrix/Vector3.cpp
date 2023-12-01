/****************************************************************************/
/**
 *  @file Vector3.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Vector3.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Vector3.h"


namespace kvs
{

// Template instantiation.
template class Vector3<char>;
template class Vector3<unsigned char>;
template class Vector3<short>;
template class Vector3<unsigned short>;
template class Vector3<int>;
template class Vector3<unsigned int>;
template class Vector3<long>;
template class Vector3<unsigned long>;
template class Vector3<float>;
template class Vector3<double>;

} // end of namespace kvs
