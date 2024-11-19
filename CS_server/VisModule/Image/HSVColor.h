/****************************************************************************/
/**
 *  @file HSVColor.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: HSVColor.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__HSV_COLOR_H_INCLUDE
#define VIS_MODULE__HSV_COLOR_H_INCLUDE

#include <vismodule/Math>
#include <vismodule/ClassName>


namespace vismodule
{

class RGBColor;

/*==========================================================================*/
/**
 *  HSV color class.
 */
/*==========================================================================*/
class HSVColor
{
    visModuleClassName( vismodule::HSVColor );

protected:

    float m_hue;        ///< hue angle         [0-1]
    float m_saturation; ///< saturation        [0-1]
    float m_value;      ///< value (intensity) [0-1]

public:

    HSVColor( float hue = 0.0f, float saturation = 0.0f, float value = 0.0f );

    HSVColor( const HSVColor& hsv );

    HSVColor( const RGBColor& rgb );

public:

    HSVColor& operator += ( const HSVColor& hsv );

    HSVColor& operator -= ( const HSVColor& hsv );

    HSVColor& operator = ( const HSVColor& hsv );

    HSVColor& operator = ( const RGBColor& rgb );

public:

    friend bool operator == ( const HSVColor& a, const HSVColor& b )
    {
        return( vismodule::Math::Equal( a.h(), b.h() ) &&
                vismodule::Math::Equal( a.s(), b.s() ) &&
                vismodule::Math::Equal( a.v(), b.v() ) );
    }

    friend HSVColor operator + ( const HSVColor& a, const HSVColor& b )
    {
        HSVColor ret( a ); ret += b;
        return( ret );
    }

public:

    void set( float hue, float saturation, float value );

public:

    const float h( void ) const;

    const float hue( void ) const;

    const float s( void ) const;

    const float saturation( void ) const;

    const float v( void ) const;

    const float value( void ) const;

    const float intensity( void ) const;
};

} // end of namespace vismodule

#endif // VIS_MODULE__HSV_COLOR_H_INCLUDE
