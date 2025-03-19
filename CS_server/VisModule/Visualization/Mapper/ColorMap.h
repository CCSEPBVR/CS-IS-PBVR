/****************************************************************************/
/**
 *  @file ColorMap.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ColorMap.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef VIS_MODULE__COLOR_MAP_H_INCLUDE
#define VIS_MODULE__COLOR_MAP_H_INCLUDE

#include <list>
#include <utility>
#include <vismodule/ClassName>
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/RGBColor>


namespace vismodule
{

/*==========================================================================*/
/**
 *  Color map class.
 */
/*==========================================================================*/
class ColorMap
{
    visModuleClassName( vismodule::ColorMap );

public:

    typedef vismodule::ValueArray<vismodule::UInt8>    Table;
    typedef std::pair<float,vismodule::RGBColor> Point;
    typedef std::list<Point>               Points;

private:

    size_t m_resolution; ///< table resolution
    float  m_min_value;  ///< min. value
    float  m_max_value;  ///< max. value
    Points m_points;     ///< control point list
    Table  m_table;      ///< value table

public:

    ColorMap( void );

    explicit ColorMap( const size_t resolution );

    explicit ColorMap( const Table& table );

    ColorMap( const ColorMap& other );

    ColorMap( const size_t resolution, const float min_value, const float max_value );

    ColorMap( const Table& table, const float min_value, const float max_value );

    virtual ~ColorMap( void );

public:

    const float minValue( void ) const;

    const float maxValue( void ) const;

    const size_t resolution( void ) const;

    const Points& points( void ) const;

    const Table& table( void ) const;

public:

    const bool hasRange( void ) const;

    void setRange( const float min_value, const float max_value );

    void setResolution( const size_t resolution );

    void addPoint( const float value, const vismodule::RGBColor color );

    void removePoint( const float value );

    void create( void );

public:

    const vismodule::RGBColor operator []( const size_t index ) const;

    const vismodule::RGBColor at( const float value ) const;

public:

    ColorMap& operator =( const ColorMap& rhs );
};

} // end of namespace vismodule

#endif // VIS_MODULE__COLOR_MAP_H_INCLUDE
