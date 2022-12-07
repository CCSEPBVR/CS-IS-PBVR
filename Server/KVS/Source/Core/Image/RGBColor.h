/****************************************************************************/
/**
 *  @file RGBColor.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: RGBColor.h 869 2011-07-25 05:11:49Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__RGB_COLOR_H_INCLUDE
#define KVS__RGB_COLOR_H_INCLUDE

#include <iostream>
#include <iomanip>
#include <kvs/Math>
#include <kvs/Type>
#include <kvs/ClassName>


namespace kvs
{

class HSVColor;
class RGBAColor;

/*==========================================================================*/
/**
 *  RGB color class.
 */
/*==========================================================================*/
class RGBColor
{
    kvsClassName_without_virtual( kvs::RGBColor );

protected:

    kvs::UInt8 m_red;   ///< red   [0-255]
    kvs::UInt8 m_green; ///< green [0-255]
    kvs::UInt8 m_blue;  ///< blue  [0-255]

public:

    RGBColor( void );

    RGBColor( kvs::UInt8 red, kvs::UInt8 green, kvs::UInt8 blue );

    RGBColor( const kvs::UInt8 rgb[3] );

    RGBColor( const RGBColor& rgb );

    RGBColor( const HSVColor& hsv );

public:

    RGBColor& operator += ( const RGBColor& rgb );

    RGBColor& operator -= ( const RGBColor& rgb );

    RGBColor& operator = ( const RGBColor& rgb );

    RGBColor& operator = ( const RGBAColor& rgba );

    RGBColor& operator = ( const HSVColor& hsv );

public:

    friend bool operator == ( const RGBColor& a, const RGBColor& b )
    {
        return( ( a.m_red   == b.m_red   ) &&
                ( a.m_green == b.m_green ) &&
                ( a.m_blue  == b.m_blue  ) );
    }

    friend RGBColor operator + ( const RGBColor& a, const RGBColor& b )
    {
        RGBColor ret( a ); ret += b;
        return( ret );
    }

    friend RGBColor operator - ( const RGBColor& a, const RGBColor& b )
    {
        RGBColor ret( a ); ret -= b;
        return( ret );
    }

    template <typename T>
    friend RGBColor operator * ( const T a, const RGBColor& rgb )
    {
        return( RGBColor( static_cast<kvs::UInt8>( kvs::Math::Round( a * rgb.r() ) ),
                          static_cast<kvs::UInt8>( kvs::Math::Round( a * rgb.g() ) ),
                          static_cast<kvs::UInt8>( kvs::Math::Round( a * rgb.b() ) ) ) );
    }

    template <typename T>
    friend RGBColor operator * ( const RGBColor& rgb, const T a )
    {
        return( RGBColor( static_cast<kvs::UInt8>( kvs::Math::Round( a * rgb.r() ) ),
                          static_cast<kvs::UInt8>( kvs::Math::Round( a * rgb.g() ) ),
                          static_cast<kvs::UInt8>( kvs::Math::Round( a * rgb.b() ) ) ) );
    }

    template <typename T>
    friend RGBColor operator / ( const RGBColor& rgb, const T a )
    {
        return( RGBColor( static_cast<kvs::UInt8>( kvs::Math::Round( rgb.r() / a ) ),
                          static_cast<kvs::UInt8>( kvs::Math::Round( rgb.g() / a ) ),
                          static_cast<kvs::UInt8>( kvs::Math::Round( rgb.b() / a ) ) ) );
    }

    friend std::ostream& operator << ( std::ostream& os, const RGBColor& rgb )
    {
        const size_t width     = 8;
        const size_t precision = 3;

        const std::ios_base::fmtflags original_flags( os.flags() );

        os.setf( std::ios::fixed );
        os.setf( std::ios::showpoint );

        os << std::setw(width) << std::setprecision(precision) << (int)rgb.r() << " ";
        os << std::setw(width) << std::setprecision(precision) << (int)rgb.g() << " ";
        os << std::setw(width) << std::setprecision(precision) << (int)rgb.b();

        os.flags( original_flags );

        return( os );
    }

public:

    void set( kvs::UInt8 red, kvs::UInt8 green, kvs::UInt8 blue );

public:

    const kvs::UInt8 r( void ) const;

    const kvs::UInt8 red( void ) const;

    const kvs::UInt8 g( void ) const;

    const kvs::UInt8 green( void ) const;

    const kvs::UInt8 b( void ) const;

    const kvs::UInt8 blue( void ) const;
};

} // end of namespace kvs

#endif // KVS__RGB_COLOR_H_INCLUDE
