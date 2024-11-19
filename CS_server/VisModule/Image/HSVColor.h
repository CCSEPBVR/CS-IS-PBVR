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
#ifndef KVS__HSV_COLOR_H_INCLUDE
#define KVS__HSV_COLOR_H_INCLUDE

#include <kvs/Math>
#include <kvs/ClassName>


namespace kvs
{

class RGBColor;

/*==========================================================================*/
/**
 *  HSV color class.
 */
/*==========================================================================*/
class HSVColor
{
    kvsClassName( kvs::HSVColor );

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
        return( kvs::Math::Equal( a.h(), b.h() ) &&
                kvs::Math::Equal( a.s(), b.s() ) &&
                kvs::Math::Equal( a.v(), b.v() ) );
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

} // end of namespace kvs

#endif // KVS__HSV_COLOR_H_INCLUDE
