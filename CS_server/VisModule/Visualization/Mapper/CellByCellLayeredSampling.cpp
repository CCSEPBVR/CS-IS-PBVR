/*****************************************************************************/
/**
 *  @file   CellByCellLayeredSampling.cpp
 *  @author Naohisa Sakamoto
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellByCellLayeredSampling.cpp 633 2010-10-10 05:12:37Z naohisa.sakamoto $
 */
/*****************************************************************************/
#include "CellByCellLayeredSampling.h"
#include <vector>
#include <vismodule/DebugNew>
#include <vismodule/GlobalCore>
#include <vismodule/ObjectManager>
#include <vismodule/Camera>
#include <vismodule/ValueArray>
#include <vismodule/CellBase>
#include <vismodule/TetrahedralCell>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/MersenneTwister>

namespace Generator = vismodule::CellByCellParticleGenerator;

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellLayeredSampling class.
 */
/*===========================================================================*/
CellByCellLayeredSampling::CellByCellLayeredSampling( void ):
    vismodule::MapperBase(),
    vismodule::PointObject(),
    m_camera( 0 ),
    m_pregenerated_particles( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellLayeredSampling class.
 *  @param  volume [in] pointer to the volume object
 *  @param  subpixel_level [in] sub-pixel level
 *  @param  sampling_step [in] sapling step
 *  @param  transfer_function [in] transfer function
 *  @param  object_depth [in] depth value of the input volume at the CoG
 */
/*===========================================================================*/
CellByCellLayeredSampling::CellByCellLayeredSampling(
    const vismodule::VolumeObjectBase* volume,
    const size_t                 subpixel_level,
    const float                  sampling_step,
    const vismodule::TransferFunction& transfer_function,
    const float                  object_depth ):
    vismodule::MapperBase( transfer_function ),
    vismodule::PointObject(),
    m_camera( 0 ),
    m_pregenerated_particles( 0 )
{
    this->setSubpixelLevel( subpixel_level );
    this->setSamplingStep( sampling_step );
    this->setObjectDepth( object_depth );
    this->exec( volume );
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellLayeredSampling class.
 *  @param  camera [in] pointer to the camera
 *  @param  volume [in] pointer to the volume object
 *  @param  subpixel_level [in] sub-pixel level
 *  @param  sampling_step [in] sapling step
 *  @param  transfer_function [in] transfer function
 *  @param  object_depth [in] depth value of the input volume at the CoG
 */
/*===========================================================================*/
CellByCellLayeredSampling::CellByCellLayeredSampling(
    const vismodule::Camera*           camera,
    const vismodule::VolumeObjectBase* volume,
    const size_t                 subpixel_level,
    const float                  sampling_step,
    const vismodule::TransferFunction& transfer_function,
    const float                  object_depth ):
    vismodule::MapperBase( transfer_function ),
    vismodule::PointObject(),
    m_camera( 0 ),
    m_pregenerated_particles( 0 )
{
    this->attachCamera( camera );
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
CellByCellLayeredSampling::~CellByCellLayeredSampling( void )
{
    m_density_map.deallocate();
    if ( m_pregenerated_particles ) delete m_pregenerated_particles;
}

/*===========================================================================*/
/**
 *  @brief  Retruns the sub-pixel level.
 *  @return sub-pixel level
 */
/*===========================================================================*/
const size_t CellByCellLayeredSampling::subpixelLevel( void ) const
{
    return( m_subpixel_level );
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampling step.
 *  @return sampling step
 */
/*===========================================================================*/
const float CellByCellLayeredSampling::samplingStep( void ) const
{
    return( m_sampling_step );
}

/*===========================================================================*/
/**
 *  @brief  Returns the depth of the object at the center of the gravity.
 *  @return depth
 */
/*===========================================================================*/
const float CellByCellLayeredSampling::objectDepth( void ) const
{
    return( m_object_depth );
}

/*===========================================================================*/
/**
 *  @brief  Attaches a camera.
 *  @param  camera [in] pointer to the camera
 */
/*===========================================================================*/
void CellByCellLayeredSampling::attachCamera( const vismodule::Camera* camera )
{
    m_camera = camera;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sub-pixel level.
 *  @param  subpixel_level [in] sub-pixel level
 */
/*===========================================================================*/
void CellByCellLayeredSampling::setSubpixelLevel( const size_t subpixel_level )
{
    m_subpixel_level = subpixel_level;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sampling step.
 *  @param  sampling_step [in] sampling step
 */
/*===========================================================================*/
void CellByCellLayeredSampling::setSamplingStep( const float sampling_step )
{
    m_sampling_step = sampling_step;
}

/*===========================================================================*/
/**
 *  @brief  Sets a depth of the object at the center of the gravity.
 *  @param  object_depth [in] depth
 */
/*===========================================================================*/
void CellByCellLayeredSampling::setObjectDepth( const float object_depth )
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
CellByCellLayeredSampling::SuperClass* CellByCellLayeredSampling::exec( const vismodule::ObjectBase* object )
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
    if ( volume_type == vismodule::VolumeObjectBase::Unstructured )
    {
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
 *  @brief  Mapping for the unstructured volume object (tetrahedral cell data only).
 *  @param  camera [in] pointer to the camera
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
void CellByCellLayeredSampling::mapping( const vismodule::Camera* camera, const vismodule::UnstructuredVolumeObject* volume )
{
    if ( volume->cellType() != vismodule::UnstructuredVolumeObject::Tetrahedra )
    {
        BaseClass::m_is_success = false;
        visModuleMessageError( "Cell type is not tetrahedra." );
        return;
    }

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
 *  @brief  Generates particles for the unstructured volume object.
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
template <typename T>
void CellByCellLayeredSampling::generate_particles( const vismodule::UnstructuredVolumeObject* volume )
{
    // Tiny value.
    const size_t resolution = BaseClass::transferFunction().resolution();
    const float TinyValue = 1.0f / resolution;

    // Pregenerate particles.
    this->pregenerate_particles( 800000 );

    // Vertex data arrays. (output)
    std::vector<vismodule::Real32> vertex_coords;
    std::vector<vismodule::UInt8>  vertex_colors;
    std::vector<vismodule::Real32> vertex_normals;

    // Set a tetrahedral cell interpolator.
    vismodule::TetrahedralCell<T>* cell = new vismodule::TetrahedralCell<T>( volume );

    // Generate particles for each cell.
    const size_t ncells = volume->ncells();
    for ( size_t index = 0; index < ncells; ++index )
    {
        // Bind the cell which is indicated by 'index'.
        cell->bindCell( index );

        const T* S = cell->scalars();
        const T S_min = vismodule::Math::Min( S[0], S[1], S[2], S[3] );
        const T S_max = vismodule::Math::Max( S[0], S[1], S[2], S[3] );

        // Uniform sampling.
        if ( vismodule::Math::Equal( S[0], S[1] ) &&
             vismodule::Math::Equal( S[1], S[2] ) &&
             vismodule::Math::Equal( S[2], S[3] ) )
        {
            const float scalar = cell->averagedScalar();
            const float density = this->calculate_density( scalar );
            const size_t nparticles = this->calculate_number_of_particles( density, cell );

            this->uniform_sampling<T>(
                cell, BaseClass::transferFunction(), nparticles,
                &vertex_coords, &vertex_colors, &vertex_normals );
        }

        // Rejection sampling.
        else if ( S_max - S_min < TinyValue )
        {
            const float scalar = cell->averagedScalar();
            const float density = this->calculate_density( scalar );
            const size_t nparticles = this->calculate_number_of_particles( density, cell );

            this->rejection_sampling<T>(
                cell, BaseClass::transferFunction(), nparticles,
                &vertex_coords, &vertex_colors, &vertex_normals );
        }

        // Roulette selection or rejection sampling.
        else
        {
            this->calculate_particles_in_cell( cell );

            const size_t N_in = m_selected_particles.nparticles;
            const size_t N_tet = this->calculate_number_of_particles( N_in );

            // All particles are selected from pregenerated particles.
            if ( N_in > N_tet )
            {
                this->roulette_selection(
                    cell, BaseClass::transferFunction(), N_tet,
                    &vertex_coords, &vertex_colors, &vertex_normals );
            }
            else
            {
                this->roulette_selection(
                    cell, BaseClass::transferFunction(), N_in,
                    &vertex_coords, &vertex_colors, &vertex_normals );

                this->rejection_sampling<T>(
                    cell, BaseClass::transferFunction(), N_tet - N_in,
                    &vertex_coords, &vertex_colors, &vertex_normals );
            }
        } // end of Roulette selection
    } // end of for

    delete cell;

    SuperClass::m_coords = vismodule::ValueArray<vismodule::Real32>( vertex_coords );
    SuperClass::m_colors = vismodule::ValueArray<vismodule::UInt8>( vertex_colors );
    SuperClass::m_normals = vismodule::ValueArray<vismodule::Real32>( vertex_normals );
    SuperClass::setSize( 1.0f );
}

/*===========================================================================*/
/**
 *  @brief  Pregenerate particles in a unit cube.
 *  @param  nparticles [in] target number of particles
 */
/*===========================================================================*/
void CellByCellLayeredSampling::pregenerate_particles( const size_t nparticles )
{
    const size_t resolution = BaseClass::transferFunction().resolution();
    const size_t nintervals = resolution - 1;

    // Integrate the density distribution function as piecewise linear function.
    vismodule::ValueArray<float> m( nintervals ); m.fill( 0x00 );
    float M = 0.0f;
    for ( size_t i = 0; i < nintervals; i++ )
    {
        const float p0 = m_density_map[i];
        const float p1 = m_density_map[i+1];
        const float s0 = static_cast<float>(i) / nintervals;
        const float s1 = static_cast<float>(i+1) / nintervals;

        m[i] = 0.5f * ( p0 + p1 ) * ( s1 - s0 );
        M += m[i];
    }

    // Set the M value.
    m_M_value = M;

    // Calculate the number of particles in each interval.
    vismodule::MersenneTwister R; R.setSeed();
    vismodule::ValueArray<vismodule::UInt32> n( nintervals ); n.fill( 0x00 );
    vismodule::UInt32 N = 0; // total number of particles
    for ( size_t i = 0; i < nintervals; i++ )
    {
        const float tmp = static_cast<float>( nparticles ) * m[i] / M;

        n[i] = static_cast<vismodule::UInt32>( tmp );
        if ( tmp - n[i] > R() ) n[i] = n[i] + 1;

        N += n[i];
    }

    // Allocate the memory for the selected particles by N which is maximum number of particles.
    m_selected_particles.indices.allocate( N );

    // Generate particles by using rejection method.
    vismodule::ValueArray<vismodule::Real32> coords( N * 3 );
    vismodule::Real32* pcoords = coords.pointer();
    for ( size_t i = 0; i < nintervals; i++ )
    {
        const float p0 = m_density_map[i];
        const float p1 = m_density_map[i+1];
        const float s0 = static_cast<float>(i) / nintervals;
        const float s1 = static_cast<float>(i+1) / nintervals;

        const float a = ( p1 - p0 ) / ( s1 - s0 );
        const float b = ( p0 * s1 - p1 * s0 ) / ( s1 - s0 );
        const float c = vismodule::Math::Max( p0, p1 );

        size_t count = 0;
        while ( count < n[i] )
        {
            const float s = static_cast<float>( R() ) * ( s1 - s0 ) + s0;
            const float g = a * s + b;
            if ( R() > 1.0F / c * g )
            {
                // reject
                continue;
            }
            else
            {
                // accept.
                const float t1 = static_cast<float>( R() );
                const float t2 = static_cast<float>( R() );
                *(pcoords++) = t1;
                *(pcoords++) = t2;
                *(pcoords++) = s;

                count++;
            }
        }
    }

    if ( m_pregenerated_particles ) delete m_pregenerated_particles;
    m_pregenerated_particles = new vismodule::PointObject( coords );
}

/*===========================================================================*/
/**
 *  @brief  Uniform sampling.
 *  @param  cell [in] tetrahedral cell
 *  @param  tfunc [in] transfer function
 *  @param  nparticles [in] number of particles
 *  @param  coords [out] pointer to the coordinate value array
 *  @param  colors [out] pointer to the color value array
 *  @param  normals [out] pointer to the normal vector array
 */
/*===========================================================================*/
template <typename T>
void CellByCellLayeredSampling::uniform_sampling(
    const vismodule::TetrahedralCell<T>* cell,
    const vismodule::TransferFunction& tfunc,
    const size_t nparticles,
    std::vector<vismodule::Real32>* coords,
    std::vector<vismodule::UInt8>*  colors,
    std::vector<vismodule::Real32>* normals )
{
    for ( size_t particle = 0; particle < nparticles; ++particle )
    {
        // Calculate a coord.
        const vismodule::Vector3f coord = cell->randomSampling();

        // Calculate a color.
        const float scalar = cell->scalar();
        const vismodule::RGBColor color( tfunc.colorMap().at( scalar ) );

        // Calculate a normal.
        const Vector3f normal( -cell->gradient() );

        // set coord, color, and normal to point object( this ).
        coords->push_back( coord.x() );
        coords->push_back( coord.y() );
        coords->push_back( coord.z() );

        colors->push_back( color.r() );
        colors->push_back( color.g() );
        colors->push_back( color.b() );

        normals->push_back( normal.x() );
        normals->push_back( normal.y() );
        normals->push_back( normal.z() );
    }
}

/*===========================================================================*/
/**
 *  @brief  Rejection sampling.
 *  @param  cell [in] tetrahedral cell
 *  @param  tfunc [in] transfer function
 *  @param  nparticles [in] number of particles
 *  @param  coords [out] pointer to the coordinate value array
 *  @param  colors [out] pointer to the color value array
 *  @param  normals [out] pointer to the normal vector array
 */
/*===========================================================================*/
template <typename T>
void CellByCellLayeredSampling::rejection_sampling(
    const vismodule::TetrahedralCell<T>* cell,
    const vismodule::TransferFunction& tfunc,
    const size_t nparticles,
    std::vector<vismodule::Real32>* coords,
    std::vector<vismodule::UInt8>*  colors,
    std::vector<vismodule::Real32>* normals )
{
    const T* S = cell->scalars();
    const float S_min = static_cast<float>( vismodule::Math::Min( S[0], S[1], S[2], S[3] ) );
    const float S_max = static_cast<float>( vismodule::Math::Max( S[0], S[1], S[2], S[3] ) );
    const float p_max = this->calculate_maximum_density( S_min, S_max ) / nparticles;

    size_t count = 0;
    while ( count < nparticles )
    {
        const vismodule::Vector3f coord = cell->randomSampling();
        const float scalar = cell->scalar();
        const float density = this->calculate_density( scalar );

        const float p = density / nparticles;
        const float R = Generator::GetRandomNumber();
        if ( p > p_max * R )
        {
            // Calculate a color.
            const vismodule::RGBColor color( tfunc.colorMap().at( scalar ) );

            // Calculate a normal.
            const Vector3f normal( -cell->gradient() );

            // set coord, color, and normal to point object( this ).
            coords->push_back( coord.x() );
            coords->push_back( coord.y() );
            coords->push_back( coord.z() );

            colors->push_back( color.r() );
            colors->push_back( color.g() );
            colors->push_back( color.b() );

            normals->push_back( normal.x() );
            normals->push_back( normal.y() );
            normals->push_back( normal.z() );

            count++;
        }
    }
}

/*===========================================================================*/
/**
 *  @brief  Roulette selection.
 *  @param  cell [in] tetrahedral cell
 *  @param  tfunc [in] transfer function
 *  @param  nparticles [in] number of particles
 *  @param  coords [out] pointer to the coordinate value array
 *  @param  colors [out] pointer to the color value array
 *  @param  normals [out] pointer to the normal vector array
 */
/*===========================================================================*/
template <typename T>
void CellByCellLayeredSampling::roulette_selection(
    const vismodule::TetrahedralCell<T>* cell,
    const vismodule::TransferFunction& tfunc,
    const size_t nparticles,
    std::vector<vismodule::Real32>* coords,
    std::vector<vismodule::UInt8>*  colors,
    std::vector<vismodule::Real32>* normals )
{
    const float a1 = m_A_matrix[0][0];
    const float a2 = m_A_matrix[1][1];
    const float a3 = m_A_matrix[2][2];
    const float b1 = m_A_matrix[0][3];
    const float b2 = m_A_matrix[1][3];
    const float b3 = m_A_matrix[2][3];
    const vismodule::Matrix44f A_inv(
        1.0f/a1, 0.0f,    0.0f,    -b1/a1,
        0.0f,    1.0f/a2, 0.0f,    -b2/a2,
        0.0f,    0.0f,    1.0f/a3, -b3/a3,
        0.0f,    0.0f,    0.0f,    1.0f       );

    const vismodule::Matrix44f L_inv( m_L_matrix.transpose() );

    const vismodule::Vector3f g( cell->gradient().normalize() );

    const vismodule::Matrix44f LA_inv = L_inv * A_inv;
    for ( size_t i = 0; i < nparticles; i++ )
    {
        const float fid = Generator::GetRandomNumber() * m_selected_particles.nparticles;
        const size_t id = static_cast< size_t >( fid );
        const size_t id3 = m_selected_particles.indices[ id ] * 3;
        const vismodule::Vector4f selected_particle(
            m_pregenerated_particles->coords()[ id3   ],
            m_pregenerated_particles->coords()[ id3+1 ],
            m_pregenerated_particles->coords()[ id3+2 ],
            1.0f  );

        const vismodule::Vector4f coord( LA_inv * selected_particle );

        const float scalar = selected_particle.z();
        const float max_range = static_cast<float>( tfunc.resolution() - 1 );
        const size_t index = static_cast<size_t>( scalar * max_range );
        const vismodule::RGBColor color( tfunc.colorMap()[index] );

        // Set coord, color, and normal to the point object.
        coords->push_back( coord.x() );
        coords->push_back( coord.y() );
        coords->push_back( coord.z() );

        colors->push_back( color.r() );
        colors->push_back( color.g() );
        colors->push_back( color.b() );

        normals->push_back( -g.x() );
        normals->push_back( -g.y() );
        normals->push_back( -g.z() );
    }
}

/*===========================================================================*/
/**
 *  @brief  Calculate density value from scalar value.
 *  @param  scalar [in] scalar value
 *  @return density value
 */
/*===========================================================================*/
const float CellByCellLayeredSampling::calculate_density( const float scalar )
{
    const float min_value = BaseClass::transferFunction().colorMap().minValue();
    const float max_value = BaseClass::transferFunction().colorMap().maxValue();
    const float max_range = static_cast<float>( BaseClass::transferFunction().resolution() - 1 );
    const float normalize_factor = max_range / ( max_value - min_value );
    const float normalized_scalar = ( scalar - min_value ) * normalize_factor;
    const size_t index0 = static_cast<size_t>( normalized_scalar );
    const size_t index1 = index0 + 1;
    const float scalar_offset = normalized_scalar - index0;

    const float* const density_map = m_density_map.pointer();

    if ( index0 == ( BaseClass::transferFunction().resolution() - 1 ) )
    {
        return density_map[ index0 ];
    }
    else
    {
        const float rho0 = density_map[ index0 ];
        const float rho1 = density_map[ index1 ];
        const float interpolated_density = ( rho1 - rho0 ) * scalar_offset + rho0;

        return interpolated_density;
    }
}

/*===========================================================================*/
/**
 *  @brief  Calculate maximum dentiy value.
 *  @param  scalar0 [in] scalar value
 *  @param  scalar1 [in] scalar value
 *  @return density value
 */
/*===========================================================================*/
const float CellByCellLayeredSampling::calculate_maximum_density( const float scalar0, const float scalar1 )
{
    if ( scalar0 > scalar1 )
    {
        visModuleMessageError("undefined use of calculate_maximum_density.");
        return 0.0f;
    }

    const float min_value = BaseClass::transferFunction().colorMap().minValue();
    const float max_value = BaseClass::transferFunction().colorMap().maxValue();
    const float max_range = static_cast<float>( BaseClass::transferFunction().resolution() - 1 );
    const float normalize_factor = max_range / ( max_value - min_value );
    const size_t index0 = static_cast<size_t>( ( scalar0 - min_value ) * normalize_factor ) + 1;
    const size_t index1 = static_cast<size_t>( ( scalar1 - min_value ) * normalize_factor );

    const float* const density_map = m_density_map.pointer();

    float maximum_density = density_map[ index0 ];

    for ( size_t i = index0 + 1; i <= index1; i++ )
    {
        maximum_density = density_map[ i ] > maximum_density ? density_map[ i ] : maximum_density;
    }

    const float density0 = this->calculate_density( scalar0 );
    maximum_density = density0 > maximum_density ? density0 : maximum_density;

    const float density1 = this->calculate_density( scalar1 );
    maximum_density = density1 > maximum_density ? density1 : maximum_density;

    return maximum_density;
}

/*===========================================================================*/
/**
 *  @brief  Calculate number of particles.
 *  @param  density [in] density value
 *  @param  cell [in] pointer to cell
 *  @return number of particles
 */
/*===========================================================================*/
template <typename T>
const size_t CellByCellLayeredSampling::calculate_number_of_particles(
    const float density,
    const vismodule::TetrahedralCell<T>* cell )
{
    const float volume_of_cell = cell->volume();
    const float N = density * volume_of_cell;
    const float R = Generator::GetRandomNumber();

    size_t n = static_cast<size_t>( N );
    if ( N - n > R ) { ++n; }

    return( n );
}

/*===========================================================================*/
/**
 *  @brief  Calculate required number of particles.
 *  @param  nparticles_in_cell [in] number of particles in cell
 *  @return required number of particles
 */
/*===========================================================================*/
const size_t CellByCellLayeredSampling::calculate_number_of_particles(
    const size_t nparticles_in_cell )
{
    const size_t N_in = nparticles_in_cell;
    const size_t N_all = m_pregenerated_particles->nvertices();

    const float a1 = m_A_matrix[0][0];
    const float a2 = m_A_matrix[1][1];
    const float a3 = m_A_matrix[2][2];
    const float detA_inv = 1.0f / vismodule::Math::Abs( a1 * a2 * a3 );
    const float N = detA_inv * m_M_value * N_in / N_all;
    const float R = Generator::GetRandomNumber();

    size_t n = static_cast<size_t>( N );
    if ( N - n > R ) { ++n; }

    return( n );
}

/*===========================================================================*/
/**
 *  @brief  Calculate particles in a cell.
 *  @param  cell [in] pointer to cell
 */
/*===========================================================================*/
template <typename T>
void CellByCellLayeredSampling::calculate_particles_in_cell(
    const vismodule::TetrahedralCell<T>* cell )
{
    const T* S = cell->scalars();
    const T S_min = vismodule::Math::Min( S[0], S[1], S[2], S[3] );
    const T S_max = vismodule::Math::Max( S[0], S[1], S[2], S[3] );

    const float min_value = BaseClass::transferFunction().colorMap().minValue();
    const float max_value = BaseClass::transferFunction().colorMap().maxValue();
    const float min_cell_scalar = static_cast<float>( S_min - min_value ) / ( max_value - min_value );
    const float max_cell_scalar = static_cast<float>( S_max - min_value ) / ( max_value - min_value );

    // Gradient vector.
    vismodule::Vector3f g = cell->gradient(); g.normalize();

    // Temporary vector for calculating basis vectors: t1 and t2.
    vismodule::Vector3f u( 0.0f, 0.0f, 0.0f );
    size_t index = 0;
    index = vismodule::Math::Abs( g[0] ) < vismodule::Math::Abs( g[1] ) ? 0 : 1;
    index = vismodule::Math::Abs( g[index] ) < vismodule::Math::Abs( g[2] ) ? index : 2;
    u[index] = 1.0f;

    // Basis vectors.
    vismodule::Vector3f t1( g.cross( u ) ); t1.normalize();
    vismodule::Vector3f t2( g.cross( t1 ) ); t2.normalize();

    // Transformation matrix L.
    m_L_matrix = vismodule::Matrix44f(
        t1.x(), t1.y(), t1.z(), 0.0f,
        t2.x(), t2.y(), t2.z(), 0.0f,
        g.x(),  g.y(),  g.z(),  0.0f,
        0.0f,   0.0f,   0.0f,   1.0f );

    // Transformation matrix A.
    const vismodule::Vector3f* v = cell->vertices();
    const vismodule::Vector4f v0 = m_L_matrix * vismodule::Vector4f( v[0].x(), v[0].y(), v[0].z(), 1.0f );
    const vismodule::Vector4f v1 = m_L_matrix * vismodule::Vector4f( v[1].x(), v[1].y(), v[1].z(), 1.0f );
    const vismodule::Vector4f v2 = m_L_matrix * vismodule::Vector4f( v[2].x(), v[2].y(), v[2].z(), 1.0f );
    const vismodule::Vector4f v3 = m_L_matrix * vismodule::Vector4f( v[3].x(), v[3].y(), v[3].z(), 1.0f );

    const float x_min = vismodule::Math::Min( v0.x(), v1.x(), v2.x(), v3.x() );
    const float x_max = vismodule::Math::Max( v0.x(), v1.x(), v2.x(), v3.x() );
    const float y_min = vismodule::Math::Min( v0.y(), v1.y(), v2.y(), v3.y() );
    const float y_max = vismodule::Math::Max( v0.y(), v1.y(), v2.y(), v3.y() );
    const float z_min = vismodule::Math::Min( v0.z(), v1.z(), v2.z(), v3.z() );
    const float z_max = vismodule::Math::Max( v0.z(), v1.z(), v2.z(), v3.z() );

    const float a1 = 1.0f / ( x_max - x_min );
    const float a2 = 1.0f / ( y_max - y_min );
    const float a3 = ( max_cell_scalar - min_cell_scalar ) / ( z_max - z_min );
    const float b1 = -a1 * x_min;
    const float b2 = -a2 * y_min;
    const float b3 = -a3 * z_min + min_cell_scalar;

    m_A_matrix = vismodule::Matrix44f(
        a1,   0.0f, 0.0f, b1,
        0.0f, a2,   0.0f, b2,
        0.0f, 0.0f, a3,   b3,
        0.0f, 0.0f, 0.0f, 1.0f );

    // Select particles in the cell.
    const vismodule::Vector4f g0 = m_A_matrix * v0;
    const vismodule::Vector4f g1 = m_A_matrix * v1;
    const vismodule::Vector4f g2 = m_A_matrix * v2;
    const vismodule::Vector4f g3 = m_A_matrix * v3;

    const vismodule::Vector4f d1( g1 - g0 );
    const vismodule::Vector4f d2( g3 - g0 );
    const vismodule::Vector4f d3( g2 - g0 );

    const vismodule::Matrix33f D(
        d1[0], d2[0], d3[0],
        d1[1], d2[1], d3[1],
        d1[2], d2[2], d3[2] );

    const vismodule::Matrix33f D_inv( D.inverse() );

    size_t counter = 0;
    const size_t nparticles = m_pregenerated_particles->nvertices();
    for ( size_t i = 0, i3 = 0; i < nparticles; i++, i3 += 3 )
    {
        const vismodule::Vector3f located_v(
            m_pregenerated_particles->coords()[i3  ] - g0.x(),
            m_pregenerated_particles->coords()[i3+1] - g0.y(),
            m_pregenerated_particles->coords()[i3+2] - g0.z() );

        const vismodule::Vector3f lambda( D_inv * located_v );
        if ( ( lambda[0] >= 0.0f && lambda[1] >= 0.0f ) && lambda[2] >= 0.0f )
        {
            if ( lambda[0] + lambda[1] + lambda[2] <= 1.0f )
            {
                m_selected_particles.indices[ counter ] = i;
                counter ++;
            }
        }
    }

    m_selected_particles.nparticles = counter;
}

} // end of namespace vismodule
