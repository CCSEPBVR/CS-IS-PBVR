/*****************************************************************************/
/**
 *  @file   StructuredVolumeExporter.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: StructuredVolumeExporter.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "StructuredVolumeExporter.h"
#include <kvs/ObjectBase>
#include <kvs/VolumeObjectBase>
#include <kvs/StructuredVolumeObject>


namespace
{

/*===========================================================================*/
/**
 *  @brief  Casts to a pointer to the structured volume object.
 *  @param  object [in] pointer to the object
 *  @return pointer to the structured volume object
 */
/*===========================================================================*/
const kvs::StructuredVolumeObject* CastToStructuredVolumeObject( const kvs::ObjectBase* object )
{
    if ( object->objectType() != kvs::ObjectBase::Volume )
    {
        kvsMessageError("Input object is not a volumetry object.");
        return( NULL );
    }

    const kvs::VolumeObjectBase* volume =
        reinterpret_cast<const kvs::VolumeObjectBase*>( object );
    if ( volume->volumeType() != kvs::VolumeObjectBase::Structured )
    {
        kvsMessageError("Input object is not a structured volume object.");
        return( NULL );
    }

    return( reinterpret_cast<const kvs::StructuredVolumeObject*>( volume ) );
}

} // end of namespace


namespace kvs
{

/*===========================================================================*/
/**
 *  @brief  Constructs a KVSMLObjectStructuredVolume data from given object.
 *  @param  object [in] pointer to the structured volume object
 */
/*===========================================================================*/
StructuredVolumeExporter<kvs::KVSMLObjectStructuredVolume>::StructuredVolumeExporter(
    const kvs::StructuredVolumeObject* object )
{
    this->exec( object );
}

/*===========================================================================*/
/**
 *  @brief  Exports object to a KVSMLObjectStructuredVolume data.
 *  @param  object [in] pointer to the structured volume object
 */
/*===========================================================================*/
kvs::KVSMLObjectStructuredVolume* StructuredVolumeExporter<kvs::KVSMLObjectStructuredVolume>::exec(
    const kvs::ObjectBase* object )
{
    if ( !object )
    {
        m_is_success = false;
        kvsMessageError("Input object is NULL.");
        return( NULL );
    }

    // Cast to the structured volume object.
    const kvs::StructuredVolumeObject* volume = kvs::StructuredVolumeObject::DownCast( object );
    if ( !volume )
    {
        m_is_success = false;
        kvsMessageError("Input object is not structured volume object.");
        return( NULL );
    }

    // Check the grid type of the given structured volume object.
    switch ( volume->gridType() )
    {
    case kvs::StructuredVolumeObject::UnknownGridType:
    {
        kvsMessageError("Unknown grid type.");
        break;
    }
    case kvs::StructuredVolumeObject::Uniform:
    {
        this->setGridType("uniform");
        break;
    }
/*
    case kvs::StructuredVolumeObject::Rectilinear:
        this->setGridType("rectilinear");
        break;
    case kvs::StructuredVolumeObject::Curvilinear:
        this->setGridType("curvilinear");
        break;
*/
    default:
    {
        m_is_success = false;
        kvsMessageError("'uniform' grid type is only supported.");
        break;
    }
    }

    this->setVeclen( volume->veclen() );
    this->setResolution( volume->resolution() );
    this->setValues( volume->values() );

    return( this );
}


/*===========================================================================*/
/**
 *  @brief  Constructs a AVSField data from given object.
 *  @param  object [in] pointer to the structured volume object
 */
/*===========================================================================*/
StructuredVolumeExporter<kvs::AVSField>::StructuredVolumeExporter(
    const kvs::StructuredVolumeObject* object )
{
    this->exec( object );
}

/*===========================================================================*/
/**
 *  @brief  Exports object to a AVSField data.
 *  @param  object [in] pointer to the structured volume object
 */
/*===========================================================================*/
kvs::AVSField* StructuredVolumeExporter<kvs::AVSField>::exec(
    const kvs::ObjectBase* object )
{
    if ( !object )
    {
        m_is_success = false;
        kvsMessageError("Input object is NULL.");
        return( NULL );
    }

    // Cast to the structured volume object.
    const kvs::StructuredVolumeObject* volume = kvs::StructuredVolumeObject::DownCast( object );
    if ( !volume )
    {
        m_is_success = false;
        kvsMessageError("Input object is not structured volume object.");
        return( NULL );
    }

    const std::type_info& type = volume->values().typeInfo()->type();
    if ( type == typeid( kvs::Int8 ) )
    {
        this->setBits( 8 );
        this->setSigned( true );
        this->setDataType( kvs::AVSField::Byte );
    }
    else if ( type == typeid( kvs::UInt8 ) )
    {
        this->setBits( 8 );
        this->setSigned( false );
        this->setDataType( kvs::AVSField::Byte );
    }
    else if ( type == typeid( kvs::Int16 ) )
    {
        this->setBits( 16 );
        this->setSigned( true );
        this->setDataType( kvs::AVSField::Short );
    }
    else if ( type == typeid( kvs::UInt16 ) )
    {
        this->setBits( 16 );
        this->setSigned( false );
        this->setDataType( kvs::AVSField::Short );
    }
    else if ( type == typeid( kvs::Int32 ) )
    {
        this->setBits( 32 );
        this->setSigned( true );
        this->setDataType( kvs::AVSField::Integer );
    }
    else if ( type == typeid( kvs::UInt32 ) )
    {
        this->setBits( 32 );
        this->setSigned( false );
        this->setDataType( kvs::AVSField::Integer );
    }
    else if ( type == typeid( kvs::Real32 ) )
    {
        this->setBits( 32 );
        this->setSigned( true );
        this->setDataType( kvs::AVSField::Float );
    }
    else if ( type == typeid( kvs::Real64 ) )
    {
        this->setBits( 64 );
        this->setSigned( true );
        this->setDataType( kvs::AVSField::Double );
    }
    else
    {
        kvsMessageError("Unsupported data type '%s'.", volume->values().typeInfo()->typeName() );
    }

    // Check the grid type of the given structured volume object.
    switch ( volume->gridType() )
    {
    case kvs::StructuredVolumeObject::Uniform:
    {
        this->setFieldType( kvs::AVSField::Uniform );
        this->setValues( volume->values() );
        break;
    }
/*
    case kvs::StructuredVolumeObject::Rectilinear:
        this->setFieldType( kvs::AVSField::Rectilinear );
        this->setValues( volume->values() );
        this->setCoords( volume->coords() );
        break;
    case kvs::StructuredVolumeObject::Curvilinear:
        this->setFieldType( kvs::AVSField::Irregular );
        this->setValues( volume->values() );
        this->setCoords( volume->coords() );
        break;
*/
    default:
    {
        m_is_success = false;
        kvsMessageError("Unknown grid type.");
        break;
    }
    }

    this->setVeclen( volume->veclen() );
    this->setNSpace( 3 );
    this->setNDim( 3 );
    this->setDim( volume->resolution() );

    return( this );
}

} // end of namespace kvs
