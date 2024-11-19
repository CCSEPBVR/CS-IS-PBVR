/****************************************************************************/
/**
 *  @file CellByCellUniformSampling.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellByCellUniformSampling.cpp 633 2010-10-10 05:12:37Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "CellByCellUniformSampling.h"
#include <vector>
#include <vismodule/DebugNew>
#include <vismodule/ObjectManager>
#include <vismodule/Camera>
#include <vismodule/TrilinearInterpolator>
#include <vismodule/Value>
#include "CellBase.h"
#include "TetrahedralCell.h"
#include "QuadraticTetrahedralCell.h"
#include "HexahedralCell.h"
#include "QuadraticHexahedralCell.h"
#include "PyramidalCell.h"
#include <vismodule/GlobalCore>


namespace Generator = vismodule::CellByCellParticleGenerator;


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellUniformSampling class.
 */
/*===========================================================================*/
CellByCellUniformSampling::CellByCellUniformSampling( void ):
    vismodule::MapperBase(),
    vismodule::PointObject(),
    m_camera( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellUniformSampling class.
 *  @param  volume [in] pointer to the volume object
 *  @param  subpixel_level [in] sub-pixel level
 *  @param  sampling_step [in] sapling step
 *  @param  transfer_function [in] transfer function
 *  @param  object_depth [in] depth value of the input volume at the CoG
 */
/*===========================================================================*/
CellByCellUniformSampling::CellByCellUniformSampling(
    const vismodule::VolumeObjectBase* volume,
    const size_t                 subpixel_level,
    const float                  sampling_step,
    const vismodule::TransferFunction& transfer_function,
    const float                  object_depth ):
    vismodule::MapperBase( transfer_function ),
    vismodule::PointObject(),
    m_camera( 0 )
{
    this->setSubpixelLevel( subpixel_level );
    this->setSamplingStep( sampling_step );
    this->setObjectDepth( object_depth );
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellUniformSampling class.
 *  @param  camera [in] pointer to the camera
 *  @param  volume [in] pointer to the volume object
 *  @param  subpixel_level [in] sub-pixel level
 *  @param  sampling_step [in] sapling step
 *  @param  transfer_function [in] transfer function
 *  @param  object_depth [in] depth value of the input volume at the CoG
 */
/*===========================================================================*/
CellByCellUniformSampling::CellByCellUniformSampling(
    const vismodule::Camera*           camera,
    const vismodule::VolumeObjectBase* volume,
    const size_t                 subpixel_level,
    const float                  sampling_step,
    const vismodule::TransferFunction& transfer_function,
    const float                  object_depth ):
    vismodule::MapperBase( transfer_function ),
    vismodule::PointObject()
{
    this->attachCamera( camera ),
    this->setSubpixelLevel( subpixel_level );
    this->setSamplingStep( sampling_step );
    this->setObjectDepth( object_depth );
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Destroys the CellByCellMetropolisSampling class.
 */
/*===========================================================================*/
CellByCellUniformSampling::~CellByCellUniformSampling( void )
{
    m_density_map.deallocate();
}

/*===========================================================================*/
/**
 *  @brief  Retruns the sub-pixel level.
 *  @return sub-pixel level
 */
/*===========================================================================*/
const size_t CellByCellUniformSampling::subpixelLevel( void ) const
{
    return( m_subpixel_level );
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampling step.
 *  @return sampling step
 */
/*===========================================================================*/
const float CellByCellUniformSampling::samplingStep( void ) const
{
    return( m_sampling_step );
}

/*===========================================================================*/
/**
 *  @brief  Returns the depth of the object at the center of the gravity.
 *  @return depth
 */
/*===========================================================================*/
const float CellByCellUniformSampling::objectDepth( void ) const
{
    return( m_object_depth );
}

/*===========================================================================*/
/**
 *  @brief  Attaches a camera.
 *  @param  camera [in] pointer to the camera
 */
/*===========================================================================*/
void CellByCellUniformSampling::attachCamera( const vismodule::Camera* camera )
{
    m_camera = camera;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sub-pixel level.
 *  @param  subpixel_level [in] sub-pixel level
 */
/*===========================================================================*/
void CellByCellUniformSampling::setSubpixelLevel( const size_t subpixel_level )
{
    m_subpixel_level = subpixel_level;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sampling step.
 *  @param  sampling_step [in] sampling step
 */
/*===========================================================================*/
void CellByCellUniformSampling::setSamplingStep( const float sampling_step )
{
    m_sampling_step = sampling_step;
}

/*===========================================================================*/
/**
 *  @brief  Sets a depth of the object at the center of the gravity.
 *  @param  object_depth [in] depth
 */
/*===========================================================================*/
void CellByCellUniformSampling::setObjectDepth( const float object_depth )
{
    m_object_depth = object_depth;
}

/*===========================================================================*/
/**
 *  @brief  Executes the mapper process.
 *  @param  object [in] pointer to the volume object
 *  @return pointer to the point object
 */
/*===========================================================================*/
CellByCellUniformSampling::SuperClass* CellByCellUniformSampling::exec( const vismodule::ObjectBase* object )
{
    if ( !object )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is NULL.");
        return( NULL );
    }

    const vismodule::VolumeObjectBase* volume = vismodule::VolumeObjectBase::DownCast( object );
    if ( !volume )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Input object is not volume dat.");
        return( NULL );
    }

    const vismodule::VolumeObjectBase::VolumeType volume_type = volume->volumeType();
    if ( volume_type == vismodule::VolumeObjectBase::Structured )
    {
//        const vismodule::Camera* camera = ( !m_camera ) ? vismodule::GlobalCore::camera : m_camera;
//        this->mapping( camera, reinterpret_cast<const vismodule::StructuredVolumeObject*>( object ) );
        if ( m_camera )
        {
            this->mapping( m_camera, reinterpret_cast<const vismodule::StructuredVolumeObject*>( object ) );
        }
        else
        {
            // Generate particles by using default camera parameters.
            if ( vismodule::GlobalCore::camera )
            {
                if ( vismodule::GlobalCore::camera->windowWidth() != 0 && vismodule::GlobalCore::camera->windowHeight() )
                {
                    const vismodule::Camera* camera = vismodule::GlobalCore::camera;
                    this->mapping( camera, reinterpret_cast<const vismodule::StructuredVolumeObject*>( object ) );
                }
            }
            else
            {
                vismodule::Camera* camera = new vismodule::Camera();
                this->mapping( camera, reinterpret_cast<const vismodule::StructuredVolumeObject*>( object ) );
                delete camera;
            }
        }
    }
    else // volume_type == vismodule::VolumeObjectBase::Unstructured
    {
//        const vismodule::Camera* camera = ( !m_camera ) ? vismodule::GlobalCore::camera : m_camera;
//        this->mapping( camera, reinterpret_cast<const vismodule::UnstructuredVolumeObject*>( object ) );
        if ( m_camera )
        {
            this->mapping( m_camera, reinterpret_cast<const vismodule::UnstructuredVolumeObject*>( object ) );
        }
        else
        {
            // Generate particles by using default camera parameters.
            if ( vismodule::GlobalCore::camera )
            {
                if ( vismodule::GlobalCore::camera->windowWidth() != 0 && vismodule::GlobalCore::camera->windowHeight() )
                {
                    const vismodule::Camera* camera = vismodule::GlobalCore::camera;
                    this->mapping( camera, reinterpret_cast<const vismodule::UnstructuredVolumeObject*>( object ) );
                }
            }
            else
            {
                vismodule::Camera* camera = new vismodule::Camera();
                this->mapping( camera, reinterpret_cast<const vismodule::UnstructuredVolumeObject*>( object ) );
                delete camera;
            }
        }
    }

    return( this );
}

/*===========================================================================*/
/**
 *  @brief  Mapping for the structured volume object.
 *  @param  camera [in] pointer to the camera
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
void CellByCellUniformSampling::mapping( const vismodule::Camera* camera, const vismodule::StructuredVolumeObject* volume )
{
    // Attach the pointer to the volume object and set the min/max coordinates.
    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );

    // Calculate the density map.
    m_density_map = Generator::CalculateDensityMap(
        camera,
        BaseClass::volume(),
        static_cast<float>( m_subpixel_level ),
        m_sampling_step,
        BaseClass::transferFunction().opacityMap() );

    // Generate the particles.
    const std::type_info& type = volume->values().typeInfo()->type();
    if (      type == typeid( vismodule::Int8   ) ) this->generate_particles<vismodule::Int8>( volume );
    else if ( type == typeid( vismodule::Int16  ) ) this->generate_particles<vismodule::Int16>( volume );
    else if ( type == typeid( vismodule::Int32  ) ) this->generate_particles<vismodule::Int32>( volume );
    else if ( type == typeid( vismodule::UInt8  ) ) this->generate_particles<vismodule::UInt8>( volume );
    else if ( type == typeid( vismodule::UInt16 ) ) this->generate_particles<vismodule::UInt16>( volume );
    else if ( type == typeid( vismodule::UInt32 ) ) this->generate_particles<vismodule::UInt32>( volume );
    else if ( type == typeid( vismodule::Real32 ) ) this->generate_particles<vismodule::Real32>( volume );
    else if ( type == typeid( vismodule::Real64 ) ) this->generate_particles<vismodule::Real64>( volume );
    else
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Unsupported data type '%s'.", volume->values().typeInfo()->typeName() );
    }
}

/*===========================================================================*/
/**
 *  @brief  Mapping for the unstructured volume object.
 *  @param  camera [in] pointer to the camera
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
void CellByCellUniformSampling::mapping( const vismodule::Camera* camera, const vismodule::UnstructuredVolumeObject* volume )
{
    // Attach the pointer to the volume object and set the min/max coordinates.
    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );

    // Calculate the density map.
    m_density_map = Generator::CalculateDensityMap(
        camera,
        BaseClass::volume(),
        static_cast<float>( m_subpixel_level ),
        m_sampling_step,
        BaseClass::transferFunction().opacityMap() );

    // Generate the particles.
//    if ( !volume->hasMinMaxValues() ) volume->updateMinMaxValues();
//    const float min_value = static_cast<float>( volume->minValue() );
//    const float max_value = static_cast<float>( volume->maxValue() );
    const std::type_info& type = volume->values().typeInfo()->type();
    if (      type == typeid( vismodule::Int8   ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( -128, 127 );
        this->generate_particles<vismodule::Int8>( volume );
    }
    else if ( type == typeid( vismodule::Int16  ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<vismodule::Int16>( volume );
    }
    else if ( type == typeid( vismodule::Int32  ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<vismodule::Int32>( volume );
    }
    else if ( type == typeid( vismodule::Int64  ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<vismodule::Int64>( volume );
    }
    else if ( type == typeid( vismodule::UInt8  ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( 0, 255 );
        this->generate_particles<vismodule::UInt8>( volume );
    }
    else if ( type == typeid( vismodule::UInt16 ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<vismodule::UInt16>( volume );
    }
    else if ( type == typeid( vismodule::UInt32 ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<vismodule::UInt32>( volume );
    }
    else if ( type == typeid( vismodule::UInt64 ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<vismodule::UInt64>( volume );
    }
    else if ( type == typeid( vismodule::Real32 ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<vismodule::Real32>( volume );
    }
    else if ( type == typeid( vismodule::Real64 ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<vismodule::Real64>( volume );
    }
    else
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Unsupported data type '%s'.", volume->values().typeInfo()->typeName() );
    }
}

/*===========================================================================*/
/**
 *  @brief  Generates particles for the structured volume object.
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
template <typename T>
void CellByCellUniformSampling::generate_particles( const vismodule::StructuredVolumeObject* volume )
{
    // Vertex data arrays. (output)
    std::vector<vismodule::Real32> vertex_coords;
    std::vector<vismodule::UInt8>  vertex_colors;
    std::vector<vismodule::Real32> vertex_normals;

    // Set a trilinear interpolator.
    vismodule::TrilinearInterpolator interpolator( volume );

    // Set parameters for normalization of the node values.
    const float min_value = BaseClass::transferFunction().colorMap().minValue();
    const float max_value = BaseClass::transferFunction().colorMap().maxValue();
    const float normalize_factor = BaseClass::transferFunction().resolution() / ( max_value - min_value );

    const float* const  density_map = m_density_map.pointer();
    const vismodule::ColorMap color_map( BaseClass::transferFunction().colorMap() );

    // Generate particles for each cell.
    const vismodule::Vector3ui ncells( volume->resolution() - vismodule::Vector3ui(1) );
    for ( vismodule::UInt32 z = 0; z < ncells.z(); ++z )
    {
        for ( vismodule::UInt32 y = 0; y < ncells.y(); ++y )
        {
            for ( vismodule::UInt32 x = 0; x < ncells.x(); ++x )
            {
                // Calculate a volume of cell.
                const float volume_of_cell = 1.0f;

                // Interpolate at the center of gravity of this cell.
                const vismodule::Vector3f cog( x + 0.5f, y + 0.5f, z + 0.5f );
                interpolator.attachPoint( cog );

                // Calculate a density.
                const float  average_scalar = interpolator.scalar<T>();
                const size_t average_degree = static_cast<size_t>( ( average_scalar - min_value ) * normalize_factor );
                const float  density = density_map[ average_degree ];

                // Calculate a number of particles in this cell.
                const float p = density * volume_of_cell;
                size_t nparticles_in_cell = static_cast<size_t>( p );
                if ( p - nparticles_in_cell > Generator::GetRandomNumber() ) { ++nparticles_in_cell; }

                const vismodule::Vector3f v( static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) );
                for ( size_t particle = 0; particle < nparticles_in_cell; ++particle )
                {
                    // Calculate a coord.
                    const vismodule::Vector3f coord( Generator::RandomSamplingInCube( v ) );

                    // Calculate a color.
                    interpolator.attachPoint( coord );
                    const float scalar = interpolator.scalar<T>();
                    const vismodule::RGBColor color( color_map.at( scalar ) );

                    // Calculate a normal.
                    const Vector3f normal( interpolator.gradient<T>() );

                    // set coord, color, and normal to point object( this ).
                    vertex_coords.push_back( coord.x() );
                    vertex_coords.push_back( coord.y() );
                    vertex_coords.push_back( coord.z() );

                    vertex_colors.push_back( color.r() );
                    vertex_colors.push_back( color.g() );
                    vertex_colors.push_back( color.b() );

                    vertex_normals.push_back( normal.x() );
                    vertex_normals.push_back( normal.y() );
                    vertex_normals.push_back( normal.z() );
                } // end of 'paricle' for-loop
            } // end of 'x' loop
        } // end of 'y' loop
    } // end of 'z' loop

    SuperClass::m_coords  = vismodule::ValueArray<vismodule::Real32>( vertex_coords );
    SuperClass::m_colors  = vismodule::ValueArray<vismodule::UInt8>( vertex_colors );
    SuperClass::m_normals = vismodule::ValueArray<vismodule::Real32>( vertex_normals );
    SuperClass::setSize( 1.0f );
}

/*===========================================================================*/
/**
 *  @brief  Generates particles for the unstructured volume object.
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
template <typename T>
void CellByCellUniformSampling::generate_particles( const vismodule::UnstructuredVolumeObject* volume )
{
    // Vertex data arrays. (output)
    std::vector<vismodule::Real32> vertex_coords;
    std::vector<vismodule::UInt8>  vertex_colors;
    std::vector<vismodule::Real32> vertex_normals;

    // Set a tetrahedral cell interpolator.
    vismodule::CellBase<T>* cell = NULL;
    switch ( volume->cellType() )
    {
    case vismodule::VolumeObjectBase::Tetrahedra:
    {
        cell = new vismodule::TetrahedralCell<T>( volume );
        break;
    }
    case vismodule::VolumeObjectBase::QuadraticTetrahedra:
    {
        cell = new vismodule::QuadraticTetrahedralCell<T>( volume );
        break;
    }
    case vismodule::VolumeObjectBase::Hexahedra:
    {
        cell = new vismodule::HexahedralCell<T>( volume );
        break;
    }
    case vismodule::VolumeObjectBase::QuadraticHexahedra:
    {
        cell = new vismodule::QuadraticHexahedralCell<T>( volume );
        break;
    }
    case vismodule::VolumeObjectBase::Pyramid:
    {
        cell = new vismodule::PyramidalCell<T>( volume );
        break;
    }
    default:
    {
        BaseClass::m_is_success = false;
        visModuleMessageError("Unsupported cell type.");
        return;
    }
    }

//    const float min_value = ( typeid(T) == typeid( vismodule::UInt8 ) ) ? 0.0f : static_cast<float>( volume->minValue() );
//    const float max_value = ( typeid(T) == typeid( vismodule::UInt8 ) ) ? 255.0f : static_cast<float>( volume->maxValue() );
    const float min_value = BaseClass::transferFunction().colorMap().minValue();
    const float max_value = BaseClass::transferFunction().colorMap().maxValue();
    const float max_range = static_cast<float>( BaseClass::transferFunction().resolution() - 1 );
    const float normalize_factor = max_range / ( max_value - min_value );

    const float* const  density_map = m_density_map.pointer();
    const vismodule::ColorMap color_map( BaseClass::transferFunction().colorMap() );

    // Generate particles for each cell.
    const size_t ncells = volume->ncells();
    for ( size_t index = 0; index < ncells; ++index )
    {
        // Bind the cell which is indicated by 'index'.
        cell->bindCell( index );

        // Calculate a density.
        const float  average_scalar = cell->averagedScalar();
        const size_t average_degree = static_cast<size_t>( ( average_scalar - min_value ) * normalize_factor );
        const float  density = density_map[ average_degree ];

        // Calculate a number of particles in this cell.
        const float volume_of_cell = cell->volume();
        const float p = density * volume_of_cell;
        size_t nparticles_in_cell = static_cast<size_t>( p );

        if ( p - nparticles_in_cell > Generator::GetRandomNumber() ) { ++nparticles_in_cell; }

        // Generate a set of particles in this cell represented by v0,...,v3 and s0,...,s3.
        for ( size_t particle = 0; particle < nparticles_in_cell; ++particle )
        {
            // Calculate a coord.
            const vismodule::Vector3f coord = cell->randomSampling();

            // Calculate a color.
            const float scalar = cell->scalar();
            const vismodule::RGBColor color( color_map.at( scalar ) );

            // Calculate a normal.
            /* NOTE: The gradient vector of the cell is reversed for shading on the rendering process.
             */
            const Vector3f normal( -cell->gradient() );

            // set coord, color, and normal to point object( this ).
            vertex_coords.push_back( coord.x() );
            vertex_coords.push_back( coord.y() );
            vertex_coords.push_back( coord.z() );

            vertex_colors.push_back( color.r() );
            vertex_colors.push_back( color.g() );
            vertex_colors.push_back( color.b() );

            vertex_normals.push_back( normal.x() );
            vertex_normals.push_back( normal.y() );
            vertex_normals.push_back( normal.z() );
        } // end of 'paricle' for-loop
    } // end of 'cell' for-loop

    SuperClass::m_coords  = vismodule::ValueArray<vismodule::Real32>( vertex_coords );
    SuperClass::m_colors  = vismodule::ValueArray<vismodule::UInt8>( vertex_colors );
    SuperClass::m_normals = vismodule::ValueArray<vismodule::Real32>( vertex_normals );
    SuperClass::setSize( 1.0f );

    delete cell;
}

} // end of namespace vismodule
