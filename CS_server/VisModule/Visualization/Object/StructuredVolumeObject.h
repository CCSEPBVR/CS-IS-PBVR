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
#ifndef VIS_MODULE__STRUCTURED_VOLUME_OBJECT_H_INCLUDE
#define VIS_MODULE__STRUCTURED_VOLUME_OBJECT_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/Module>
#include <vismodule/VolumeObjectBase>

namespace vismodule
{

/*==========================================================================*/
/**
 *  StructuredVolumeObject.
 */
/*==========================================================================*/
class StructuredVolumeObject : public vismodule::VolumeObjectBase
{
    // Class name.
    visModuleClassName( vismodule::StructuredVolumeObject );

    // Module information.
    typedef vismodule::ObjectBase::ModuleTag ModuleCategory;
    visModuleBaseClass( vismodule::VolumeObjectBase );

private:

    GridType       m_grid_type;  ///< Grid type.
    vismodule::Vector3ui m_resolution; ///< Node resolution.

public:

    StructuredVolumeObject();

    StructuredVolumeObject(
        const vismodule::Vector3ui& resolution,
        const size_t          veclen,
        const Values&         values );

    StructuredVolumeObject(
        const GridType        grid_type,
        const vismodule::Vector3ui& resolution,
        const size_t          veclen,
        const Coords&         coords,
        const Values&         values );

    StructuredVolumeObject( const StructuredVolumeObject& other );

    virtual ~StructuredVolumeObject();

public:

    static vismodule::StructuredVolumeObject* DownCast( vismodule::ObjectBase* object );

    static const vismodule::StructuredVolumeObject* DownCast( const vismodule::ObjectBase& object );

public:

    StructuredVolumeObject& operator = ( const StructuredVolumeObject& object );

    friend std::ostream& operator << ( std::ostream& os, const StructuredVolumeObject& object );

public:

    void shallowCopy( const StructuredVolumeObject& object );

    void deepCopy( const StructuredVolumeObject& object );

public:

    void setGridType( const GridType grid_type );

    //void setm_resolution( const vismodule::Vector3ui& resolution );
    void setResolution( const vismodule::Vector3ui& resolution );

public:

    const VolumeType volumeType() const;

    const GridType gridType() const;

    const CellType cellType() const;

    const vismodule::Vector3ui& resolution() const;

    const size_t nnodesPerLine() const;

    const size_t nnodesPerSlice() const;

    const size_t nnodes() const;

public:

    void updateMinMaxCoords();

private:

    void calculateMinMaxCoords();
};

} // end of namespace vismodule

#endif // VIS_MODULE__STRUCTURED_VOLUME_OBJECT_H_INCLUDE
