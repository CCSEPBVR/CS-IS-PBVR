/****************************************************************************/
/**
 *  @file Value.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: Value.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "Value.h"
#include <vismodule/Type>


namespace vismodule
{

// Template specialization.
template class Value<vismodule::Int8>;
template class Value<vismodule::UInt8>;
template class Value<vismodule::Int16>;
template class Value<vismodule::UInt16>;
template class Value<vismodule::Int32>;
template class Value<vismodule::UInt32>;
template class Value<vismodule::Int64>;
template class Value<vismodule::UInt64>;
template class Value<vismodule::Real32>;
template class Value<vismodule::Real64>;

} // end of namespace vismodule
