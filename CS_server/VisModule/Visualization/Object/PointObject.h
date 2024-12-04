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
#include <vismodule/ValueArray>
#include <vismodule/Type>
#include <vismodule/Vector3>
#include <vismodule/RGBColor>
#include <vismodule/Module>
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
    visModuleClassName( pbvr::PointObject );

    // Module information.
    typedef pbvr::ObjectBase::ModuleTag ModuleCategory;
    visModuleBaseClass( pbvr::GeometryObjectBase );

protected:

    vismodule::ValueArray<vismodule::Real32> m_sizes; ///< size array
    vismodule::ValueArray<pbvr::FrequencyTable> m_color_histogram;
    vismodule::ValueArray<pbvr::FrequencyTable> m_opacity_histogram;
    vismodule::ValueArray<int> m_c_histogram;
    vismodule::ValueArray<int> m_o_histogram;
    int m_nbins;
    int m_tf_number;


public:

    PointObject();

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const vismodule::ValueArray<vismodule::Real32>& sizes );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const vismodule::Real32                   size );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const vismodule::ValueArray<vismodule::Real32>& sizes );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const vismodule::ValueArray<vismodule::Real32>& sizes );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::ValueArray<vismodule::Real32>& normals,
        const vismodule::Real32                   size );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::ValueArray<vismodule::Real32>& sizes );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::ValueArray<vismodule::Real32>& sizes );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::ValueArray<vismodule::UInt8>&  colors,
        const vismodule::Real32                   size );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords,
        const vismodule::RGBColor&                color,
        const vismodule::Real32                   size );

    PointObject(
        const vismodule::ValueArray<vismodule::Real32>& coords );

    PointObject( const pbvr::PointObject& other );

    //PointObject( const vismodule::LineObject& line );

    //PointObject( const vismodule::PolygonObject& polygon );

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

    void setSizes( const vismodule::ValueArray<vismodule::Real32>& sizes );

    void setSize( const vismodule::Real32 size );

    void setTfnumber( const int tf_number);

    void setNbins(const int nbins); 

public:

    const BaseClass::GeometryType geometryType() const;

    const size_t nsizes() const;

public:

    const vismodule::Real32 size( const size_t index = 0 ) const;

    const vismodule::ValueArray<vismodule::Real32>& sizes() const;

public:

    const vismodule::ValueArray<pbvr::FrequencyTable>& getColorHistogram() const;
    const vismodule::ValueArray<pbvr::FrequencyTable>& getOpacityHistogram() const;
    //add shimomura 2023/05/29
    const vismodule::ValueArray<int>& getCHistogram() const;
    const vismodule::ValueArray<int>& getOHistogram() const;
    const int getTfnumber() const; 
    const int getNbins() const; 
};

} // end of namespace pbvr

#endif // VIS_MODULE__POINT_OBJECT_H_INCLUDE
