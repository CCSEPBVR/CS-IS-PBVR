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
#ifndef PBVR__POINT_OBJECT_H_INCLUDE
#define PBVR__POINT_OBJECT_H_INCLUDE

#include "GeometryObjectBase.h"
#include "ObjectBase.h"
#include "ClassName.h"
#include <kvs/ValueArray>
#include <kvs/Type>
#include <kvs/Vector3>
#include <kvs/RGBColor>
#include <kvs/Module>
#include "FrequencyTable.h"

namespace pbvr
{

//class LineObject;
//class PolygonObject;

/*==========================================================================*/
/**
 *  Point object class.
 */
/*==========================================================================*/
class PointObject : public pbvr::GeometryObjectBase
{
    // Class name.
    kvsClassName( pbvr::PointObject );

    // Module information.
    typedef pbvr::ObjectBase::ModuleTag ModuleCategory;
    kvsModuleBaseClass( pbvr::GeometryObjectBase );

protected:

    kvs::ValueArray<kvs::Real32> m_sizes; ///< size array
    kvs::ValueArray<pbvr::FrequencyTable> m_color_histogram;
    kvs::ValueArray<pbvr::FrequencyTable> m_opacity_histogram;
    kvs::ValueArray<int> m_c_histogram;
    kvs::ValueArray<int> m_o_histogram;
    int m_nbins;
    int m_tf_number;


public:

    PointObject();

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

    PointObject( const pbvr::PointObject& other );

    //PointObject( const kvs::LineObject& line );

    //PointObject( const kvs::PolygonObject& polygon );

    virtual ~PointObject();

public:

    static pbvr::PointObject* DownCast( pbvr::ObjectBase* object );

    static const pbvr::PointObject* DownCast( const pbvr::ObjectBase& object );

public:

    PointObject& operator = ( const PointObject& other );

    PointObject& operator += ( const PointObject& other );

    friend std::ostream& operator << ( std::ostream& os, const PointObject& object );

public:

    void add( const PointObject& other );

    void shallowCopy( const PointObject& other );

    void deepCopy( const PointObject& other );

    void clear();

public:

    void setSizes( const kvs::ValueArray<kvs::Real32>& sizes );

    void setSize( const kvs::Real32 size );

    void setTfnumber( const int tf_number);

    void setNbins(const int nbins); 

public:

    const BaseClass::GeometryType geometryType() const;

    const size_t nsizes() const;

public:

    const kvs::Real32 size( const size_t index = 0 ) const;

    const kvs::ValueArray<kvs::Real32>& sizes() const;

public:

    const kvs::ValueArray<pbvr::FrequencyTable>& getColorHistogram() const;
    const kvs::ValueArray<pbvr::FrequencyTable>& getOpacityHistogram() const;
    //add shimomura 2023/05/29
    const kvs::ValueArray<int>& getCHistogram() const;
    const kvs::ValueArray<int>& getOHistogram() const;
    const int getTfnumber() const; 
    const int getNbins() const; 
};

} // end of namespace pbvr

#endif // KVS__POINT_OBJECT_H_INCLUDE
