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
#ifndef PBVR__MAPPER_BASE_H_INCLUDE
#define PBVR__MAPPER_BASE_H_INCLUDE

#include "ClassName.h"
#include "ObjectBase.h"
#include "VolumeObjectBase.h"
#include "TransferFunction.h"
#include <kvs/ColorMap>
#include <kvs/OpacityMap>
#include <kvs/Module>


namespace pbvr
{

class VolumeObjectBase;

/*==========================================================================*/
/**
 *  MapperBase.
 */
/*==========================================================================*/
class MapperBase
{
    kvsClassName( pbvr::MapperBase );

    kvsModuleBase;

protected:

    pbvr::TransferFunction m_transfer_function; ///< Transfer function.
    const pbvr::VolumeObjectBase* m_volume; ///< Volume object.
    bool m_is_success; ///< Check flag for mapping.

public:

    MapperBase();

    explicit MapperBase( const pbvr::TransferFunction& m_transfer_function );

    virtual ~MapperBase();

public:

    virtual pbvr::ObjectBase* exec( const pbvr::ObjectBase& object ) = 0;

public:

    void setTransferFunction( const pbvr::TransferFunction& transfer_function );

    void setColorMap( const kvs::ColorMap& color_map );

    void setOpacityMap( const kvs::OpacityMap& opacity_map );

public:

    const pbvr::VolumeObjectBase* const volume() const;

    const pbvr::TransferFunction& transferFunction() const;

    const kvs::ColorMap& colorMap() const;

    const kvs::OpacityMap& opacityMap() const;

    const bool isSuccess() const;

    const bool isFailure() const;

protected:

    void attach_volume( const pbvr::VolumeObjectBase& volume );

    void set_range( const pbvr::VolumeObjectBase& volume );

    void set_min_max_coords( const pbvr::VolumeObjectBase& volume, pbvr::ObjectBase* object );
};

} // end of namespace pbvr

#endif // PBVR__MAPPER_BASE_H_INCLUDE
