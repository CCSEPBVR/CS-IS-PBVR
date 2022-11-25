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
#ifndef KVS__STRUCTURED_VOLUME_OBJECT_H_INCLUDE
#define KVS__STRUCTURED_VOLUME_OBJECT_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/Module>
#include <kvs/VolumeObjectBase>


namespace kvs
{

/*==========================================================================*/
/**
 *  StructuredVolumeObject.
 */
/*==========================================================================*/
class StructuredVolumeObject
    : public kvs::VolumeObjectBase
{
    // Class name.
    kvsClassName( kvs::StructuredVolumeObject );

    // Module information.
    kvsModuleCategory( Object );
    kvsModuleBaseClass( kvs::VolumeObjectBase );

private:

    GridType       m_grid_type;  ///< Grid type.
    kvs::Vector3ui m_resolution; ///< Node resolution.

public:

    StructuredVolumeObject( void );

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

    virtual ~StructuredVolumeObject( void );

public:

    static kvs::StructuredVolumeObject* DownCast( kvs::ObjectBase* object );

    static const kvs::StructuredVolumeObject* DownCast( const kvs::ObjectBase* object );

public:

    StructuredVolumeObject& operator = ( const StructuredVolumeObject& object );

    friend std::ostream& operator << ( std::ostream& os, const StructuredVolumeObject& object );

public:

    void shallowCopy( const StructuredVolumeObject& object );

    void deepCopy( const StructuredVolumeObject& object );

public:

    void setGridType( const GridType grid_type );

    void setResolution( const kvs::Vector3ui& resolution );

public:

    const VolumeType volumeType( void ) const;

    const GridType gridType( void ) const;

    const CellType cellType( void ) const;

    const kvs::Vector3ui& resolution( void ) const;

    const size_t nnodesPerLine( void ) const;

    const size_t nnodesPerSlice( void ) const;

    const size_t nnodes( void ) const;

public:

    void updateMinMaxCoords( void );

private:

    void calculate_min_max_coords( void );
};

} // end of namespace kvs

#endif // KVS__STRUCTURED_VOLUME_OBJECT_H_INCLUDE
