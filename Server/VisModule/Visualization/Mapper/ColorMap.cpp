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
#include <vismodule/Assert>
#include <vismodule/RGBColor>
#include <vismodule/HSVColor>
#include <vismodule/Math>


namespace
{

const std::size_t Resolution = 256;
const std::size_t NumberOfChannels = 3;

struct Equal
{
    float value;

    Equal( const float v ) : value( v ){}

    bool operator() ( const vismodule::ColorMap::Point& point ) const
    {
        return( vismodule::Math::Equal( point.first, value ) );
    }
};

struct Less
{
    bool operator() ( const vismodule::ColorMap::Point& p1, const vismodule::ColorMap::Point& p2 ) const
    {
        return( p1.first < p2.first );
    }
};

}

namespace vismodule
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
ColorMap::ColorMap( const std::size_t resolution ):
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
ColorMap::ColorMap( const std::size_t resolution, const float min_value, const float max_value ):
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
const std::size_t ColorMap::resolution( void ) const
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
    return( !vismodule::Math::Equal( m_min_value, m_max_value ) );
}

/*===========================================================================*/
/**
 *  @brief  Sets a table resolution.
 *  @param  resolution [in] table resolution
 */
/*===========================================================================*/
void ColorMap::setResolution( const std::size_t resolution )
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
void ColorMap::addPoint( const float value, const vismodule::RGBColor color )
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
    if ( vismodule::Math::IsZero( m_min_value ) && vismodule::Math::IsZero( m_max_value ) )
    {
        this->setRange( 0.0f, static_cast<float>( m_resolution - 1 ) );
    }
*/
    vismodule::Real32 min_value = 0.0f;
    vismodule::Real32 max_value = static_cast<vismodule::Real32>( m_resolution - 1 );
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

        vismodule::UInt8* color = m_table.pointer();
        for ( std::size_t i = 0; i < m_resolution; ++i )
        {
            // HSV to RGB
            const vismodule::HSVColor hsv(
                ( max_hue - increment * static_cast<float>( i ) ) / 360.0f,
                1.0f,
                1.0f );
            const vismodule::RGBColor rgb( hsv );

            *( color++ ) = rgb.red();
            *( color++ ) = rgb.green();
            *( color++ ) = rgb.blue();
        }

    }
    else
    {
        m_points.sort( ::Less() );

        const vismodule::RGBColor black( 0, 0, 0 );
        const vismodule::RGBColor white( 255, 255, 255 );
        if ( m_points.front().first > min_value ) this->addPoint( min_value, black );
        if ( m_points.back().first < max_value ) this->addPoint( max_value, white );

        const float stride = ( max_value - min_value ) / static_cast<float>( m_resolution - 1 );
        float f = min_value;
        for ( std::size_t i = 0; i < m_resolution; ++i, f += stride )
        {
            Points::iterator p = m_points.begin();
            Points::iterator last = m_points.end();

            vismodule::RGBColor color( 0, 0, 0 );
            Point p0( min_value, vismodule::RGBColor(   0,   0,   0 ) );
            Point p1( max_value, vismodule::RGBColor( 255, 255, 255 ) );
            while ( p != last )
            {
                const float s = p->first;
                if ( vismodule::Math::Equal( f, s ) )
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
                    const vismodule::RGBColor c0 = p0.second;
                    const vismodule::RGBColor c1 = p1.second;
                    const float r = c0.r() + ( c1.r() - c0.r() ) * ( f - s0 ) / ( s1 - s0 );
                    const float g = c0.g() + ( c1.g() - c0.g() ) * ( f - s0 ) / ( s1 - s0 );
                    const float b = c0.b() + ( c1.b() - c0.b() ) * ( f - s0 ) / ( s1 - s0 );
                    const vismodule::UInt8 R = static_cast<vismodule::UInt8>( r );
                    const vismodule::UInt8 G = static_cast<vismodule::UInt8>( g );
                    const vismodule::UInt8 B = static_cast<vismodule::UInt8>( b );
                    color = vismodule::RGBColor( R, G, B );
                    break;
                }
                else
                {
                    p0 = *p;
                    ++p;
                    if ( p == last )
                    {
                        if ( vismodule::Math::Equal( p0.first, max_value ) )
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
const vismodule::RGBColor ColorMap::operator []( const std::size_t index ) const
{
    VIS_MODULE_ASSERT( index < this->resolution() );

    const std::size_t offset = ::NumberOfChannels * index;
    return( vismodule::RGBColor( m_table.pointer() + offset ) );
}

/*===========================================================================*/
/**
 *  @brief  Returns interpolated RGB color value by assuming piecewise linear map.
 *  @param  value [in] value
 *  @return interpolated RGB color value
 */
/*===========================================================================*/
const vismodule::RGBColor ColorMap::at( const float value ) const
{
    if ( value <= m_min_value )
    {
        const vismodule::RGBColor color( m_table.pointer() );
        return( color );
    }
    else if ( value >= m_max_value )
    {
        const vismodule::RGBColor color( m_table.pointer() + ::NumberOfChannels * ( m_resolution - 1 ) );
        return( color );
    }

    const float r = static_cast<float>( m_resolution - 1 );
    const float v = ( value - m_min_value ) / ( m_max_value - m_min_value ) * r;
    const std::size_t s0 = static_cast<size_t>( v );
    const std::size_t s1 = s0 + 1;

    const vismodule::RGBColor c0( m_table.pointer() + ::NumberOfChannels * s0 );
    const vismodule::RGBColor c1( m_table.pointer() + ::NumberOfChannels * s1 );

    const int r0 = c0.r();
    const int g0 = c0.g();
    const int b0 = c0.b();
    const int r1 = c1.r();
    const int g1 = c1.g();
    const int b1 = c1.b();

    const vismodule::UInt8 R = static_cast<vismodule::UInt8>( ( r1 - r0 ) * v + r0 * s1 - r1 * s0 );
    const vismodule::UInt8 G = static_cast<vismodule::UInt8>( ( g1 - g0 ) * v + g0 * s1 - g1 * s0 );
    const vismodule::UInt8 B = static_cast<vismodule::UInt8>( ( b1 - b0 ) * v + b0 * s1 - b1 * s0 );

    return( vismodule::RGBColor( R, G, B ) );
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

} // end of namespace vismodule
