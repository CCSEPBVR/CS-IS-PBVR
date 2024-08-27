/****************************************************************************/
/**
 *  @file MapperBase.h
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MapperBase.h 634 2010-10-13 07:04:05Z naohisa.sakamoto $
 */
/****************************************************************************/
#ifndef KVS__MAPPER_BASE_H_INCLUDE
#define KVS__MAPPER_BASE_H_INCLUDE

#include <kvs/ClassName>
#include <kvs/ObjectBase>
#include <kvs/VolumeObjectBase>
#include <kvs/TransferFunction>
#include <kvs/ColorMap>
#include <kvs/OpacityMap>
#include <kvs/Module>


namespace kvs
{

class VolumeObjectBase;

/*==========================================================================*/
/**
 *  MapperBase.
 */
/*==========================================================================*/
class MapperBase
{
    kvsClassName( kvs::MapperBase );

    kvsModuleBase;

protected:

    kvs::TransferFunction m_transfer_function; ///< Transfer function.
    const kvs::VolumeObjectBase* m_volume; ///< Volume object.
    bool m_is_success; ///< Check flag for mapping.

public:

    MapperBase( void );

    explicit MapperBase( const kvs::TransferFunction& transfer_function );

    virtual ~MapperBase( void );

public:

    virtual kvs::ObjectBase* exec( const kvs::ObjectBase* object ) = 0;

public:

    void setTransferFunction( const kvs::TransferFunction& transfer_function );

    void setColorMap( const kvs::ColorMap& color_map );

    void setOpacityMap( const kvs::OpacityMap& opacity_map );

public:

    const kvs::VolumeObjectBase* const volume( void ) const;

    const kvs::TransferFunction& transferFunction( void ) const;

    const kvs::ColorMap& colorMap( void ) const;

    const kvs::OpacityMap& opacityMap( void ) const;

    const bool isSuccess( void ) const;

    const bool isFailure( void ) const;

protected:

    void attach_volume( const kvs::VolumeObjectBase* volume );

    void set_range( const kvs::VolumeObjectBase* volume );

    void set_min_max_coords( const kvs::VolumeObjectBase* volume, kvs::ObjectBase* object );
};

} // end of namespace kvs

#endif // KVS__MAPPER_BASE_H_INCLUDE
