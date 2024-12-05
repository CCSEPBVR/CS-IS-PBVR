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
#ifndef VIS_MODULE__MAPPER_BASE_H_INCLUDE
#define VIS_MODULE__MAPPER_BASE_H_INCLUDE

#include <vismodule/ClassName>
#include <vismodule/ObjectBase>
#include <vismodule/VolumeObjectBase>
#include <vismodule/TransferFunction>
#include <vismodule/ColorMap>
#include <vismodule/OpacityMap>
#include <vismodule/Module>


namespace vismodule
{

class VolumeObjectBase;

/*==========================================================================*/
/**
 *  MapperBase.
 */
/*==========================================================================*/
class MapperBase
{
    visModuleClassName( vismodule::MapperBase );

    visModuleBase;

protected:

    vismodule::TransferFunction m_transfer_function; ///< Transfer function.
    const vismodule::VolumeObjectBase* m_volume; ///< Volume object.
    bool m_is_success; ///< Check flag for mapping.

public:

    MapperBase();

    explicit MapperBase( const vismodule::TransferFunction& m_transfer_function );

    virtual ~MapperBase();

public:

    virtual vismodule::ObjectBase* exec( const vismodule::ObjectBase& object ) = 0;

public:

    void setTransferFunction( const vismodule::TransferFunction& transfer_function );

    void setColorMap( const vismodule::ColorMap& color_map );

    void setOpacityMap( const vismodule::OpacityMap& opacity_map );

public:

    const vismodule::VolumeObjectBase* const volume() const;

    const vismodule::TransferFunction& transferFunction() const;

    const vismodule::ColorMap& colorMap() const;

    const vismodule::OpacityMap& opacityMap() const;

    const bool isSuccess() const;

    const bool isFailure() const;

protected:

    void attach_volume( const vismodule::VolumeObjectBase& volume );

    void set_range( const vismodule::VolumeObjectBase& volume );

    void set_min_max_coords( const vismodule::VolumeObjectBase& volume, vismodule::ObjectBase* object );
};

} // end of namespace vismodule

#endif // VIS_MODULE__MAPPER_BASE_H_INCLUDE
