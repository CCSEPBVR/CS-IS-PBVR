/****************************************************************************/
/**
 *  @file Vector4.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Vector4.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Vector4.h"


namespace kvs
{

// Template instantiation.
template class Vector4<char>;
template class Vector4<unsigned char>;
template class Vector4<short>;
template class Vector4<unsigned short>;
template class Vector4<int>;
template class Vector4<unsigned int>;
template class Vector4<long>;
template class Vector4<unsigned long>;
template class Vector4<float>;
template class Vector4<double>;

} // end of namespace kvs
