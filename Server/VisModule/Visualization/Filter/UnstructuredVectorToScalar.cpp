/*****************************************************************************/
/**
 *  @file   UnstructuredVectorToScalar.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: UnstructuredVectorToScalar.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "UnstructuredVectorToScalar.h"
#include <vismodule/Math>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new UnstructuredVectorToScalar class.
 */
/*===========================================================================*/
UnstructuredVectorToScalar::UnstructuredVectorToScalar( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new UnstructuredVectorToScalar class.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
UnstructuredVectorToScalar::UnstructuredVectorToScalar( const vismodule::UnstructuredVolumeObject& volume )
{
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the UnstructuredVolumeObject class.
 */
/*===========================================================================*/
UnstructuredVectorToScalar::~UnstructuredVectorToScalar( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Main routine.
 *  @param  object [in] pointer to the object
 *  @return pointer to the converted unstructured volume object
 */
/*===========================================================================*/
UnstructuredVectorToScalar::SuperClass* UnstructuredVectorToScalar::exec( const vismodule::ObjectBase& object )
{
    if ( !&object )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    const vismodule::UnstructuredVolumeObject* volume = vismodule::UnstructuredVolumeObject::DownCast( object );
    if ( !&volume )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is not supported.");
        return( NULL );
    }

    const std::type_info& type = volume->values().typeInfo()->type();
    if (      type == typeid( vismodule::Int8   ) ) this->calculate_magnitude<vismodule::Int8>(   *volume );
    else if ( type == typeid( vismodule::Int16  ) ) this->calculate_magnitude<vismodule::Int16>(  *volume );
    else if ( type == typeid( vismodule::Int32  ) ) this->calculate_magnitude<vismodule::Int32>(  *volume );
    else if ( type == typeid( vismodule::Int64  ) ) this->calculate_magnitude<vismodule::Int64>(  *volume );
    else if ( type == typeid( vismodule::UInt8  ) ) this->calculate_magnitude<vismodule::UInt8>(  *volume );
    else if ( type == typeid( vismodule::UInt16 ) ) this->calculate_magnitude<vismodule::UInt16>( *volume );
    else if ( type == typeid( vismodule::UInt32 ) ) this->calculate_magnitude<vismodule::UInt32>( *volume );
    else if ( type == typeid( vismodule::UInt64 ) ) this->calculate_magnitude<vismodule::UInt64>( *volume );
    else if ( type == typeid( vismodule::Real32 ) ) this->calculate_magnitude<vismodule::Real32>( *volume );
    else if ( type == typeid( vismodule::Real64 ) ) this->calculate_magnitude<vismodule::Real64>( *volume );
    else
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Unsupported data type '%s'.", volume->values().typeInfo()->typeName() );
        return( NULL );
    }

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Calculates magnitude of the vector value
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*===========================================================================*/
template <typename T>
void UnstructuredVectorToScalar::calculate_magnitude( const vismodule::UnstructuredVolumeObject& volume )
{
    const std::size_t veclen = volume.veclen();
    const std::size_t nnodes = volume.nnodes();

    vismodule::AnyValueArray values;
    vismodule::Real32* dst = static_cast<vismodule::Real32*>( values.allocate<vismodule::Real32>( nnodes ) );
    const T* src = static_cast<const T*>( volume.values().pointer() );

    for ( std::size_t i = 0; i < nnodes; i++ )
    {
        vismodule::Real32 magnitude = 0;
        for ( std::size_t j = 0; j < veclen; j++ )
        {
            magnitude += vismodule::Math::Square( static_cast<vismodule::Real32>( *(src++) ) );
        }

        *(dst++) = vismodule::Math::SquareRoot( magnitude );
    }

    SuperClass::setVeclen( 1 );
    SuperClass::setNNodes( volume.nnodes() );
    SuperClass::setNCells( volume.ncells() );
    SuperClass::setCellType( volume.cellType() );
    SuperClass::setValues( values );
    SuperClass::setCoords( volume.coords() );
    SuperClass::setConnections( volume.connections() );
    SuperClass::updateMinMaxValues();
    SuperClass::updateMinMaxCoords();
}

} // end of namespace vismodule
