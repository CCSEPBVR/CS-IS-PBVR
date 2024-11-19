/****************************************************************************/
/**
 *  @file RGBAColor.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RGBAColor.h 869 2011-07-25 05:11:49Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__RGBA_COLOR_H_INCLUDE
#define VIS_MODULE__RGBA_COLOR_H_INCLUDE

#include "RGBColor.h"
#include <vismodule/Type>
#include <vismodule/ClassName>


namespace vismodule
{

/*==========================================================================*/
/**
 *  RGB-a color class.
 */
/*==========================================================================*/
class RGBAColor : public vismodule::RGBColor
{
    visModuleClassName_without_virtual( vismodule::RGBAColor );

protected:

    float m_opacity; ///< opacity [0-1]

public:

    RGBAColor( float opacity = 1.0f );

    RGBAColor( vismodule::UInt8 red, vismodule::UInt8 greeb, vismodule::UInt8 blue, float opacity = 1.0f );

    RGBAColor( const vismodule::UInt8 rgb[3], float opacity = 1.0f );

    RGBAColor( const vismodule::RGBColor& rgb, float opacity = 1.0f );

public:

    RGBAColor& operator = ( const RGBAColor& rgba );

    RGBAColor& operator = ( const RGBColor& rgb );

public:

    const float a( void ) const;

    const float alpha( void ) const;

    const float opacity( void ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__RGBA_COLOR_H_INCLUDE
