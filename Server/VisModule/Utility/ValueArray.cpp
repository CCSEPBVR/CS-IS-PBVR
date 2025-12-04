/****************************************************************************/
/**
 *  @file ValueArray.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ValueArray.cpp 837 2011-06-02 09:32:56Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#include "ValueArray.h"


namespace vismodule
{

// Template specialization.
template class ValueArray<vismodule::Int8>;
template class ValueArray<vismodule::UInt8>;
template class ValueArray<vismodule::Int16>;
template class ValueArray<vismodule::UInt16>;
template class ValueArray<vismodule::Int32>;
template class ValueArray<vismodule::UInt32>;
template class ValueArray<vismodule::Int64>;
template class ValueArray<vismodule::UInt64>;
template class ValueArray<vismodule::Real32>;
template class ValueArray<vismodule::Real64>;
template class ValueArray<std::string>;

} // end of namespace vismodule
