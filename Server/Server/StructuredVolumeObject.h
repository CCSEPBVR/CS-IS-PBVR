/****************************************************************************/
/**
 *  @file StructuredVolumeObject.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StructuredVolumeObject.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef PBVR__STRUCTURED_VOLUME_OBJECT_H_INCLUDE
#define PBVR__STRUCTURED_VOLUME_OBJECT_H_INCLUDE

#include "ClassName.h"
#include <kvs/Module>
#include "VolumeObjectBase.h"


namespace pbvr
{

/*==========================================================================*/
/**
 *  StructuredVolumeObject.
 */
/*==========================================================================*/
class StructuredVolumeObject : public pbvr::VolumeObjectBase
{
    // Class name.
    kvsClassName( pbvr::StructuredVolumeObject );

    // Module information.
    typedef pbvr::ObjectBase::ModuleTag ModuleCategory;
    kvsModuleBaseClass( pbvr::VolumeObjectBase );

private:

    GridType       m_grid_type;  ///< Grid type.
    kvs::Vector3ui m_resolution; ///< Node resolution.

public:

    StructuredVolumeObject();

    StructuredVolumeObject(
        const kvs::Vector3ui& resolution,
        const size_t          veclen,
        const Values&         values );

    StructuredVolumeObject(
        const GridType        grid_type,
        const kvs::Vector3ui& resolution,
        const size_t          veclen,
        const Coords&         coords,
        const Values&         values );

    StructuredVolumeObject( const StructuredVolumeObject& other );

    virtual ~StructuredVolumeObject();

public:

    static pbvr::StructuredVolumeObject* DownCast( pbvr::ObjectBase* object );

    static const pbvr::StructuredVolumeObject* DownCast( const pbvr::ObjectBase& object );

public:

    StructuredVolumeObject& operator = ( const StructuredVolumeObject& object );

    friend std::ostream& operator << ( std::ostream& os, const StructuredVolumeObject& object );

public:

    void shallowCopy( const StructuredVolumeObject& object );

    void deepCopy( const StructuredVolumeObject& object );

public:

    void setGridType( const GridType grid_type );

    void setm_resolution( const kvs::Vector3ui& resolution );

public:

    const VolumeType volumeType() const;

    const GridType gridType() const;

    const CellType cellType() const;

    const kvs::Vector3ui& resolution() const;

    const size_t nnodesPerLine() const;

    const size_t nnodesPerSlice() const;

    const size_t nnodes() const;

public:

    void updateMinMaxCoords();

private:

    void calculateMinMaxCoords();
};

} // end of namespace pbvr

#endif // KVS__STRUCTURED_VOLUME_OBJECT_H_INCLUDE
