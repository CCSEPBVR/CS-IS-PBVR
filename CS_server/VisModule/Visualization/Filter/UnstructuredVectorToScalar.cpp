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
#include <kvs/Math>


namespace kvs
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
UnstructuredVectorToScalar::UnstructuredVectorToScalar( const kvs::UnstructuredVolumeObject* volume )
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
UnstructuredVectorToScalar::SuperClass* UnstructuredVectorToScalar::exec( const kvs::ObjectBase* object )
{
    if ( !object )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input object is NULL.");
        return( NULL );
    }

    const kvs::UnstructuredVolumeObject* volume = kvs::UnstructuredVolumeObject::DownCast( object );
    if ( !volume )
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Input object is not supported.");
        return( NULL );
    }

    const std::type_info& type = volume->values().typeInfo()->type();
    if (      type == typeid( kvs::Int8   ) ) this->calculate_magnitude<kvs::Int8>(   volume );
    else if ( type == typeid( kvs::Int16  ) ) this->calculate_magnitude<kvs::Int16>(  volume );
    else if ( type == typeid( kvs::Int32  ) ) this->calculate_magnitude<kvs::Int32>(  volume );
    else if ( type == typeid( kvs::Int64  ) ) this->calculate_magnitude<kvs::Int64>(  volume );
    else if ( type == typeid( kvs::UInt8  ) ) this->calculate_magnitude<kvs::UInt8>(  volume );
    else if ( type == typeid( kvs::UInt16 ) ) this->calculate_magnitude<kvs::UInt16>( volume );
    else if ( type == typeid( kvs::UInt32 ) ) this->calculate_magnitude<kvs::UInt32>( volume );
    else if ( type == typeid( kvs::UInt64 ) ) this->calculate_magnitude<kvs::UInt64>( volume );
    else if ( type == typeid( kvs::Real32 ) ) this->calculate_magnitude<kvs::Real32>( volume );
    else if ( type == typeid( kvs::Real64 ) ) this->calculate_magnitude<kvs::Real64>( volume );
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError("Unsupported data type '%s'.", volume->values().typeInfo()->typeName() );
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
void UnstructuredVectorToScalar::calculate_magnitude( const kvs::UnstructuredVolumeObject* volume )
{
    const size_t veclen = volume->veclen();
    const size_t nnodes = volume->nnodes();

    kvs::AnyValueArray values;
    kvs::Real32* dst = static_cast<kvs::Real32*>( values.allocate<kvs::Real32>( nnodes ) );
    const T* src = static_cast<const T*>( volume->values().pointer() );

    for ( size_t i = 0; i < nnodes; i++ )
    {
        kvs::Real32 magnitude = 0;
        for ( size_t j = 0; j < veclen; j++ )
        {
            magnitude += kvs::Math::Square( static_cast<kvs::Real32>( *(src++) ) );
        }

        *(dst++) = kvs::Math::SquareRoot( magnitude );
    }

    SuperClass::setVeclen( 1 );
    SuperClass::setNNodes( volume->nnodes() );
    SuperClass::setNCells( volume->ncells() );
    SuperClass::setCellType( volume->cellType() );
    SuperClass::setValues( values );
    SuperClass::setCoords( volume->coords() );
    SuperClass::setConnections( volume->connections() );
    SuperClass::updateMinMaxValues();
    SuperClass::updateMinMaxCoords();
}

} // end of namespace kvs
