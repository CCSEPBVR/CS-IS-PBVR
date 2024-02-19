/*****************************************************************************/
/**
 *  @file   GeometryObjectBase.h
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: GeometryObjectBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/*****************************************************************************/
#ifndef PBVR__GEOMETRY_OBJECT_BASE_H_INCLUDE
#define PBVR__GEOMETRY_OBJECT_BASE_H_INCLUDE

#include "ObjectBase.h"
#include "ClassName.h"
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector3>
#include <kvs/RGBColor>


namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  Geometric object class.
 */
/*===========================================================================*/
class GeometryObjectBase : public pbvr::ObjectBase
{
    kvsClassName( pbvr::GeometryObjectBase );

public:

    typedef pbvr::ObjectBase BaseClass;

    enum GeometryType
    {
        Point,   ///< point object
        Line,    ///< line object
        Polygon, ///< polygon object
    };

protected:

    kvs::ValueArray<kvs::Real32> m_coords;  ///< vertex array
    kvs::ValueArray<kvs::UInt8>  m_colors;  ///< color(r,g,b) array
    kvs::ValueArray<kvs::Real32> m_normals; ///< normal array

public:

    GeometryObjectBase( void );

    GeometryObjectBase(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::ValueArray<kvs::Real32>& normals );

    GeometryObjectBase(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::RGBColor&                color,
        const kvs::ValueArray<kvs::Real32>& normals );

    GeometryObjectBase(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::Real32>& normals );

    GeometryObjectBase(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt8>&  colors );

    GeometryObjectBase(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::RGBColor&                color );

    GeometryObjectBase(
        const kvs::ValueArray<kvs::Real32>& coords );

    virtual ~GeometryObjectBase( void );

public:

    static pbvr::GeometryObjectBase* DownCast( pbvr::ObjectBase* object );

    static const pbvr::GeometryObjectBase* DownCast( const pbvr::ObjectBase* object );

public:

    friend std::ostream& operator << ( std::ostream& os, const GeometryObjectBase& object );

public:

    void shallowCopy( const GeometryObjectBase& object );

    void deepCopy( const GeometryObjectBase& object );

    void clear( void );

public:

    void setCoords( const kvs::ValueArray<kvs::Real32>& coords );

    void setColors( const kvs::ValueArray<kvs::UInt8>& colors );

    void setColor( const kvs::RGBColor& color );

    void setNormals( const kvs::ValueArray<kvs::Real32>& normals );

public:

    const ObjectType objectType( void ) const;

    virtual const GeometryType geometryType( void ) const = 0;

    const size_t nvertices( void ) const;

    const size_t ncolors( void ) const;

    const size_t nnormals( void ) const;

public:

    const kvs::Vector3f coord( const size_t index = 0 ) const;

    const kvs::RGBColor color( const size_t index = 0 ) const;

    const kvs::Vector3f normal( const size_t index = 0 ) const;

public:

    const kvs::ValueArray<kvs::Real32>& coords( void ) const;

    const kvs::ValueArray<kvs::UInt8>& colors( void ) const;

    const kvs::ValueArray<kvs::Real32>& normals( void ) const;

public:

    void updateMinMaxCoords( void );

private:

    void calculate_min_max_coords( void );
};

} // end of namespace kvs

#endif // KVS__GEOMETRY_OBJECT_BASE_H_INCLUDE
