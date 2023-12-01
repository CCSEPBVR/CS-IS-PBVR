
#include <kvs/HSVColor>
#include "ColorMapLibrary.h"
#include <algorithm>

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <math.h>
#endif

using namespace kvs::visclient;

ColorMapLibrary::ColorMapLibrary( const size_t resolution ):
    m_resolution( resolution )
{
    const float min_value = 0.00;
    const float max_value = 1.00;
    const float stride = ( max_value - min_value ) / ( resolution - 1 );

    kvs::ColorMap color_map;

    // Rainbow
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    color_map.addPoint( 0.00, kvs::RGBColor(  0,  0, 255 ) );
    color_map.addPoint( 0.25, kvs::RGBColor(  0, 255, 255 ) );
    color_map.addPoint( 0.50, kvs::RGBColor(  0, 255,  0 ) );
    color_map.addPoint( 0.75, kvs::RGBColor( 255, 255,  0 ) );
    color_map.addPoint( 1.00, kvs::RGBColor( 255,  0,  0 ) );
    color_map.create();
    this->add( "Rainbow", color_map );

    // Blue-white-red
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    color_map.addPoint( 0.00, kvs::RGBColor(  0,  0, 255 ) );
    color_map.addPoint( 0.50, kvs::RGBColor( 255, 255, 255 ) );
    color_map.addPoint( 1.00, kvs::RGBColor( 255,  0,  0 ) );
    color_map.create();
    this->add( "Blue-white-red", color_map );

    // Black-red-yellow-white
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    color_map.addPoint( 0.0 / 3, kvs::RGBColor(  0,  0,  0 ) );
    color_map.addPoint( 1.0f / 3, kvs::RGBColor( 255,  0,  0 ) );
    color_map.addPoint( 2.0f / 3, kvs::RGBColor( 255, 255,  0 ) );
    color_map.addPoint( 3.0 / 3, kvs::RGBColor( 255, 255, 255 ) );
    color_map.create();
    this->add( "Black-red-yellow-white", color_map );

    // Black-blue-violet-yellow-white
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    color_map.addPoint( 0.00, kvs::RGBColor(  0,  0,  0 ) );
    color_map.addPoint( 0.25, kvs::RGBColor(  0,  0, 255 ) );
    color_map.addPoint( 0.50, kvs::RGBColor( 255,  0, 255 ) );
    color_map.addPoint( 0.75, kvs::RGBColor( 255, 255,  0 ) );
    color_map.addPoint( 1.00, kvs::RGBColor( 255, 255, 255 ) );
    color_map.create();
    this->add( "Black-blue-violet-yellow-white", color_map );

    // Black-yellow-white
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    color_map.addPoint( 0.00, kvs::RGBColor(  0,  0,  0 ) );
    color_map.addPoint( 0.50, kvs::RGBColor( 255, 255,  0 ) );
    color_map.addPoint( 1.00, kvs::RGBColor( 255, 255, 255 ) );
    color_map.create();
    this->add( "Black-yellow-white", color_map );

    // Blue-green-red
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    for ( size_t n = 0; n < m_resolution; n++ )
    {
        const float x = min_value + ( stride * n );
        int r = 255 * std::min( std::max(        ( 2 * x - 0.5f      ), 0.0f ), 1.0f );
        int g = 255 * std::min( std::max( std::sin( x * float( M_PI )   ), 0.0f ), 1.0f );
        int b = 255 * std::min( std::max( std::cos( x * float( M_PI ) / 2 ), 0.0f ), 1.0f );
        color_map.addPoint( x, kvs::RGBColor( r, g, b ) );
    }
    color_map.create();
    this->add( "Blue-green-red", color_map );

    // Green-red-violet
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    for ( size_t n = 0; n < m_resolution; n++ )
    {
        const float x = min_value + ( stride * n );
        int r = 255 * std::min( std::max(        ( x        ), 0.0f ), 1.0f );
        int g = 255 * std::min( std::max( std::abs( x - 0.5f ), 0.0f ), 1.0f );
        int b = 255 * std::min( std::max( (float)std::pow( x, 4     ), 0.0f ), 1.0f );
        color_map.addPoint( x, kvs::RGBColor( r, g, b ) );
    }
    color_map.create();
    this->add( "Green-red-violet", color_map );

    // Green-blue-white
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    for ( size_t n = 0; n < m_resolution; n++ )
    {
        const float x = min_value + ( stride * n );
        int r = 255 * std::min( std::max(        ( 3 * x - 2 ), 0.0f ), 1.0f );
        int g = 255 * std::min( std::max( std::abs( ( 3 * x - 1 ) / 2 ), 0.0f ), 1.0f );
        int b = 255 * std::min( std::max(        ( x         ), 0.0f ), 1.0f );
        color_map.addPoint( x, kvs::RGBColor( r, g, b ) );
    }
    color_map.create();
    this->add( "Green-blue-white", color_map );

    // HSV model
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    for ( size_t n = 0; n < m_resolution; n++ )
    {
        const float x = min_value + ( stride * n );
        const float p = n * 1.0 / ( m_resolution - 1 );
        color_map.addPoint( x, kvs::HSVColor( p, 1.0, 1.0 ) );
    }
    color_map.create();
    this->add( "HSV model", color_map );

    // Gray-scale
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    color_map.addPoint( 0.00, kvs::RGBColor(  0,  0,  0 ) );
    color_map.addPoint( 1.00, kvs::RGBColor( 255, 255, 255 ) );
    color_map.create();
    this->add( "Gray-scale", color_map );

    // Black
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    color_map.addPoint( 0.00, kvs::RGBColor(  0,  0,  0 ) );
    color_map.addPoint( 1.00, kvs::RGBColor(  0,  0,  0 ) );
    color_map.create();
    this->add( "Black", color_map );

    // White
    color_map = kvs::ColorMap( m_resolution, min_value, max_value );
    color_map.addPoint( 0.00, kvs::RGBColor( 255, 255, 255 ) );
    color_map.addPoint( 1.00, kvs::RGBColor( 255, 255, 255 ) );
    color_map.create();
    this->add( "White", color_map );
}

ColorMapLibrary::~ColorMapLibrary()
{
}

void ColorMapLibrary::add( const std::string& name, const kvs::ColorMap& color_map )
{
    this->push_back( name );
    m_map[name] = color_map;
}

const std::string ColorMapLibrary::getNameByIndex( const size_t n ) const
{
    return this->at( n );
}

const kvs::ColorMap& ColorMapLibrary::getColorMapByIndex( const size_t n ) const
{
    return m_map.at( at( n ) );
}

const size_t ColorMapLibrary::getIndexByName( const std::string& name ) const
{
    size_t n;
    for ( n = 0; n < size(); n++ )
    {
        if ( at( n ) == name ) break;
    }
    return n;
}

const kvs::ColorMap& ColorMapLibrary::getColorMapByName( const std::string& name ) const
{
    return m_map.at( name );
}

