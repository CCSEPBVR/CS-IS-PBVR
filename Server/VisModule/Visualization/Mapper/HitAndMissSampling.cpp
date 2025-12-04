/****************************************************************************/
/**
 *  @file HitAndMissSampling.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: HitAndMissSampling.cpp 602 2010-08-19 02:43:34Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "HitAndMissSampling.h"
#include <vismodule/MersenneTwister>
#include <vismodule/IgnoreUnusedVariable>
#include <vector>


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new HitAndMissSampling class.
 */
/*===========================================================================*/
HitAndMissSampling::HitAndMissSampling( void ):
    vismodule::MapperBase(),
    vismodule::PointObject()
{
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new HitAndMissSampling class.
 *  @param  volume [in] pointer to the volume object
 */
/*==========================================================================*/
HitAndMissSampling::HitAndMissSampling( const vismodule::VolumeObjectBase& volume ):
    vismodule::MapperBase(),
    vismodule::PointObject()
{
    this->exec( volume );
}

/*==========================================================================*/
/**
 *  @brief  Constructs a new HitAndMissSampling class.
 *  @param  volume [in] pointer to the volume object
 *  @param  transfer_function [in] transfer function
 */
/*==========================================================================*/
HitAndMissSampling::HitAndMissSampling(
    const vismodule::VolumeObjectBase& volume,
    const vismodule::TransferFunction& transfer_function ):
    vismodule::MapperBase( transfer_function ),
    vismodule::PointObject()
{
    this->exec( volume );
}

/*==========================================================================*/
/**
 *  @brief  Destroys the HitAndMissSampling class.
 */
/*==========================================================================*/
HitAndMissSampling::~HitAndMissSampling( void )
{
}

/*===========================================================================*/
/**
 *  @brief  Executes the mapper process.
 *  @param  object [in] pointer to the volume object
 *  @return pointer to the point object
 */
/*===========================================================================*/
HitAndMissSampling::SuperClass* HitAndMissSampling::exec( const vismodule::ObjectBase& object )
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

    const vismodule::VolumeObjectBase::VolumeType volume_type = volume->volumeType();
    if ( volume_type == vismodule::VolumeObjectBase::Structured )
    {
        this->mapping( *reinterpret_cast<const vismodule::StructuredVolumeObject*>( &object ) );
    }
    else // volume_type == vismodule::VolumeObjectBase::Unstructured
    {
        this->mapping( *reinterpret_cast<const vismodule::UnstructuredVolumeObject*>( &object ) );
    }

    return( this );
}

/*==========================================================================*/
/**
 *  @brief  Mapping for the structured volume object.
 *  @param  volume [in] pointer to the structured volume object
 */
/*==========================================================================*/
void HitAndMissSampling::mapping( const vismodule::StructuredVolumeObject& volume )
{
    // Attach the pointer to the volume object.
    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );

    // Generate the particles.
    const std::type_info& type = volume.values().typeInfo()->type();
    if (      type == typeid( vismodule::UInt8  ) ) this->generate_particles<vismodule::UInt8>( volume );
    else if ( type == typeid( vismodule::UInt16 ) ) this->generate_particles<vismodule::UInt16>( volume );
    else
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Unsupported data type '%s'.", volume.values().typeInfo()->typeName() );
    }
}

/*==========================================================================*/
/**
 *  @brief  Mapping for the unstructured volume object.
 *  @param  volume [in] pointer to the unstructured volume object
 */
/*==========================================================================*/
void HitAndMissSampling::mapping( const vismodule::UnstructuredVolumeObject& volume )
{
    // Attach the pointer to the volume object.
    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );

    // Generate the particles.
    const std::type_info& type = volume.values().typeInfo()->type();
    if (      type == typeid( vismodule::Int8   ) ) this->generate_particles<vismodule::Int8>( volume );
    else if ( type == typeid( vismodule::Int16  ) ) this->generate_particles<vismodule::Int16>( volume );
    else if ( type == typeid( vismodule::Int32  ) ) this->generate_particles<vismodule::Int32>( volume );
    else if ( type == typeid( vismodule::Int64  ) ) this->generate_particles<vismodule::Int64>( volume );
    else if ( type == typeid( vismodule::UInt8  ) ) this->generate_particles<vismodule::UInt8>( volume );
    else if ( type == typeid( vismodule::UInt16 ) ) this->generate_particles<vismodule::UInt16>( volume );
    else if ( type == typeid( vismodule::UInt32 ) ) this->generate_particles<vismodule::UInt32>( volume );
    else if ( type == typeid( vismodule::UInt64 ) ) this->generate_particles<vismodule::UInt64>( volume );
    else if ( type == typeid( vismodule::Real32 ) ) this->generate_particles<vismodule::Real32>( volume );
    else if ( type == typeid( vismodule::Real64 ) ) this->generate_particles<vismodule::Real64>( volume );
    else
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Unsupported data type '%s'.", volume.values().typeInfo()->typeName() );
    }
}

/*==========================================================================*/
/**
 *  @brief  Generate particles for the structured volume object.
 *  @param  volume [in] pointer to the structured volume object
 */
/*==========================================================================*/
template <typename T>
void HitAndMissSampling::generate_particles( const vismodule::StructuredVolumeObject& volume  )
{
    // Set the geometry arrays.
    const size_t max_nparticles = volume.nnodes();
    std::vector<vismodule::Real32> coords;  coords.reserve( max_nparticles * 3 );
    std::vector<vismodule::UInt8>  colors;  colors.reserve( max_nparticles * 3 );
    std::vector<vismodule::Real32> normals; normals.reserve( max_nparticles * 3 );

    // Aliases.
    const vismodule::Vector3ui resolution = volume.resolution();
    const size_t line_size  = volume.nnodesPerLine();
    const size_t slice_size = volume.nnodesPerSlice();
    const T* values = reinterpret_cast<const T*>( volume.values().pointer() );

    vismodule::MersenneTwister R; // Random number generator
    size_t index = 0;     // index of voxel
    for ( size_t k = 0; k < resolution.z(); k++ )
    {
        for ( size_t j = 0; j < resolution.y(); j++ )
        {
            for ( size_t i = 0; i < resolution.x(); i++, index++ )
            {
                // Rejection.
                const size_t voxel_value = values[ index ];
                if( R() < BaseClass::opacityMap()[ voxel_value ] )
                {
                    // Set coordinate value.
                    coords.push_back( static_cast<vismodule::Real32>(i) );
                    coords.push_back( static_cast<vismodule::Real32>(j) );
                    coords.push_back( static_cast<vismodule::Real32>(k) );

                    // Set color value.
                    colors.push_back( BaseClass::colorMap()[ voxel_value ].r() );
                    colors.push_back( BaseClass::colorMap()[ voxel_value ].g() );
                    colors.push_back( BaseClass::colorMap()[ voxel_value ].b() );

                    // Calculate a normal vector at the node(i,j,k).
                    vismodule::Vector3ui front( index ); // front index
                    vismodule::Vector3ui back( index );  // back index

                    if(      i == 0                  ) front.x() += 1;
                    else if( i == resolution.x() - 1 ) back.x()  -= 1;
                    else{ front.x() += 1; back.x() -= 1; }

                    if(      j == 0                  ) front.y() += line_size;
                    else if( j == resolution.y() - 1 ) back.y()  -= line_size;
                    else{ front.y() += line_size; back.y() -= line_size; }

                    if(      k == 0                  ) front.z() += slice_size;
                    else if( k == resolution.z() - 1 ) back.z()  -= slice_size;
                    else{ front.z() += slice_size; back.z() -= slice_size; }

                    // Set normal vector.
                    normals.push_back( static_cast<vismodule::Real32>( values[ front.x() ] - values[ back.x() ] ) );
                    normals.push_back( static_cast<vismodule::Real32>( values[ front.y() ] - values[ back.y() ] ) );
                    normals.push_back( static_cast<vismodule::Real32>( values[ front.z() ] - values[ back.z() ] ) );
                }
            } // end of i-loop
        } // end of j-loop
    } // end of k-loop

    SuperClass::m_coords  = vismodule::ValueArray<vismodule::Real32>( coords );
    SuperClass::m_colors  = vismodule::ValueArray<vismodule::UInt8>( colors );
    SuperClass::m_normals = vismodule::ValueArray<vismodule::Real32>( normals );
    SuperClass::setSize( 1.0f );
}

template
void HitAndMissSampling::generate_particles<vismodule::UInt8>( const vismodule::StructuredVolumeObject& volume );

template
void HitAndMissSampling::generate_particles<vismodule::UInt16>( const vismodule::StructuredVolumeObject& volume );

/*==========================================================================*/
/**
 *  Generate particles for the unstructured volume object.
 *  @param volume [in] pointer to the unstructured volume object
 */
/*==========================================================================*/
template <typename T>
void HitAndMissSampling::generate_particles( const vismodule::UnstructuredVolumeObject& volume  )
{
    vismodule::IgnoreUnusedVariable( volume );

    BaseClass::m_is_success = false;
    visModuleMessageError("Not yet implemented the hit-and-miss method for the unstructured volume.");
}

} // end of namespace vismodule
