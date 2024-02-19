/****************************************************************************/
/**
 *  @file ColorMap.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ColorMap.cpp 650 2010-10-22 07:06:13Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ColorMap.h"
#include <kvs/Assert>
#include <kvs/RGBColor>
#include <kvs/HSVColor>
#include <kvs/Math>


namespace
{

const size_t Resolution = 256;
const size_t NumberOfChannels = 3;

struct Equal
{
    float value;

    Equal( const float v ) : value( v ){}

    bool operator() ( const kvs::ColorMap::Point& point ) const
    {
        return( kvs::Math::Equal( point.first, value ) );
    }
};

struct Less
{
    bool operator() ( const kvs::ColorMap::Point& p1, const kvs::ColorMap::Point& p2 ) const
    {
        return( p1.first < p2.first );
    }
};

}

namespace kvs
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new ColorMap class.
 */
/*==========================================================================*/
ColorMap::ColorMap( void ):
    m_resolution( ::Resolution ),
    m_min_value( 0.0f ),
//    m_max_value( ::Resolution - 1.0f ),
    m_max_value( 0.0f ),
    m_points(),
    m_table()
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new ColorMap class.
 *  @param  resolution [in] resolution
 */
/*==========================================================================*/
ColorMap::ColorMap( const size_t resolution ):
    m_resolution( resolution ),
    m_min_value( 0.0f ),
//    m_max_value( resolution - 1.0f ),
    m_max_value( 0.0f ),
    m_points(),
    m_table()
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new ColorMap class.
 *  @param  table [in] opacity value table
 */
/*==========================================================================*/
ColorMap::ColorMap( const ColorMap::Table& table ):
    m_resolution( table.size() / 3 ),
    m_min_value( 0.0f ),
//    m_max_value( table.size() - 1.0f ),
    m_max_value( 0.0f ),
    m_points(),
    m_table( table )
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new ColoryMap class.
 *  @param  resolution [in] resolution
 *  @param  min_value [in] min value
 *  @param  max_value [in] max value
 */
/*==========================================================================*/
ColorMap::ColorMap( const size_t resolution, const float min_value, const float max_value ):
    m_resolution( resolution ),
    m_min_value( min_value ),
    m_max_value( max_value ),
    m_points(),
    m_table()
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new ColorMap class.
 *  @param  table [in] opacity value table
 *  @param  min_value [in] min value
 *  @param  max_value [in] max value
 */
/*==========================================================================*/
ColorMap::ColorMap( const ColorMap::Table& table, const float min_value, const float max_value ):
    m_resolution( table.size() / 3 ),
    m_min_value( min_value ),
    m_max_value( max_value ),
    m_points(),
    m_table( table )
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new ColorMap class.
 *  @param  color_map [in] color map
 */
/*==========================================================================*/
ColorMap::ColorMap( const ColorMap& other ):
    m_resolution( other.m_resolution ),
    m_min_value( other.m_min_value ),
    m_max_value( other.m_max_value ),
    m_points( other.m_points ),
    m_table( other.m_table )
{
}

/*==========================================================================*/
/**
 *  @brief  Destroys the OpacityMap class.
 */
/*==========================================================================*/
ColorMap::~ColorMap( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Returns the min value.
 *  @return min value
 */
/*===========================================================================*/
const float ColorMap::minValue( void ) const
{
    return( m_min_value );
}

/*===========================================================================*/
/**
 *  @brief  Returns the max value.
 *  @return max value
 */
/*===========================================================================*/
const float ColorMap::maxValue( void ) const
{
    return( m_max_value );
}

/*==========================================================================*/
/**
 *  Returns the resolution of the color map.
 *  @return resolusion
 */
/*==========================================================================*/
const size_t ColorMap::resolution( void ) const
{
    return( m_resolution );
}

/*===========================================================================*/
/**
 *  @brief  Returns the control point list.
 *  @return control point list
 */
/*===========================================================================*/
const ColorMap::Points& ColorMap::points( void ) const
{
    return( m_points );
}

/*==========================================================================*/
/**
 *  Returns the color map table.
 *  @return color map table
 */
/*==========================================================================*/
const ColorMap::Table& ColorMap::table( void ) const
{
    return( m_table );
}

const bool ColorMap::hasRange( void ) const
{
    return( !kvs::Math::Equal( m_min_value, m_max_value ) );
}

/*===========================================================================*/
/**
 *  @brief  Sets a table resolution.
 *  @param  resolution [in] table resolution
 */
/*===========================================================================*/
void ColorMap::setResolution( const size_t resolution )
{
    m_resolution = resolution;
}

/*===========================================================================*/
/**
 *  @brief  Sets min and max values.
 *  @param  min_value [in] min. value
 *  @param  max_value [in] max. value
 */
/*===========================================================================*/
void ColorMap::setRange( const float min_value, const float max_value )
{
    m_min_value = min_value;
    m_max_value = max_value;
}

/*===========================================================================*/
/**
 *  @brief  Adds a control point.
 *  @param  value [in] scalar value in [min_value, max_value]
 *  @param  color [in] color value
 */
/*===========================================================================*/
void ColorMap::addPoint( const float value, const kvs::RGBColor color )
{
    m_points.push_back( Point( value, color ) );
}

/*===========================================================================*/
/**
 *  @brief  Removes the constrol point associated with the given scalar value.
 *  @param  value [in] scalar value in [min_value, max_value]
 */
/*===========================================================================*/
void ColorMap::removePoint( const float value )
{
    m_points.remove_if( ::Equal( value ) );
}

/*==========================================================================*/
/**
 *  @brief  Creates the color map.
 */
/*==========================================================================*/
void ColorMap::create( void )
{
/*
    if ( kvs::Math::IsZero( m_min_value ) && kvs::Math::IsZero( m_max_value ) )
    {
        this->setRange( 0.0f, static_cast<float>( m_resolution - 1 ) );
    }
*/
    kvs::Real32 min_value = 0.0f;
    kvs::Real32 max_value = static_cast<kvs::Real32>( m_resolution - 1 );
    if ( this->hasRange() )
    {
        min_value = this->minValue();
        max_value = this->maxValue();
    }

    m_table.allocate( ::NumberOfChannels * m_resolution );
    if ( m_points.size() == 0 )
    {
        const float min_hue = 0.0f;   // blue
        const float max_hue = 240.0f; // red
        const float increment = ( max_hue - min_hue ) / static_cast<float>( m_resolution - 1 );

        kvs::UInt8* color = m_table.pointer();
        for ( size_t i = 0; i < m_resolution; ++i )
        {
            // HSV to RGB
            const kvs::HSVColor hsv(
                ( max_hue - increment * static_cast<float>( i ) ) / 360.0f,
                1.0f,
                1.0f );
            const kvs::RGBColor rgb( hsv );

            *( color++ ) = rgb.red();
            *( color++ ) = rgb.green();
            *( color++ ) = rgb.blue();
        }

    }
    else
    {
        m_points.sort( ::Less() );

        const kvs::RGBColor black( 0, 0, 0 );
        const kvs::RGBColor white( 255, 255, 255 );
        if ( m_points.front().first > min_value ) this->addPoint( min_value, black );
        if ( m_points.back().first < max_value ) this->addPoint( max_value, white );

        const float stride = ( max_value - min_value ) / static_cast<float>( m_resolution - 1 );
        float f = min_value;
        for ( size_t i = 0; i < m_resolution; ++i, f += stride )
        {
            Points::iterator p = m_points.begin();
            Points::iterator last = m_points.end();

            kvs::RGBColor color( 0, 0, 0 );
            Point p0( min_value, kvs::RGBColor(   0,   0,   0 ) );
            Point p1( max_value, kvs::RGBColor( 255, 255, 255 ) );
            while ( p != last )
            {
                const float s = p->first;
                if ( kvs::Math::Equal( f, s ) )
                {
                    color = p->second;
                    break;
                }
                else if ( f < s )
                {
                    p1 = *p;
                    // Interpolate.
                    const float s0 = p0.first;
                    const float s1 = p1.first;
                    const kvs::RGBColor c0 = p0.second;
                    const kvs::RGBColor c1 = p1.second;
                    const float r = c0.r() + ( c1.r() - c0.r() ) * ( f - s0 ) / ( s1 - s0 );
                    const float g = c0.g() + ( c1.g() - c0.g() ) * ( f - s0 ) / ( s1 - s0 );
                    const float b = c0.b() + ( c1.b() - c0.b() ) * ( f - s0 ) / ( s1 - s0 );
                    const kvs::UInt8 R = static_cast<kvs::UInt8>( r );
                    const kvs::UInt8 G = static_cast<kvs::UInt8>( g );
                    const kvs::UInt8 B = static_cast<kvs::UInt8>( b );
                    color = kvs::RGBColor( R, G, B );
                    break;
                }
                else
                {
                    p0 = *p;
                    ++p;
                    if ( p == last )
                    {
                        if ( kvs::Math::Equal( p0.first, max_value ) )
                        {
                            color = p0.second;
                        }
                    }
                }
            }

            m_table[ i * ::NumberOfChannels + 0 ] = color.r();
            m_table[ i * ::NumberOfChannels + 1 ] = color.g();
            m_table[ i * ::NumberOfChannels + 2 ] = color.b();
        }
    }
}

/*==========================================================================*/
/**
 *  @brief  Access operator [].
 *  @param  index [in] index of the color map.
 *  @retval RGB color value
 */
/*==========================================================================*/
const kvs::RGBColor ColorMap::operator []( const size_t index ) const
{
    KVS_ASSERT( index < this->resolution() );

    const size_t offset = ::NumberOfChannels * index;
    return( kvs::RGBColor( m_table.pointer() + offset ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns interpolated RGB color value by assuming piecewise linear map.
 *  @param  value [in] value
 *  @return interpolated RGB color value
 */
/*===========================================================================*/
const kvs::RGBColor ColorMap::at( const float value ) const
{
    if ( value <= m_min_value )
    {
        const kvs::RGBColor color( m_table.pointer() );
        return( color );
    }
    else if ( value >= m_max_value )
    {
        const kvs::RGBColor color( m_table.pointer() + ::NumberOfChannels * ( m_resolution - 1 ) );
        return( color );
    }

    const float r = static_cast<float>( m_resolution - 1 );
    const float v = ( value - m_min_value ) / ( m_max_value - m_min_value ) * r;
    const size_t s0 = static_cast<size_t>( v );
    const size_t s1 = s0 + 1;

    const kvs::RGBColor c0( m_table.pointer() + ::NumberOfChannels * s0 );
    const kvs::RGBColor c1( m_table.pointer() + ::NumberOfChannels * s1 );

    const int r0 = c0.r();
    const int g0 = c0.g();
    const int b0 = c0.b();
    const int r1 = c1.r();
    const int g1 = c1.g();
    const int b1 = c1.b();

    const kvs::UInt8 R = static_cast<kvs::UInt8>( ( r1 - r0 ) * v + r0 * s1 - r1 * s0 );
    const kvs::UInt8 G = static_cast<kvs::UInt8>( ( g1 - g0 ) * v + g0 * s1 - g1 * s0 );
    const kvs::UInt8 B = static_cast<kvs::UInt8>( ( b1 - b0 ) * v + b0 * s1 - b1 * s0 );

    return( kvs::RGBColor( R, G, B ) );
}

/*==========================================================================*/
/**
 *  @brief  Substitution operator =.
 *  @param  opacity_map [in] color map
 *  @retval opacity map
 */
/*==========================================================================*/
ColorMap& ColorMap::operator =( const ColorMap& rhs )
{
    m_resolution = rhs.m_resolution;
    m_min_value = rhs.m_min_value;
    m_max_value = rhs.m_max_value;
    m_points = rhs.m_points;
    m_table = rhs.m_table;

    return( *this );
}

} // end of namespace kvs
