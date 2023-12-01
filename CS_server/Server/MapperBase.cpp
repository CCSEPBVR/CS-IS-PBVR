/****************************************************************************/
/**
 *  @file MapperBase.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: MapperBase.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "MapperBase.h"
#include "StructuredVolumeObject.h"
#include "UnstructuredVolumeObject.h"


namespace pbvr
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new empty MapperBase.
 */
/*==========================================================================*/
MapperBase::MapperBase():
    m_transfer_function(),
    m_volume( 0 ),
    m_is_success( false )
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new MapperBase.
 */
/*==========================================================================*/
MapperBase::MapperBase( const TransferFunction& transfer_function ):
    m_transfer_function( transfer_function ),
    m_volume( 0 ),
    m_is_success( false )
{
}

/*==========================================================================*/
/**
 *  @brief  Destroys the MapperBase.
 */
/*==========================================================================*/
MapperBase::~MapperBase()
{
}

/*===========================================================================*/
/**
 *  @brief  Sets a transfer function.
 *  @param  m_transfer_function [in] transfer function
 */
/*===========================================================================*/
void MapperBase::setTransferFunction( const pbvr::TransferFunction& transfer_function )
{
    m_transfer_function = transfer_function;
}

/*===========================================================================*/
/**
 *  @brief  Sets a color map.
 *  @param  color_map [in] color map
 */
/*===========================================================================*/
void MapperBase::setColorMap( const kvs::ColorMap& color_map )
{
    m_transfer_function.setColorMap( color_map );
}

/*===========================================================================*/
/**
 *  @brief  Sets a opacity map.
 *  @param  opacity_map [in] opacity map
 */
/*===========================================================================*/
void MapperBase::setOpacityMap( const kvs::OpacityMap& opacity_map )
{
    m_transfer_function.setOpacityMap( opacity_map );
}

/*===========================================================================*/
/**
 *  @brief  Returns the pointer to the volume object.
 *  @return pointer to the volume object
 */
/*===========================================================================*/
const pbvr::VolumeObjectBase* const MapperBase::volume() const
{
    return m_volume;
}

/*===========================================================================*/
/**
 *  @brief  Returns the transfer function.
 */
/*===========================================================================*/
const pbvr::TransferFunction& MapperBase::transferFunction() const
{
    return m_transfer_function;
}

/*===========================================================================*/
/**
 *  @brief  Returns the color map.
 */
/*===========================================================================*/
const kvs::ColorMap& MapperBase::colorMap() const
{
    return m_transfer_function.colorMap();
}

/*===========================================================================*/
/**
 *  @brief  Returns the opacity map.
 */
/*===========================================================================*/
const kvs::OpacityMap& MapperBase::opacityMap() const
{
    return m_transfer_function.opacityMap();
}

/*==========================================================================*/
/**
 *  @brief  Returns true if the mapping is success; otherwise returns false.
 *  @return Whether the mapping is success or not.
 */
/*==========================================================================*/
const bool MapperBase::isSuccess() const
{
    return m_is_success;
}

/*==========================================================================*/
/**
 *  @brief  Returns true if the mapping is failure; otherwise returns false.
 *  @return Whether the mapping is failure or not.
 */
/*==========================================================================*/
const bool MapperBase::isFailure() const
{
    return !m_is_success;
}

/*===========================================================================*/
/**
 *  @brief  Attaches a voluem object.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
void MapperBase::attach_volume( const pbvr::VolumeObjectBase& volume )
{
    m_volume = &volume;
}

void MapperBase::set_range( const pbvr::VolumeObjectBase& volume )
{
    if ( !volume.hasMinMaxValues() ) volume.updateMinMaxValues();
    const std::type_info& type = volume.values().typeInfo()->type();
    if ( type == typeid( kvs::Int8 ) )
    {
        if ( !m_transfer_function.hasRange() ) m_transfer_function.setRange( -128, 127 );
    }
    else if ( type == typeid( kvs::UInt8  ) )
    {
        if ( !m_transfer_function.hasRange() ) m_transfer_function.setRange( 0, 255 );
    }
    else
    {
        const float min_value = static_cast<float>( volume.minValue() );
        const float max_value = static_cast<float>( volume.maxValue() );
        if ( !m_transfer_function.hasRange() ) m_transfer_function.setRange( min_value, max_value );
    }
}

/*===========================================================================*/
/**
 *  @brief  Calculates the min/max coordinate values.
 *  @param  volume [in] pointer to the input volume object
 *  @param  object [in] pointer to the object (The calculated coordinates are set to this object)
 */
/*===========================================================================*/
void MapperBase::set_min_max_coords( const pbvr::VolumeObjectBase& volume, pbvr::ObjectBase* object )
{
    if ( !volume.hasMinMaxObjectCoords() )
    {
        switch ( volume.volumeType() )
        {
        case pbvr::VolumeObjectBase::Structured:
        {
            // WARNING: remove constness, but safe in this case.
            pbvr::VolumeObjectBase* b = const_cast<pbvr::VolumeObjectBase*>( &volume );
            //reinterpret_cast<pbvr::StructuredVolumeObject*>( b )->updateMinMaxCoords();
            break;
        }
        case pbvr::VolumeObjectBase::Unstructured:
        {
            // WARNING: remove constness, but safe in this case.
            pbvr::VolumeObjectBase* b = const_cast<pbvr::VolumeObjectBase*>( &volume );
            reinterpret_cast<pbvr::UnstructuredVolumeObject*>( b )->updateMinMaxCoords();
            break;
        }
        default:
            break;
        }
    }

    const kvs::Vector3f min_obj_coord( m_volume->minObjectCoord() );
    const kvs::Vector3f max_obj_coord( m_volume->maxObjectCoord() );
    const kvs::Vector3f min_ext_coord( m_volume->minExternalCoord() );
    const kvs::Vector3f max_ext_coord( m_volume->maxExternalCoord() );
    object->setMinMaxObjectCoords( min_obj_coord, max_obj_coord );
    object->setMinMaxExternalCoords( min_ext_coord, max_ext_coord );
}

} // end of namespace pbvr
