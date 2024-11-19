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
class StructuredVolumeObject
    : public vismodule::VolumeObjectBase
{
    // Class name.
    visModuleClassName( vismodule::StructuredVolumeObject );

    // Module information.
    visModuleCategory( Object );
    visModuleBaseClass( vismodule::VolumeObjectBase );

private:

    GridType       m_grid_type;  ///< Grid type.
    vismodule::Vector3ui m_resolution; ///< Node resolution.

public:

    StructuredVolumeObject( void );

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

    virtual ~StructuredVolumeObject( void );

public:

    static vismodule::StructuredVolumeObject* DownCast( vismodule::ObjectBase* object );

    static const vismodule::StructuredVolumeObject* DownCast( const vismodule::ObjectBase* object );

public:

    StructuredVolumeObject& operator = ( const StructuredVolumeObject& object );

    friend std::ostream& operator << ( std::ostream& os, const StructuredVolumeObject& object );

public:

    void shallowCopy( const StructuredVolumeObject& object );

    void deepCopy( const StructuredVolumeObject& object );

public:

    void setGridType( const GridType grid_type );

    void setResolution( const vismodule::Vector3ui& resolution );

public:

    const VolumeType volumeType( void ) const;

    const GridType gridType( void ) const;

    const CellType cellType( void ) const;

    const vismodule::Vector3ui& resolution( void ) const;

    const size_t nnodesPerLine( void ) const;

    const size_t nnodesPerSlice( void ) const;

    const size_t nnodes( void ) const;

public:

    void updateMinMaxCoords( void );

private:

    void calculate_min_max_coords( void );
};

} // end of namespace vismodule

#endif // VIS_MODULE__STRUCTURED_VOLUME_OBJECT_H_INCLUDE
