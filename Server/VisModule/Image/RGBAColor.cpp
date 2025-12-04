/****************************************************************************/
/**
 *  @file RGBAColor.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RGBAColor.cpp 856 2011-07-15 00:05:12Z naohisa.sakamoto@gmail.com $
 */
/****************************************************************************/
#include "RGBAColor.h"
#include "RGBColor.h"


namespace vismodule
{

RGBAColor::RGBAColor( float opacity ):
    vismodule::RGBColor( 0, 0, 0 ),
    m_opacity( opacity )
{
}

RGBAColor::RGBAColor( vismodule::UInt8 red, vismodule::UInt8 green, vismodule::UInt8 blue, float opacity ):
    vismodule::RGBColor( red, green, blue ),
    m_opacity( opacity )
{
}

RGBAColor::RGBAColor( const vismodule::UInt8 rgb[3], float opacity ):
    vismodule::RGBColor( rgb[0], rgb[1], rgb[2] ),
    m_opacity( opacity )
{
}

RGBAColor::RGBAColor( const vismodule::RGBColor& rgb, float opacity ):
    vismodule::RGBColor( rgb ),
    m_opacity( opacity )
{
}

RGBAColor& RGBAColor::operator = ( const RGBAColor& rgba )
{
    m_red     = rgba.m_red;
    m_green   = rgba.m_green;
    m_blue    = rgba.m_blue;
    m_opacity = rgba.m_opacity;

    return( *this );
}

RGBAColor& RGBAColor::operator = ( const RGBColor& rgb )
{
    m_red     = rgb.r();
    m_green   = rgb.g();
    m_blue    = rgb.b();
    m_opacity = 1.0f;

    return( *this );
}

const float RGBAColor::a( void ) const
{
    return( m_opacity );
}

const float RGBAColor::alpha( void ) const
{
    return( m_opacity );
}

const float RGBAColor::opacity( void ) const
{
    return( m_opacity );
}

} // end of namespace vismodule
