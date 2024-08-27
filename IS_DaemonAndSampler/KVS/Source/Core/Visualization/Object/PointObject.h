/****************************************************************************/
/**
 *  @file PointObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: PointObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__POINT_OBJECT_H_INCLUDE
#define KVS__POINT_OBJECT_H_INCLUDE

#include <kvs/GeometryObjectBase>
#include <kvs/ClassName>
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector3>
#include <kvs/RGBColor>
#include <kvs/Module>


namespace kvs
{

class LineObject;
class PolygonObject;

/*==========================================================================*/
/**
 *  Point object class.
 */
/*==========================================================================*/
class PointObject : public kvs::GeometryObjectBase
{
    // Class name.
    kvsClassName( kvs::PointObject );

    // Module information.
    kvsModuleCategory( Object );
    kvsModuleBaseClass( kvs::GeometryObjectBase );

protected:

    kvs::ValueArray<kvs::Real32> m_sizes; ///< size array

public:

    PointObject( void );

    PointObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::ValueArray<kvs::Real32>& normals,
        const kvs::ValueArray<kvs::Real32>& sizes );

    PointObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::ValueArray<kvs::Real32>& normals,
        const kvs::Real32                   size );

    PointObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::RGBColor&                color,
        const kvs::ValueArray<kvs::Real32>& normals,
        const kvs::ValueArray<kvs::Real32>& sizes );

    PointObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::Real32>& normals,
        const kvs::ValueArray<kvs::Real32>& sizes );

    PointObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::RGBColor&                color,
        const kvs::ValueArray<kvs::Real32>& normals,
        const kvs::Real32                   size );

    PointObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::ValueArray<kvs::Real32>& sizes );

    PointObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::RGBColor&                color,
        const kvs::ValueArray<kvs::Real32>& sizes );

    PointObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::ValueArray<kvs::UInt8>&  colors,
        const kvs::Real32                   size );

    PointObject(
        const kvs::ValueArray<kvs::Real32>& coords,
        const kvs::RGBColor&                color,
        const kvs::Real32                   size );

    PointObject(
        const kvs::ValueArray<kvs::Real32>& coords );

    PointObject( const kvs::PointObject& other );

    PointObject( const kvs::LineObject& line );

    PointObject( const kvs::PolygonObject& polygon );

    virtual ~PointObject( void );

public:

    static kvs::PointObject* DownCast( kvs::ObjectBase* object );

    static const kvs::PointObject* DownCast( const kvs::ObjectBase* object );

public:

    PointObject& operator = ( const PointObject& other );

    PointObject& operator += ( const PointObject& other );

    friend std::ostream& operator << ( std::ostream& os, const PointObject& object );

public:

    void add( const PointObject& other );

    void shallowCopy( const PointObject& other );

    void deepCopy( const PointObject& other );

    void clear( void );

public:

    void setSizes( const kvs::ValueArray<kvs::Real32>& sizes );

    void setSize( const kvs::Real32 size );

public:

    const BaseClass::GeometryType geometryType( void ) const;

    const size_t nsizes( void ) const;

public:

    const kvs::Real32 size( const size_t index = 0 ) const;

    const kvs::ValueArray<kvs::Real32>& sizes( void ) const;
};

} // end of namespace kvs

#endif // KVS__POINT_OBJECT_H_INCLUDE
