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
#include <vismodule/ObjectBase>
#include <vismodule/VolumeObjectBase>
#include <vismodule/StructuredVolumeObject>


namespace
{

/*===========================================================================*/
/**
 *  @brief  Casts to a pointer to the structured volume object.
 *  @param  object [in] pointer to the object
 *  @return pointer to the structured volume object
 */
/*===========================================================================*/
const vismodule::StructuredVolumeObject* CastToStructuredVolumeObject( const vismodule::ObjectBase& object )
{
    if ( object.objectType() != vismodule::ObjectBase::Volume )
    {
        visModuleMessageError("Input object is not a volumetry object.");
        return( NULL );
    }

    const vismodule::VolumeObjectBase* volume =
        reinterpret_cast<const vismodule::VolumeObjectBase*>( &object );
    if ( volume->volumeType() != vismodule::VolumeObjectBase::Structured )
    {
        visModuleMessageError("Input object is not a structured volume object.");
        return( NULL );
    }

    return( reinterpret_cast<const vismodule::StructuredVolumeObject*>( &volume ) );
}

} // end of namespace


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a KVSMLObjectStructuredVolume data from given object.
 *  @param  object [in] pointer to the structured volume object
 */
/*===========================================================================*/
StructuredVolumeExporter<vismodule::KVSMLObjectStructuredVolume>::StructuredVolumeExporter(
    const vismodule::StructuredVolumeObject& object )
{
    this->exec( object );
}

/*===========================================================================*/
/**
 *  @brief  Exports object to a KVSMLObjectStructuredVolume data.
 *  @param  object [in] pointer to the structured volume object
 */
/*===========================================================================*/
vismodule::KVSMLObjectStructuredVolume* StructuredVolumeExporter<vismodule::KVSMLObjectStructuredVolume>::exec(
    const vismodule::ObjectBase& object )
{
    if ( !&object )
    {
        m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    // Cast to the structured volume object.
    const vismodule::StructuredVolumeObject* volume = vismodule::StructuredVolumeObject::DownCast( object );
    if ( !&volume )
    {
        m_is_success = false;
        visModuleMessageError("Input object is not structured volume object.");
        return( NULL );
    }

    // Check the grid type of the given structured volume object.
    switch ( volume->gridType() )
    {
    case vismodule::StructuredVolumeObject::UnknownGridType:
    {
        visModuleMessageError("Unknown grid type.");
        break;
    }
    case vismodule::StructuredVolumeObject::Uniform:
    {
        this->setGridType("uniform");
        break;
    }
/*
    case vismodule::StructuredVolumeObject::Rectilinear:
        this->setGridType("rectilinear");
        break;
    case vismodule::StructuredVolumeObject::Curvilinear:
        this->setGridType("curvilinear");
        break;
*/
    default:
    {
        m_is_success = false;
        visModuleMessageError("'uniform' grid type is only supported.");
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
StructuredVolumeExporter<vismodule::AVSField>::StructuredVolumeExporter(
    const vismodule::StructuredVolumeObject& object )
{
    this->exec( object );
}

/*===========================================================================*/
/**
 *  @brief  Exports object to a AVSField data.
 *  @param  object [in] pointer to the structured volume object
 */
/*===========================================================================*/
vismodule::AVSField* StructuredVolumeExporter<vismodule::AVSField>::exec(
    const vismodule::ObjectBase& object )
{
    if ( !&object )
    {
        m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    // Cast to the structured volume object.
    const vismodule::StructuredVolumeObject* volume = vismodule::StructuredVolumeObject::DownCast( object );
    if ( !&volume )
    {
        m_is_success = false;
        visModuleMessageError("Input object is not structured volume object.");
        return( NULL );
    }

    const std::type_info& type = volume->values().typeInfo()->type();
    if ( type == typeid( vismodule::Int8 ) )
    {
        this->setBits( 8 );
        this->setSigned( true );
        this->setDataType( vismodule::AVSField::Byte );
    }
    else if ( type == typeid( vismodule::UInt8 ) )
    {
        this->setBits( 8 );
        this->setSigned( false );
        this->setDataType( vismodule::AVSField::Byte );
    }
    else if ( type == typeid( vismodule::Int16 ) )
    {
        this->setBits( 16 );
        this->setSigned( true );
        this->setDataType( vismodule::AVSField::Short );
    }
    else if ( type == typeid( vismodule::UInt16 ) )
    {
        this->setBits( 16 );
        this->setSigned( false );
        this->setDataType( vismodule::AVSField::Short );
    }
    else if ( type == typeid( vismodule::Int32 ) )
    {
        this->setBits( 32 );
        this->setSigned( true );
        this->setDataType( vismodule::AVSField::Integer );
    }
    else if ( type == typeid( vismodule::UInt32 ) )
    {
        this->setBits( 32 );
        this->setSigned( false );
        this->setDataType( vismodule::AVSField::Integer );
    }
    else if ( type == typeid( vismodule::Real32 ) )
    {
        this->setBits( 32 );
        this->setSigned( true );
        this->setDataType( vismodule::AVSField::Float );
    }
    else if ( type == typeid( vismodule::Real64 ) )
    {
        this->setBits( 64 );
        this->setSigned( true );
        this->setDataType( vismodule::AVSField::Double );
    }
    else
    {
        visModuleMessageError("Unsupported data type '%s'.", volume->values().typeInfo()->typeName() );
    }

    // Check the grid type of the given structured volume object.
    switch ( volume->gridType() )
    {
    case vismodule::StructuredVolumeObject::Uniform:
    {
        this->setFieldType( vismodule::AVSField::Uniform );
        this->setValues( volume->values() );
        break;
    }
/*
    case vismodule::StructuredVolumeObject::Rectilinear:
        this->setFieldType( vismodule::AVSField::Rectilinear );
        this->setValues( volume->values() );
        this->setCoords( volume->coords() );
        break;
    case vismodule::StructuredVolumeObject::Curvilinear:
        this->setFieldType( vismodule::AVSField::Irregular );
        this->setValues( volume->values() );
        this->setCoords( volume->coords() );
        break;
*/
    default:
    {
        m_is_success = false;
        visModuleMessageError("Unknown grid type.");
        break;
    }
    }

    this->setVeclen( volume->veclen() );
    this->setNSpace( 3 );
    this->setNDim( 3 );
    this->setDim( volume->resolution() );

    return( this );
}

} // end of namespace vismodule
