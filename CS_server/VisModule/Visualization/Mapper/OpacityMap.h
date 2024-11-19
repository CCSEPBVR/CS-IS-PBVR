/****************************************************************************/
/**
 *  @file OpacityMap.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: OpacityMap.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__OPACITY_MAP_H_INCLUDE
#define KVS__OPACITY_MAP_H_INCLUDE

#include <list>
#include <utility>
#include <kvs/ClassName>
#include <kvs/ValueArray>


namespace kvs
{

/*==========================================================================*/
/**
 *  OpacityMap.
 */
/*==========================================================================*/
class OpacityMap
{
    kvsClassName( kvs::OpacityMap );

public:

    typedef kvs::ValueArray<float> Table;
    typedef std::pair<float,float> Point;
    typedef std::list<Point>       Points;

private:

    size_t m_resolution; ///< table resolution
    float  m_min_value;  ///< min. value
    float  m_max_value;  ///< max. value
    Points m_points;     ///< control point list
    Table  m_table;      ///< value table

public:

    OpacityMap( void );

    explicit OpacityMap( const size_t resolution );

    explicit OpacityMap( const Table& table );

    OpacityMap( const OpacityMap& other );

    OpacityMap( const size_t resolution, const float min_value, const float max_value );

    OpacityMap( const Table& table, const float min_value, const float max_value );

    virtual ~OpacityMap( void );

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

    void addPoint( const float value, const float opacity );

    void removePoint( const float value );

    void create( void );

public:

    const kvs::Real32 operator []( const size_t index ) const;

    const kvs::Real32 at( const float value ) const;

public:

    OpacityMap& operator =( const OpacityMap& rhs );
};

} // end of namespace kvs

#endif // KVS__OPACITY_MAP_H_INCLUDE
