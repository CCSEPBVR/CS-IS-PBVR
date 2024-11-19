/*****************************************************************************/
/**
 *  @file   LookAtMatrix44.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: LookAtMatrix44.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "LookAtMatrix44.h"


namespace vismodule
{

template vismodule::Matrix44<float> LookAtMatrix44<float>(
    const vismodule::Vector3<float>& eye,
    const vismodule::Vector3<float>& up,
    const vismodule::Vector3<float>& target );

template vismodule::Matrix44<double> LookAtMatrix44<double>(
    const vismodule::Vector3<double>& eye,
    const vismodule::Vector3<double>& up,
    const vismodule::Vector3<double>& target );

} // end of namespace vismodule
