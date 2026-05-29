/****************************************************************************/
/**
 *  @file ExtractVertices.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: ExtractVertices.cpp 631 2010-10-10 02:15:35Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "ExtractVertices.h"
#include <vismodule/VolumeObjectBase>
#include <vismodule/StructuredVolumeObject>


namespace vismodule
{

/*==========================================================================*/
/**
 *  @brief  Constructs a new empty ExtractVertices.
 */
/*==========================================================================*/
ExtractVertices::ExtractVertices( void )
    : MapperBase()
    , PointObject()
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new ExtractVertices.
 *  @param  volume [in] pointer to the structured volume object
 */
/*==========================================================================*/
ExtractVertices::ExtractVertices(
    const vismodule::VolumeObjectBase& volume )
    : MapperBase()
    , PointObject()
{
    this->exec( volume );
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new ExtractVertices.
 *  @param  volume [in] pointer to the structured volume object
 *  @param  transfer_function [in] transfer function
 */
/*==========================================================================*/
ExtractVertices::ExtractVertices(
    const vismodule::VolumeObjectBase& volume,
    const vismodule::TransferFunction& transfer_function )
    : MapperBase( transfer_function )
    , PointObject()
{
    this->exec( volume );
}

/*==========================================================================*/
/**
 *  Destroys the ExtractVertices.
 */
/*==========================================================================*/
ExtractVertices::~ExtractVertices( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the mapper process.
 *  @param  object [in] pointer to the volume object
 *  @return pointer to the point object
 */
/*===========================================================================*/
ExtractVertices::SuperClass* ExtractVertices::exec( const vismodule::ObjectBase& object )
{
    if ( !&object )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    const vismodule::VolumeObjectBase* volume = vismodule::VolumeObjectBase::DownCast( object );
    if ( !&volume )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is not volume dat.");
        return( NULL );
    }

    this->mapping( *volume );

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Mapping for the volume object.
 *  @param  volume [in] pointer to the volume object
 */
/*===========================================================================*/
void ExtractVertices::mapping(
    const vismodule::VolumeObjectBase& volume )
{
    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );

    this->calculate_coords();

    const std::type_info& type = volume.values().typeInfo()->type();
    if (      type == typeid( vismodule::Int8   ) ) { this->calculate_colors<vismodule::Int8  >(); }
    else if ( type == typeid( vismodule::Int16  ) ) { this->calculate_colors<vismodule::Int16 >(); }
    else if ( type == typeid( vismodule::Int32  ) ) { this->calculate_colors<vismodule::Int32 >(); }
    else if ( type == typeid( vismodule::Int64  ) ) { this->calculate_colors<vismodule::Int64 >(); }
    else if ( type == typeid( vismodule::UInt8  ) ) { this->calculate_colors<vismodule::UInt8 >(); }
    else if ( type == typeid( vismodule::UInt16 ) ) { this->calculate_colors<vismodule::UInt16>(); }
    else if ( type == typeid( vismodule::UInt32 ) ) { this->calculate_colors<vismodule::UInt32>(); }
    else if ( type == typeid( vismodule::UInt64 ) ) { this->calculate_colors<vismodule::UInt64>(); }
    else if ( type == typeid( vismodule::Real32 ) ) { this->calculate_colors<vismodule::Real32>(); }
    else if ( type == typeid( vismodule::Real64 ) ) { this->calculate_colors<vismodule::Real64>(); }
}

/*===========================================================================*/
/**
 *  @brief  Calculates the coordinate values.
 */
/*===========================================================================*/
void ExtractVertices::calculate_coords( void )
{
    const VolumeObjectBase::GridType& type = m_volume->gridType();

    if ( type == VolumeObjectBase::Uniform )
    {
       this->calculate_uniform_coords();
    }
    else if ( type == VolumeObjectBase::Rectilinear )
    {
        this->calculate_rectiliner_coords();
    }
    else
    {
        SuperClass::setCoords( m_volume->coords() );
    }
}

/*===========================================================================*/
/**
 *  @brief  Calculates the coordinate values of the uniform grid.
 */
/*===========================================================================*/
void ExtractVertices::calculate_uniform_coords( void )
{
    const vismodule::StructuredVolumeObject* volume
        = dynamic_cast<const vismodule::StructuredVolumeObject*>( m_volume );

    vismodule::ValueArray<float> coords( 3 * volume->nnodes() );
    float*                 coord = coords.pointer();

    const vismodule::Vector3ui resolution( volume->resolution() );
    const vismodule::Vector3f  volume_size( volume->maxObjectCoord() - volume->minObjectCoord() );
    const vismodule::Vector3ui ngrids( resolution - vismodule::Vector3ui( 1, 1, 1 ) );
    const vismodule::Vector3f  grid_size(
        volume_size.x() / static_cast<float>( ngrids.x() ),
        volume_size.y() / static_cast<float>( ngrids.y() ),
        volume_size.z() / static_cast<float>( ngrids.z() ) );

    for ( std::size_t k = 0; k < resolution.z(); ++k )
    {
        const float z =
            grid_size.z() * static_cast<float>( k );
        for ( std::size_t j = 0; j < resolution.y(); ++j )
        {
            const float y =
                grid_size.y() * static_cast<float>( j );
            for ( std::size_t i = 0; i < resolution.x(); ++i )
            {
                const float x =
                    grid_size.x() * static_cast<float>( i );

                *( coord++ ) = x;
                *( coord++ ) = y;
                *( coord++ ) = z;
            }
        }
    }

    SuperClass::setCoords( coords );
}

/*===========================================================================*/
/**
 *  @brief  Calculates the coordinate values of the rectilinear grid.
 */
/*===========================================================================*/
void ExtractVertices::calculate_rectiliner_coords( void )
{
    BaseClass::m_is_success = false;
    visModuleMessageError("Rectilinear volume has not yet supportted.");
}

/*===========================================================================*/
/**
 *  @brief  Calculates the color values.
 */
/*===========================================================================*/
template <typename T>
void ExtractVertices::calculate_colors( void )
{
    const vismodule::VolumeObjectBase* volume = m_volume;

    const T*       value = reinterpret_cast<const T*>( volume->values().pointer() );
    const T* const end   = value + volume->values().size();

    vismodule::ValueArray<vismodule::UInt8> colors( 3 * volume->nnodes() );
    vismodule::UInt8*                 color = colors.pointer();

    vismodule::ColorMap cmap( BaseClass::colorMap() );

    if ( !volume->hasMinMaxValues() ) { volume->updateMinMaxValues(); }

    const vismodule::Real64 min_value = volume->minValue();
    const vismodule::Real64 max_value = volume->maxValue();

    const vismodule::Real64 normalize_factor =
        static_cast<vismodule::Real64>( cmap.resolution() - 1 ) / ( max_value - min_value );

    const std::size_t veclen = m_volume->veclen();

    if ( veclen == 1 )
    {
        while( value < end )
        {
            const vismodule::UInt32 color_level =
                static_cast<vismodule::UInt32>( normalize_factor * ( static_cast<vismodule::Real64>( *( value++ ) ) - min_value ) );

            *( color++ ) = cmap[ color_level ].red();
            *( color++ ) = cmap[ color_level ].green();
            *( color++ ) = cmap[ color_level ].blue();
        }
    }
    else
    {
        while( value < end )
        {
            vismodule::Real64 magnitude = 0.0;
            for ( std::size_t i = 0; i < veclen; ++i )
            {
                magnitude += vismodule::Math::Square( static_cast<vismodule::Real64>( *value ) );
                ++value;
            }
            magnitude = vismodule::Math::SquareRoot( magnitude );

            const vismodule::UInt32 color_level =
                static_cast<vismodule::UInt32>( normalize_factor * ( magnitude - min_value ) );

            *( color++ ) = cmap[ color_level ].red();
            *( color++ ) = cmap[ color_level ].green();
            *( color++ ) = cmap[ color_level ].blue();
        }
    }

    SuperClass::setColors( colors );
}

} // end of namespace vismodule
