/****************************************************************************/
/**
 *  @file RGBColor.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RGBColor.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include <kvs/Math>
#include <kvs/Type>
#include "RGBColor.h"
#include "HSVColor.h"
#include "RGBAColor.h"


namespace kvs
{

RGBColor::RGBColor( void ):
    m_red( 0 ),
    m_green( 0 ),
    m_blue( 0 )
{
}

RGBColor::RGBColor( kvs::UInt8 red, kvs::UInt8 green, kvs::UInt8 blue ):
    m_red( red ),
    m_green( green ),
    m_blue( blue )
{
}

RGBColor::RGBColor( const kvs::UInt8 rgb[3] ):
    m_red( rgb[0] ),
    m_green( rgb[1] ),
    m_blue( rgb[2] )
{
}

RGBColor::RGBColor( const RGBColor& rgb ):
    m_red( rgb.m_red ),
    m_green( rgb.m_green ),
    m_blue( rgb.m_blue )
{
}

RGBColor::RGBColor( const HSVColor& hsv )
{
    *this = hsv;
}

RGBColor& RGBColor::operator += ( const RGBColor& rgb )
{
    m_red   += rgb.m_red;
    m_green += rgb.m_green;
    m_blue  += rgb.m_blue;
    return( *this );
}

RGBColor& RGBColor::operator -= ( const RGBColor& rgb )
{
    m_red   -= rgb.m_red;
    m_green -= rgb.m_green;
    m_blue  -= rgb.m_blue;
    return( *this );
}

RGBColor& RGBColor::operator = ( const RGBColor& rgb )
{
    m_red   = rgb.m_red;
    m_green = rgb.m_green;
    m_blue  = rgb.m_blue;
    return( *this );
}

/*==========================================================================*/
/**
 *  '=' operator.
 *
 *  @param rgba [in] RGB-a color
 */
/*==========================================================================*/
RGBColor& RGBColor::operator = ( const RGBAColor& rgba )
{
    m_red   = rgba.r();
    m_green = rgba.g();
    m_blue  = rgba.b();

    return( *this );
}

/*==========================================================================*/
/**
 *  '=' operator. ( convert HSV to RGB )
 *
 *  @param hsv [in] HSV color        
 */
/*==========================================================================*/
RGBColor& RGBColor::operator = ( const HSVColor& hsv )
{
    if( kvs::Math::IsZero( hsv.s() ) )
    {
        m_red   = static_cast<kvs::UInt8>( hsv.v() * 255 );
        m_green = static_cast<kvs::UInt8>( hsv.v() * 255 );
        m_blue  = static_cast<kvs::UInt8>( hsv.v() * 255 );
    }
    else
    {
        float h = ( hsv.h() < 1.0f ? hsv.h() : hsv.h() - 1.0f ) * 6.0f;
        int   i = int( h );

        float tmp1 = hsv.v() * ( 1 - hsv.s() );
        float tmp2 = hsv.v() * ( 1 - hsv.s() * ( h - i ) );
        float tmp3 = hsv.v() * ( 1 - hsv.s() * ( 1 - h + i ) );

        float tmp_r, tmp_g, tmp_b;
        switch( i )
        {
        case 0:
        {
            tmp_r = hsv.v();
            tmp_g = tmp3;
            tmp_b = tmp1;
            break;
        }
        case 1:
        {
            tmp_r = tmp2;
            tmp_g = hsv.v();
            tmp_b = tmp1;
            break;
        }
        case 2:
        {
            tmp_r = tmp1;
            tmp_g = hsv.v();
            tmp_b = tmp3;
            break;
        }
        case 3:
        {
            tmp_r = tmp1;
            tmp_g = tmp2;
            tmp_b = hsv.v();
            break;
        }
        case 4:
        {
            tmp_r = tmp3;
            tmp_g = tmp1;
            tmp_b = hsv.v();
            break;
        }
        default:
        {
            tmp_r = hsv.v();
            tmp_g = tmp1;
            tmp_b = tmp2;
            break;
        }
        }

        m_red   = static_cast<kvs::UInt8>( tmp_r * 255.0f + 0.5f );
        m_green = static_cast<kvs::UInt8>( tmp_g * 255.0f + 0.5f );
        m_blue  = static_cast<kvs::UInt8>( tmp_b * 255.0f + 0.5f );
    }

    return( *this );
}

void RGBColor::set( kvs::UInt8 red, kvs::UInt8 green, kvs::UInt8 blue )
{
    m_red   = red;
    m_green = green;
    m_blue  = blue;
}

const kvs::UInt8 RGBColor::r( void ) const
{
    return( m_red );
}

const kvs::UInt8 RGBColor::red( void ) const
{
    return( m_red );
}

const kvs::UInt8 RGBColor::g( void ) const
{
    return( m_green );
}

const kvs::UInt8 RGBColor::green( void ) const
{
    return( m_green );
}

const kvs::UInt8 RGBColor::b( void ) const
{
    return( m_blue );
}

const kvs::UInt8 RGBColor::blue( void ) const
{
    return( m_blue );
}

} // end of namespace kvs
