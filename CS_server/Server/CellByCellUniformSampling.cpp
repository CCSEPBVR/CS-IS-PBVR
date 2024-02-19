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
#include <kvs/Compiler>
#ifdef KVS_COMPILER_VC
#include <time.h>
#else
#include <sys/time.h>
#endif
#include "CellByCellUniformSampling.h"
#include <vector>
#include <stdlib.h>
#include <kvs/DebugNew>
#include "ObjectManager.h"
#include <kvs/Camera>
#include <kvs/TrilinearInterpolator>
#include <kvs/Value>
#include "CellBase.h"
#include "TetrahedralCell.h"
#include "QuadraticTetrahedralCell.h"
#include "HexahedralCell.h"
#include "QuadraticHexahedralCell.h"
#include "PrismaticCell.h"
#include "PyramidalCell.h"
//#include "TriangleCell.h"
//#include "QuadraticTriangleCell.h"
//#include "SquareCell.h"
#include "GlobalCore.h"
#include "CropRegion.h"

#include "common.h"
#include "FrequencyTable.h"
//#include "SFMT/SFMT.h" 

#ifdef ENABLE_MPI
#include <mpi.h>
#include "mpi_controller.h"
#include <kvs/DistributedUniformSampling>
#endif

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

#include "timer_simple.h"

namespace Generator = pbvr::CellByCellParticleGenerator;

using FuncParser::Variable;
using FuncParser::Variables;
using FuncParser::Function;
using FuncParser::FunctionParser;


namespace pbvr
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellUniformSampling class.
 */
/*===========================================================================*/
CellByCellUniformSampling::CellByCellUniformSampling():
    pbvr::MapperBase(),
    pbvr::PointObject(),
    m_transfer_function_synthesizer( NULL ),
    m_normal_ingredient( 0 ),
    m_camera( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellUniformSampling class.
 *  @param  volume [in] pointer to the volume object
 *  @param  m_subpixel_level [in] sub-pixel level
 *  @param  m_sampling_step [in] sapling step
 *  @param  m_transfer_function [in] transfer function
 *  @param  object_depth [in] depth value of the input volume at the CoG
 */
/*===========================================================================*/
CellByCellUniformSampling::CellByCellUniformSampling(
    const pbvr::VolumeObjectBase& volume,
    const size_t                 subpixel_level,
    const float                  sampling_step,
    const pbvr::TransferFunction& transfer_function,
    TransferFunctionSynthesizer* transfunc_synthesizer,
    const size_t                 normal_ingredient,
    const CropRegion&            crop,
    const float                  object_depth ):
    pbvr::MapperBase( transfer_function ),
    pbvr::PointObject(),
    m_transfer_function_synthesizer( transfunc_synthesizer ),
    m_normal_ingredient( normal_ingredient ),
    m_camera( 0 ),
    m_particle_density( 1.0 ),
    m_batch( false )
{
    m_crop = crop;
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
 *  @param  m_subpixel_level [in] sub-pixel level
 *  @param  m_sampling_step [in] sapling step
 *  @param  m_transfer_function [in] transfer function
 *  @param  object_depth [in] depth value of the input volume at the CoG
 */
/*===========================================================================*/
CellByCellUniformSampling::CellByCellUniformSampling(
    const kvs::Camera&           camera,
    const pbvr::VolumeObjectBase& volume,
    const size_t                 subpixel_level,
    const float                  sampling_step,
    const pbvr::TransferFunction& transfer_function,
    std::vector<pbvr::TransferFunction>& transfer_function_array, 
    TransferFunctionSynthesizer* transfunc_synthesizer,
    const size_t                 normal_ingredient,
    const CropRegion&            crop,
    const float                  particle_density,
    const bool                   batch,
    const float                  object_depth ):
    pbvr::MapperBase( transfer_function ),
    pbvr::PointObject(),
    m_transfer_function_array( transfer_function_array ),
    m_transfer_function_synthesizer( transfunc_synthesizer ),
    m_normal_ingredient( normal_ingredient ),
    m_particle_density( particle_density ),
    m_batch( batch )
{
    m_crop = crop;
    this->attachCamera( camera ),
    this->setSubpixelLevel( subpixel_level );
    this->setSamplingStep( sampling_step );
    this->setObjectDepth( object_depth );
    this->exec( volume);
}

CellByCellUniformSampling::CellByCellUniformSampling(
    const kvs::Camera&           camera,
    const pbvr::VolumeObjectBase& volume,
    const size_t                 subpixel_level,
    const float                  sampling_step,
    const pbvr::TransferFunction& transfer_function,
    TransferFunctionSynthesizer* transfunc_synthesizer,
    const size_t                 normal_ingredient,
    const CropRegion&            crop,
    const bool                   gt5d_full,
    const float                  object_depth ):
    pbvr::MapperBase( transfer_function ),
    pbvr::PointObject(),
    m_transfer_function_synthesizer( transfunc_synthesizer ),
    m_normal_ingredient( normal_ingredient ),
    m_particle_density( 1.0 ),
    m_batch( false )
{
    m_crop = crop;
    m_gt5d_full = gt5d_full;
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
CellByCellUniformSampling::~CellByCellUniformSampling()
{
    m_density_map.deallocate();
}

/*===========================================================================*/
/**
 *  @brief  Retruns the sub-pixel level.
 *  @return sub-pixel level
 */
/*===========================================================================*/
const size_t CellByCellUniformSampling::subpixelLevel() const
{
    return m_subpixel_level;
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampling step.
 *  @return sampling step
 */
/*===========================================================================*/
const float CellByCellUniformSampling::samplingStep() const
{
    return m_sampling_step;
}

/*===========================================================================*/
/**
 *  @brief  Returns the depth of the object at the center of the gravity.
 *  @return depth
 */
/*===========================================================================*/
const float CellByCellUniformSampling::objectDepth() const
{
    return m_object_depth;
}

/*===========================================================================*/
/**
 *  @brief  Attaches a camera.
 *  @param  camera [in] pointer to the camera
 */
/*===========================================================================*/
void CellByCellUniformSampling::attachCamera( const kvs::Camera& camera )
{
    m_camera = &camera;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sub-pixel level.
 *  @param  m_subpixel_level [in] sub-pixel level
 */
/*===========================================================================*/
void CellByCellUniformSampling::setSubpixelLevel( const size_t subpixel_level )
{
    m_subpixel_level = subpixel_level;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sampling step.
 *  @param  m_sampling_step [in] sampling step
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
CellByCellUniformSampling::SuperClass* CellByCellUniformSampling::exec( const pbvr::ObjectBase& object )
{
    if ( !&object )
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Input object is NULL." );
        return NULL;
    }

    const pbvr::VolumeObjectBase* volume = pbvr::VolumeObjectBase::DownCast( object );
    if ( !volume )
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Input object is not volume dat." );
        return NULL;
    }

    const pbvr::VolumeObjectBase::VolumeType volume_type = volume->volumeType();
    if ( volume_type == pbvr::VolumeObjectBase::Structured )
    {
    }
    else // volume_type == pbvr::VolumeObjectBase::Unstructured
    {
        const pbvr::UnstructuredVolumeObject* uvo_p = static_cast<const pbvr::UnstructuredVolumeObject*>( &object );
        if ( m_camera )
        {
            this->mapping( *m_camera, *uvo_p );
        }
        else
        {
            // Generate particles by using default camera parameters.
            if ( pbvr::GlobalCore::m_camera )
            {
                if ( pbvr::GlobalCore::m_camera->windowWidth() != 0 && pbvr::GlobalCore::m_camera->windowHeight() )
                {
                    const kvs::Camera* camera = pbvr::GlobalCore::m_camera;
                    this->mapping( *camera, *uvo_p );
                }
            }
            else
            {
                kvs::Camera* camera = new kvs::Camera();
                this->mapping( *camera, *uvo_p );
                delete camera;
            }
        }
    }

    return this;
}

template <>
void CellByCellUniformSampling::generate_particles<kvs::Real32>( const pbvr::UnstructuredVolumeObject& volume );

/*===========================================================================*/
/**
 *  @brief  Mapping for the unstructured volume object.
 *  @param  camera [in] pointer to the camera
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
void CellByCellUniformSampling::mapping( const kvs::Camera& camera, const pbvr::UnstructuredVolumeObject& volume )
{
    // Attach the pointer to the volume object and set the min/max coordinates.
    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );

    const pbvr::VolumeObjectBase *object = BaseClass::volume();

    if ( m_transfer_function_synthesizer )
    {
        float max_opacity;
        float max_density;
        float sampling_volume_inverse;

        Generator::CalculateDensityConstaint(
            camera,
            *object,
            static_cast<float>( m_subpixel_level ),
            m_sampling_step,
            &sampling_volume_inverse,
            &max_opacity,
            &max_density );

        m_transfer_function_synthesizer->setMaxOpacity( max_opacity );
        m_transfer_function_synthesizer->setMaxDensity( max_density );
        m_transfer_function_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );
    }
    else
    {
        BaseClass::m_transfer_function.setRange( volume );

        // Calculate the density map.
        m_density_map = Generator::CalculateDensityMap(
                            camera,
                            *object,
                            static_cast<float>( m_subpixel_level ),
                            m_sampling_step,
                            BaseClass::transferFunction().opacityMap() );
    }
#ifdef ENABLE_MPI
    if ( m_gt5d_full == true )
    {
        this->generate_particles_gt5d( volume );
        return;
    }
#endif
    // Generate the particles.
//    if ( !volume.hasMinMaxValues() ) volume.updateMinMaxValues();
//    const float min_value = static_cast<float>( volume.minValue() );
//    const float max_value = static_cast<float>( volume.maxValue() );
    const std::type_info& type = volume.values().typeInfo()->type();
    if (      type == typeid( kvs::Int8   ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( -128, 127 );
        this->generate_particles<kvs::Int8>( volume );
    }
    else if ( type == typeid( kvs::Int16  ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<kvs::Int16>( volume );
    }
    else if ( type == typeid( kvs::Int32  ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<kvs::Int32>( volume );
    }
    else if ( type == typeid( kvs::Int64  ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<kvs::Int64>( volume );
    }
    else if ( type == typeid( kvs::UInt8  ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( 0, 255 );
        this->generate_particles<kvs::UInt8>( volume );
    }
    else if ( type == typeid( kvs::UInt16 ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<kvs::UInt16>( volume );
    }
    else if ( type == typeid( kvs::UInt32 ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<kvs::UInt32>( volume );
    }
    else if ( type == typeid( kvs::UInt64 ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<kvs::UInt64>( volume );
    }
    else if ( type == typeid( kvs::Real32 ) )
    {
        this->generate_particles<kvs::Real32>( volume );
    }
    else if ( type == typeid( kvs::Real64 ) )
    {
//        if ( !m_transfer_function.hasRange() ) BaseClass::m_transfer_function.setRange( min_value, max_value );
        this->generate_particles<kvs::Real64>( volume );
    }
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Unsupported data type '%s'.", volume.values().typeInfo()->typeName() );
    }
}


/*===========================================================================*/
/**
 *  @brief  Generates particles for the unstructured volume object.
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
template <typename T>
void CellByCellUniformSampling::generate_particles( const pbvr::UnstructuredVolumeObject& volume )
{
    // Vertex data arrays. (output)
    std::vector<kvs::Real32> vertex_coords;
    std::vector<kvs::UInt8>  vertex_colors;
    std::vector<kvs::Real32> vertex_normals;

    // Set a tetrahedral cell interpolator.
    pbvr::CellBase<T>* cell = NULL;
    switch ( volume.cellType() )
    {
    case pbvr::VolumeObjectBase::Tetrahedra:
    {
        cell = new pbvr::TetrahedralCell<T>( volume );
        break;
    }
    case pbvr::VolumeObjectBase::QuadraticTetrahedra:
    {
        cell = new pbvr::QuadraticTetrahedralCell<T>( volume );
        break;
    }
    case pbvr::VolumeObjectBase::Hexahedra:
    {
        cell = new pbvr::HexahedralCell<T>( volume );
        break;
    }
    case pbvr::VolumeObjectBase::QuadraticHexahedra:
    {
        cell = new pbvr::QuadraticHexahedralCell<T>( volume );
        break;
    }
    case pbvr::VolumeObjectBase::Prism:
    {
        cell = new pbvr::PrismaticCell<T>( volume );
        break;
    }
    case pbvr::VolumeObjectBase::Pyramid:
    {
        cell = new pbvr::PyramidalCell<T>( volume );
        break;
    }
//    case pbvr::VolumeObjectBase::Triangle:
//    {
//        cell = new pbvr::TriangleCell<T>( volume );
//        break;
//    }
//    case pbvr::VolumeObjectBase::QuadraticTriangle:
//    {
//        cell = new pbvr::QuadraticTriangleCell<T>( volume );
//        break;
//    }
//    case pbvr::VolumeObjectBase::Square:
//    {
//        cell = new pbvr::SquareCell<T>( volume );
//        break;
//    }
//    case pbvr::VolumeObjectBase::QuadraticSquare:
//    {
//        cell = new pbvr::QuadraticSquareCell<T>( volume );
//        break;
//    }
    default:
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Unsupported cell type." );
        return;
    }
    }

    const float min_value = BaseClass::transferFunction().colorMap().minValue();
    const float max_value = BaseClass::transferFunction().colorMap().maxValue();
    const float max_range = static_cast<float>( BaseClass::transferFunction().resolution() - 1 );
    const float normalize_factor = max_range / ( max_value - min_value );

    const float* const  density_map = m_density_map.pointer();
    const kvs::ColorMap color_map( BaseClass::transferFunction().colorMap() );

    // Generate particles for each cell.
    const size_t ncells = volume.ncells();
    const size_t veclen = volume.veclen();
    const pbvr::CoordSynthesizerStrings* pCrdSynthStr = volume.getCoordSynthesizerStrings();
    CoordSynthesizerStrings css;
    if ( pCrdSynthStr ) css = *pCrdSynthStr;
    FuncParser::Variables synth_vars;
    FuncParser::Variable Tm;
    Tm.tag( "T" );
    synth_vars.push_back( Tm );
    FuncParser::Variable t;
    t.tag( "t" );
    synth_vars.push_back( t );
    Tm = t = ( float )pCrdSynthStr->m_time_step;
    FuncParser::Variable X;
    X.tag( "X" );
    synth_vars.push_back( X );
    FuncParser::Variable x;
    x.tag( "x" );
    synth_vars.push_back( x );
    FuncParser::Variable Y;
    Y.tag( "Y" );
    synth_vars.push_back( Y );
    FuncParser::Variable y;
    y.tag( "y" );
    synth_vars.push_back( y );
    FuncParser::Variable Z;
    Z.tag( "Z" );
    synth_vars.push_back( Z );
    FuncParser::Variable z;
    z.tag( "z" );
    synth_vars.push_back( z );
    std::vector<FuncParser::Variable> FuncVars(veclen);
    std::vector<std::string> varnames(veclen);
    for ( size_t i = 0; i < veclen; i++ )
    {
        std::stringstream ss;
        ss << "q" << i + 1;
        varnames[i] = ss.str();
        FuncVars[i] = FuncParser::Variable();
        FuncVars[i].tag(const_cast<char*>(varnames[i].c_str()));
        FuncVars[i] = 0.0f;
        synth_vars.push_back( FuncVars[i] );
    } // end of for(i)
    FunctionParser synth_func_parseX( css.m_x_coord_synthesizer_string,
                                      css.m_x_coord_synthesizer_string.size() + 1 );
    FuncParser::Function synth_funcX;
    if ( ! css.m_x_coord_synthesizer_string.empty() )
    {
        FunctionParser::Error err = synth_func_parseX.express( synth_funcX, synth_vars );
        if ( err != FunctionParser::ERR_NONE )
        {
            std::stringstream ess;
            ess << "Function : " << synth_funcX.str() << " error: "
                << FunctionParser::error_type_to_string[err].c_str() << std::endl;
            throw TransferFunctionSynthesizer::NumericException( ess.str() );
        }
    }
    FunctionParser synth_func_parseY( css.m_y_coord_synthesizer_string,
                                      css.m_y_coord_synthesizer_string.size() + 1 );
    FuncParser::Function synth_funcY;
    if ( ! css.m_y_coord_synthesizer_string.empty() )
    {
        FunctionParser::Error err = synth_func_parseY.express( synth_funcY, synth_vars );
        if ( err != FunctionParser::ERR_NONE )
        {
            std::stringstream ess;
            ess << "Function : " << synth_funcY.str() << " error: "
                << FunctionParser::error_type_to_string[err].c_str() << std::endl;
            throw TransferFunctionSynthesizer::NumericException( ess.str() );
        }
    }
    FunctionParser synth_func_parseZ( css.m_z_coord_synthesizer_string,
                                      css.m_z_coord_synthesizer_string.size() + 1 );
    FuncParser::Function synth_funcZ;
    if ( ! css.m_z_coord_synthesizer_string.empty() )
    {
        FunctionParser::Error err = synth_func_parseZ.express( synth_funcZ, synth_vars );
        if ( err != FunctionParser::ERR_NONE )
        {
            std::stringstream ess;
            ess << "Function : " << synth_funcZ.str() << " error: "
                << FunctionParser::error_type_to_string[err].c_str() << std::endl;
            throw TransferFunctionSynthesizer::NumericException( ess.str() );
        }
    }

    for ( size_t index = 0; index < ncells; ++index )
    {
        // add by @hira at 2016/12/01
        if (cell == NULL) continue;

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

        if ( p - nparticles_in_cell > Generator::GetRandomNumber() )
        {
            ++nparticles_in_cell;
        }

        // Generate a set of particles in this cell represented by v0,...,v3 and s0,...,s3.
        for ( size_t particle = 0; particle < nparticles_in_cell; ++particle )
        {
            // Calculate a coord.
            const kvs::Vector3f coord = cell->randomSampling();

            // Calculate a color.
            const float scalar = cell->scalar();
            const kvs::RGBColor color( color_map.at( scalar ) );

            // Calculate a normal.
            /* NOTE: The gradient vector of the cell is reversed for shading on the rendering process.
             */
            const kvs::Vector3f normal( -cell->gradient() );

            // using coord synthesizer
            kvs::Vector3f new_coord = coord;
            if ( pCrdSynthStr )
            {
                X = x = coord.x();
                Y = y = coord.y();
                Z = z = coord.z();

                size_t qn = 0;
                synth_vars[qn + 8] = scalar;
                for ( qn++; qn < veclen; qn++ )
                {
                    cell->bindCell( index, qn );
                    synth_vars[qn + 8] = cell->scalar();
                }

                kvs::Vector3f new_coord;
                if ( ! css.m_x_coord_synthesizer_string.empty() )
                {
                    float d = ( float )synth_funcX.eval();
                    TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                    new_coord[0] = d;
                }
                if ( ! css.m_y_coord_synthesizer_string.empty() )
                {
                    float d = ( float )synth_funcY.eval();
                    TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                    new_coord[1] = d;
                }
                if ( ! css.m_z_coord_synthesizer_string.empty() )
                {
                    float d = ( float )synth_funcZ.eval();
                    TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                    new_coord[2] = d;
                }
            }

            // set coord, color, and normal to point object( this ).
            vertex_coords.push_back( new_coord.x() );
            vertex_coords.push_back( new_coord.y() );
            vertex_coords.push_back( new_coord.z() );

            vertex_colors.push_back( color.r() );
            vertex_colors.push_back( color.g() );
            vertex_colors.push_back( color.b() );

            vertex_normals.push_back( normal.x() );
            vertex_normals.push_back( normal.y() );
            vertex_normals.push_back( normal.z() );

            cell->bindCell( index );
        } // end of 'paricle' for-loop
    } // end of 'cell' for-loop

    SuperClass::m_coords  = kvs::ValueArray<kvs::Real32>( vertex_coords );
    SuperClass::m_colors  = kvs::ValueArray<kvs::UInt8>( vertex_colors );
    SuperClass::m_normals = kvs::ValueArray<kvs::Real32>( vertex_normals );
    SuperClass::setSize( 1.0f );

    if (cell != NULL) delete cell;
}

template <>
void CellByCellUniformSampling::generate_particles<kvs::Real32>( const pbvr::UnstructuredVolumeObject& volume )
{
    double start = GetTime();
    size_t resolution = DEFAULT_NBINS;
//    kvs::ValueArray<float> O_min_recv;
//    kvs::ValueArray<float> O_max_recv;
//    kvs::ValueArray<float> C_min_recv;
//    kvs::ValueArray<float> C_max_recv;
//    kvs::ValueArray<int> o_histogram_recv;
//    kvs::ValueArray<int> c_histogram_recv;

    kvs::AnyValueArray valueArray = volume.values(); 
    Type* coordinates =  (float * )volume.coords().pointer(); 
    int ncoords =  volume.nnodes();
    unsigned int* connections =  (unsigned int*)volume.connections().pointer();
    int ncells = volume.ncells();
    int nnodes = volume.nnodes();

    const int nvariables = volume.veclen();
    //Type*  values[nvariables];
    Type** values;
    values = new Type * [nvariables];

    float sampling_volume_inverse = m_transfer_function_synthesizer -> getSamplingVolumeInverse()  ;
    float max_opacity = m_transfer_function_synthesizer -> getMaxOpacity();
    float max_density = m_transfer_function_synthesizer -> getMaxDensity();

    for ( int j = 0; j < nvariables; j++ )
    {
        values[j] = new float[nnodes];
        for ( int i = 0; i < nnodes; i++ )
        {
            int  it = j * nnodes  + i;
            values[j][i] = valueArray.at<Type>(it);  
        }
    } 
#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

#ifndef CPU_VER
    int mpi_rank;
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else 
    int mpi_rank = 0;
#endif

//    int mpi_rank;
//    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

    //if(mpi->rank==0)std::cout<<"start generate_particles\n";
    static bool start_flag = true;
    //static bool parameter_file_opened=false;
    static bool parameter_file_opened=true;

    std::vector< std::vector< pbvr::CellBase<Type>* > >  interp;

    interp.resize( max_threads );
    switch ( volume.cellType() )
    {
        case pbvr::VolumeObjectBase::Tetrahedra:
            {
                if (mpi_rank == 0) std::cout << "celltype: tetrahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::TetrahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::QuadraticTetrahedra:
            {
                if (mpi_rank == 0)std::cout << "Cell type : Quadratic tetrahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::QuadraticTetrahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::Hexahedra:
            {
                if (mpi_rank == 0) std::cout << "celltype: hexahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables  );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::HexahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::QuadraticHexahedra:
            {
                if (mpi_rank == 0) std::cout << "celltype: quadratichexahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::QuadraticHexahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::Prism:
            {
                if (mpi_rank == 0) std::cout << "celltype: prism " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::PrismaticCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case pbvr::VolumeObjectBase::Pyramid:
            {
                if (mpi_rank == 0) std::cout << "celltype: pyramid " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new pbvr::PyramidalCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
//        case pbvr::VolumeObjectBase::Triangle:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new pbvr::TriangleCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
//        case pbvr::VolumeObjectBase::QuadraticTriangle:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new pbvr::QuadraticTriangleCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
//        case pbvr::VolumeObjectBase::Square:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new pbvr::HexahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
//        case pbvr::VolumeObjectBase::QuadraticSquare:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new pbvr::HexahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
        default:
            {
                BaseClass::m_is_success = false;
                kvsMessageError( "Unsupported cell type." );
                return;
            }
    }
    //    PBVR_TIMER_END( 270 );

    int tf_number = m_transfer_function_array.size();

    SuperClass::m_color_histogram  = kvs::ValueArray<pbvr::FrequencyTable>( tf_number );
    SuperClass::m_opacity_histogram  = kvs::ValueArray<pbvr::FrequencyTable>( tf_number );

    const int max_nparticles = (int)m_transfer_function_synthesizer->getMaxDensity() + 1;

    if(mpi_rank==RANK) std::cout<<"******* max_nparticles="<<max_nparticles<<std::endl;

    int nbins = 256;
    kvs::ValueArray<float> o_min( tf_number );//read minmax from TFS
    kvs::ValueArray<float> o_max( tf_number );
    kvs::ValueArray<float> c_min( tf_number );
    kvs::ValueArray<float> c_max( tf_number );


    kvs::ValueArray<int> o_histogram( tf_number * nbins );// opacity histogram
    kvs::ValueArray<int> c_histogram( tf_number * nbins );// color histogram
    
    // 2023/07/31 add by shimomura
    SuperClass::m_c_histogram = kvs::ValueArray<int> (tf_number * nbins);
    SuperClass::m_o_histogram = kvs::ValueArray<int> (tf_number * nbins);
    SuperClass::setTfnumber(tf_number);
    SuperClass::setNbins(nbins);

    m_o_histogram.fill(0x00);
    m_c_histogram.fill(0x00);

//    if( parameter_file_opened )
//    {
//        O_min_recv.allocate(tf_number);
//        O_max_recv.allocate(tf_number);
//        C_min_recv.allocate(tf_number);
//        C_max_recv.allocate(tf_number);
////        o_histogram_recv.allocate(tf_number * nbins);
////        c_histogram_recv.allocate(tf_number * nbins);
//
//        o_histogram.fill(0x00);
//        c_histogram.fill(0x00);
//    }

    for( size_t i = 0; i < tf_number; i++ )
    {
        o_min[i] = m_transfer_function_array[i].opacityMap().minValue();
        o_max[i] = m_transfer_function_array[i].opacityMap().maxValue();
        c_min[i] = m_transfer_function_array[i].colorMap().minValue();
        c_max[i] = m_transfer_function_array[i].colorMap().maxValue();
    }
    //min max
    kvs::ValueArray<float> O_min( tf_number );// 4 calculate
    kvs::ValueArray<float> O_max( tf_number );
    kvs::ValueArray<float> C_min( tf_number );
    kvs::ValueArray<float> C_max( tf_number );

    // dynamic  array 4 paritcle
    std::vector<float> vertex_coords;
    std::vector<Byte>  vertex_colors;
    std::vector<float> vertex_normals;

    if( parameter_file_opened )
    {
        for ( size_t i = 0; i < tf_number; i++ ) // initialize
        {
            O_min[ i ] =  FLT_MAX;
            O_max[ i ] = -FLT_MAX;
            C_min[ i ] =  FLT_MAX;
            C_max[ i ] = -FLT_MAX;
        }
    }

    TransferFunctionSynthesizer** th_tfs = new TransferFunctionSynthesizer*[max_threads];
    std::vector< std::vector<pbvr::TransferFunction> > th_tf;

    for ( int n = 0; n < max_threads; n++ )
    {
        th_tfs[n] = new TransferFunctionSynthesizer( *m_transfer_function_synthesizer );
    }

    th_tf.resize( max_threads );
    for ( int i = 0; i < max_threads; i++ )
    {
        th_tf[ i ].resize( tf_number );
        for ( int j = 0; j < tf_number; j++ )
        {
            th_tf[i][j] = m_transfer_function_array[j];
        }
    }

#ifdef DEBUG
//    // debug input stab data add by shimomura 2022/12/28 
//    for ( int n = 0; n < max_threads; n++ )
//    {
//    th_tfs[n] -> setStabToken();
//    }
//    //
#endif

    float particle_data_size_limit = m_transfer_function_synthesizer -> getParticleDataSizeLimit();
    int particles_process_limit = static_cast<int> (  ( particle_data_size_limit * 10E6 )
            / ( sizeof( float ) + sizeof( Byte ) + sizeof( float ) ) );
    bool particle_limit_over = false;
    // coordinate synthesis
    // 2023 shimomura 
    const pbvr::CoordSynthesizerTokens* pCrdSynthTkn = volume.getCoordSynthesizerTokens();
    CoordSynthesizerTokens cst;
    if ( pCrdSynthTkn ) cst = *pCrdSynthTkn;

    const pbvr::CoordSynthesizerStrings* pCrdSynthStr = volume.getCoordSynthesizerStrings();
    CoordSynthesizerStrings css;
    if ( pCrdSynthStr ) 
    {
        css = *pCrdSynthStr;
    }

    if (! css.m_x_coord_synthesizer_string.empty() )cst.x_token_empty=false;
    if (! css.m_y_coord_synthesizer_string.empty() )cst.y_token_empty=false;
    if (! css.m_z_coord_synthesizer_string.empty() )cst.z_token_empty=false;

#pragma omp parallel
    {
#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif

       kvs::MersenneTwister MT( thid + mpi_rank * nthreads );

        std::vector<float> th_vertex_coords;
        std::vector<Byte>  th_vertex_colors;
        std::vector<float> th_vertex_normals;


        //¿¿¿¿¿¿¿¿¿
        std::vector<float> o_scalars( tf_number );//vertex opacity
        std::vector<float> c_scalars( tf_number );//vertex color
        kvs::ValueArray<int> th_o_histogram( tf_number * nbins );// opacity
        kvs::ValueArray<int> th_c_histogram( tf_number * nbins );// color

        if( parameter_file_opened )
        {
            th_o_histogram.fill(0x00);
            th_c_histogram.fill(0x00);
        }

        //¿¿¿¿¿
        kvs::ValueArray<float> th_O_min( tf_number );//¿¿¿¿¿¿¿¿¿¿¿
        kvs::ValueArray<float> th_O_max( tf_number );
        kvs::ValueArray<float> th_C_min( tf_number );
        kvs::ValueArray<float> th_C_max( tf_number );

        if( parameter_file_opened )
        {
            for ( int i = 0; i < tf_number; i++ ) //¿¿¿
            {
                th_O_min[ i ] =  FLT_MAX;
                th_O_max[ i ] = -FLT_MAX;
                th_C_min[ i ] =  FLT_MAX;
                th_C_max[ i ] = -FLT_MAX;
            }
        }

        // -----------------------------------
        //¿¿¿¿¿
        kvs::Vector3f local_center_array[ SIMDW ];
        kvs::Vector3f global_center_array[ SIMDW ];
        kvs::UInt32 cell_index[ SIMDW ];

        float cell_opacity_array[ SIMDW ];
        std::vector<float> o_scalars_array[ SIMDW ];
        std::vector<float> c_scalars_array[ SIMDW ];

        for (int i = 0; i < SIMDW; i++ )
        {
            o_scalars_array[i].resize( tf_number );
            c_scalars_array[i].resize( tf_number );
        }

        int nparticles_array[ SIMDW ];

        kvs::Vector3f local_coord_array[ SIMDW ];
        kvs::Vector3f global_coord_array[ SIMDW ];
        float density_array[ SIMDW ];

        kvs::Vector3f l_plus_coord[ SIMDW ];
        kvs::Vector3f l_minus_coord[ SIMDW ];
        kvs::Vector3f g_plus_coord[ SIMDW ];
        kvs::Vector3f g_minus_coord[ SIMDW ];
        float S_plus_opacity[ SIMDW ];
        float S_minus_opacity[ SIMDW ];
        float dsdx_array[ SIMDW ];
        float dsdy_array[ SIMDW ];
        float dsdz_array[ SIMDW ];
        kvs::Vector3f grad_array[ SIMDW ];
        kvs::RGBColor color_array[ SIMDW ];
        // -----------------------------------

        //¿¿¿¿¿¿¿¿¿
#pragma omp for schedule( dynamic ) nowait  
        //#pragma omp for schedule( static ) nowait
        //#pragma omp for schedule( static, 1 ) nowait
        for( int cell_base = 0; cell_base < ncells; cell_base += SIMDW )
        {
            //¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
            int remain = ( ncells - cell_base > SIMDW )? SIMDW: ncells - cell_base;

            /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            //¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (kvs::UInt32)(cell_base + cell_BLK);
                local_center_array[cell_BLK] = kvs::Vector3f ( 0.5, 0.5, 0.5 );
            }

            //¿¿¿¿¿¿¿¿¿¿¿¿¿¿
            for(int i = 0; i < nvariables; i++)
            {
                interp[thid][i]->bindCellArray(remain, cell_index);
            }

            interp[thid][0]->setLocalPointArray( remain, local_center_array );
            interp[thid][0]->transformLocalToGlobalArray( remain,
                    local_center_array,
                    global_center_array );

            if( parameter_file_opened )
            {

                th_tfs[thid]->SynthesizedOpacityScalarsArray( interp[thid],
                        remain,
                        local_center_array,
                        global_center_array,
                        o_scalars_array );

                th_tfs[thid]->SynthesizedColorScalarsArray( interp[thid],
                        remain,
                        local_center_array,
                        global_center_array,
                        c_scalars_array );

                for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ ) 
                {
                    for( int i = 0; i < tf_number; i++ )
                    {
                        float h = (o_scalars_array[cell_BLK][i] - o_min[i])/( o_max[i] - o_min[i] )*nbins;
                        int H = (int)h;
                        if( 0 <= H && H <= nbins )
                        {
                            if( H == nbins ) H--;
                            th_o_histogram[ H + nbins*i]++;
                        }

                        h = (c_scalars_array[cell_BLK][i] - c_min[i])/( c_max[i] - c_min[i] )*nbins;
                        H = (int)h;
                        if( 0 <= H && H <= nbins )
                        {
                            if( H == nbins ) H--;
                            th_c_histogram[ H + nbins*i]++;
                        }
                        // 20190128 ¿¿
                        th_O_min[i] = th_O_min[i] < o_scalars_array[cell_BLK][i] ? th_O_min[i] : o_scalars_array[cell_BLK][i];
                        th_O_max[i] = th_O_max[i] > o_scalars_array[cell_BLK][i] ? th_O_max[i] : o_scalars_array[cell_BLK][i];
                        th_C_min[i] = th_C_min[i] < c_scalars_array[cell_BLK][i] ? th_C_min[i] : c_scalars_array[cell_BLK][i];
                        th_C_max[i] = th_C_max[i] > c_scalars_array[cell_BLK][i] ? th_C_max[i] : c_scalars_array[cell_BLK][i];
                        
                    }
                }
            }

//            th_tfs[thid]->CalculateOpacityArray( interp[thid],
                th_tfs[thid]->CalculateOpacityArrayAverage( interp[thid],
                    remain,
                    local_center_array,
                    global_center_array,
                    th_tf[thid],
                    cell_opacity_array );

            //clac num of particles in the each cell
            int nparticles_num = 0;
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                    const float density = Generator::CalculateDensity( cell_opacity_array[cell_BLK],
                            sampling_volume_inverse,
                            max_opacity, max_density );
                    //interp[thid][0]->bindCellWoVol( cell_index[cell_BLK] );
                    interp[thid][0]->bindCell( cell_index[cell_BLK] );
                    nparticles_array[cell_BLK] 
                        = calculate_number_of_particles( density, interp[thid][0]->volume(), &MT );
                nparticles_array[cell_BLK] *= m_particle_density;
                nparticles_num += nparticles_array[cell_BLK];
            }
            /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////

            /////////////////////////////// CalculateOpacity(), CalculateColor() ///////////////////////////////////
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                // ------------------------------------------------

                for( int i = 0; i < nparticles_array[cell_BLK]; i+=SIMD_BLK_SIZE )
                {
                    //¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
                    int remain_BLK = ( nparticles_array[cell_BLK] - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: nparticles_array[cell_BLK] - i;

                    //¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = cell_base + cell_BLK;
                            local_coord_array[j] = interp[thid][0] -> randomSampling_MT( &MT);

                            //¿¿¿¿¿¿¿¿¿¿¿¿¿¿
                            for( int k = 0; k < nvariables; k++ )
                            {
                                interp[thid][k]->bindCell( cell_index[j] );
                            }

                            interp[thid][0]->setLocalPoint( local_coord_array[j] );
                            global_coord_array[j] = interp[thid][0]->transformLocalToGlobal( local_coord_array[j] );
                    }
                    
                    //¿¿¿¿¿cell_index, local_coord¿¿¿¿
                    int nparticles_count = 0;
                    for( int j = 0; j < remain_BLK; j++ )
                    {
                        cell_index[ nparticles_count ] = cell_index[j];
                        local_coord_array[ nparticles_count ] = local_coord_array[j];
                        global_coord_array[ nparticles_count ] = global_coord_array[j];
                        nparticles_count +=1;
                    }

                    // ------------------------------------------------

                    // bind cell in bulk 
                    for( int j = 0; j < nvariables; j++ )
                    {
                        interp[thid][j]->bindCellArray( nparticles_count, cell_index );
                    }

                    // dsdx ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + kvs::Vector3f(0.1,0,0);
                        l_minus_coord[j] = local_coord_array[j] + kvs::Vector3f(-0.1,0,0);
                    }

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                            l_plus_coord,
                            g_plus_coord );

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                            l_minus_coord,
                            g_minus_coord );

                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                            nparticles_count,
                            l_plus_coord,
                            g_plus_coord,
                            th_tf[thid],
                            S_plus_opacity );
                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                            nparticles_count,
                            l_minus_coord,
                            g_minus_coord,
                            th_tf[thid],
                            S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        dsdx_array[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
                    // ------------------------------------------------
                    // dsdy ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,0.1,0);
                        l_minus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,-0.1,0);
                    }

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                            l_plus_coord,
                            g_plus_coord );

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                            l_minus_coord,
                            g_minus_coord );

                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                            nparticles_count,
                            l_plus_coord,
                            g_plus_coord,
                            th_tf[thid],
                            S_plus_opacity );
                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                            nparticles_count,
                            l_minus_coord,
                            g_minus_coord,
                            th_tf[thid],
                            S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        dsdy_array[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
                    // ------------------------------------------------
                    // dsdz ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,0,0.1);
                        l_minus_coord[j] = local_coord_array[j] + kvs::Vector3f(0,0,-0.1);
                    }

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                            l_plus_coord,
                            g_plus_coord );

                    interp[thid][0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[thid][0]->transformLocalToGlobalArray( nparticles_count,
                            l_minus_coord,
                            g_minus_coord );

                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                            nparticles_count,
                            l_plus_coord,
                            g_plus_coord,
                            th_tf[thid],
                            S_plus_opacity );
                    th_tfs[thid]->CalculateOpacityArray( interp[thid],
                            nparticles_count,
                            l_minus_coord,
                            g_minus_coord,
                            th_tf[thid],
                            S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        dsdz_array[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
                    // ------------------------------------------------
                    //grad_array¿¿¿
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        //JacobiMatrix¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
                        interp[thid][0]->bindCell( cell_index[j] );

                        const kvs::Vector3f g( -dsdx_array[j], -dsdy_array[j], -dsdz_array[j] );
                        const kvs::Matrix33f J = interp[thid][0]->JacobiMatrix();
                        float determinant = 0.0f;
                        const kvs::Vector3f G = J.inverse( &determinant ) * g;
                        grad_array[j] = kvs::Math::IsZero( determinant ) ? kvs::Vector3f( 0.0f, 0.0f, 0.0f ) : G;
                    }

                    //color
                    th_tfs[thid]->CalculateColorArray( interp[thid],
                            nparticles_count,
                            local_coord_array,
                            global_coord_array,
                            th_tf[thid],
                            color_array );

                    kvs::Vector3f new_coord_array[ SIMDW ];
                    if ( pCrdSynthStr )
                    {
                        th_tfs[thid]->CalculateCoordArray( interp[thid],
                                nparticles_count,
                                local_coord_array,
                                global_coord_array,
                                th_tf[thid],
      /*CoordSynthesizerTokens*/        cst,
                                new_coord_array );
                    }
                   
                    //2023 shimomura 
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        kvs::Vector3f new_coord = new_coord_array[j];
                        th_vertex_coords.push_back( new_coord.x() );
                        th_vertex_coords.push_back( new_coord.y() );
                        th_vertex_coords.push_back( new_coord.z() );

                        th_vertex_colors.push_back( color_array[j].r() );
                        th_vertex_colors.push_back( color_array[j].g() );
                        th_vertex_colors.push_back( color_array[j].b() );

                        th_vertex_normals.push_back( grad_array[j].x() );
                        th_vertex_normals.push_back( grad_array[j].y() );
                        th_vertex_normals.push_back( grad_array[j].z() );
                    }
                    // ------------------------------------------------
                }//end of for i
            }
            /////////////////////////////// CalculateOpacity(), CalculateColor() ///////////////////////////////////
        }// end of for cell

#pragma omp critical
        {
            if( parameter_file_opened )
            {
                //¿¿¿¿¿
                for( int i = 0; i < tf_number; i++ )
                {
                    //¿¿¿¿
                    O_min[i] = O_min[i] < th_O_min[i] ? O_min[i] : th_O_min[i];
                    O_max[i] = O_max[i] > th_O_max[i] ? O_max[i] : th_O_max[i];
                    //¿
                    C_min[i] = C_min[i] < th_C_min[i] ? C_min[i] : th_C_min[i];
                    C_max[i] = C_max[i] > th_C_max[i] ? C_max[i] : th_C_max[i];
                }
            }   

            for( int n = 0; n < tf_number * nbins; n++ )
            {
//                o_histogram[n] += th_o_histogram[n];
//                c_histogram[n] += th_c_histogram[n];
                m_o_histogram[n] += th_o_histogram[n];
                m_c_histogram[n] += th_c_histogram[n];
            }

            vertex_coords.insert ( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            vertex_colors.insert ( vertex_colors.end(), th_vertex_colors.begin(), th_vertex_colors.end() );
            vertex_normals.insert( vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end() );
        }


    } //#pragma omp parallel

//        O_min_recv.fill(0x00);
//        O_max_recv.fill(0x00);
//        C_min_recv.fill(0x00);
//        C_max_recv.fill(0x00);
//
//        //¿¿¿¿¿¿¿¿¿
//        o_histogram_recv.fill(0x00);
//        c_histogram_recv.fill(0x00);

    // add by shimomura 
    // set minmax range
    
    m_transfer_function_synthesizer->m_o_min.resize(tf_number);        
    m_transfer_function_synthesizer->m_o_max.resize(tf_number);        
    m_transfer_function_synthesizer->m_c_min.resize(tf_number);        
    m_transfer_function_synthesizer->m_c_max.resize(tf_number);        
    for (int i = 0; i < tf_number; i++)
    {
        m_transfer_function_synthesizer->m_o_min[i] = O_min[i];
        m_transfer_function_synthesizer->m_o_max[i] = O_max[i];
        m_transfer_function_synthesizer->m_c_min[i] = C_min[i];
        m_transfer_function_synthesizer->m_c_max[i] = C_max[i];
    }

    //delete tfs;  // ¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿

    for(int i=0; i<max_threads; i++)
    {
        delete th_tfs[i];
    }
    delete[] th_tfs;
    for ( int i = 0; i < max_threads; i++ )
    {
        for ( int j = 0; j < interp[i].size(); j++ )
        {
           if (interp[i][j] != NULL)delete interp[i][j];
        }
    }

    for (int i = 0; i < nvariables; i++)
    {
        delete[] values[i];
    }
    delete[] values;

    //TIMER_END( 290 );

    SuperClass::m_coords  = kvs::ValueArray<kvs::Real32>( vertex_coords );
    SuperClass::m_colors  = kvs::ValueArray<kvs::UInt8>( vertex_colors );
    SuperClass::m_normals = kvs::ValueArray<kvs::Real32>( vertex_normals );

//#endif
    SuperClass::setSize( 1.0f );
    //TIMER_END( 280 );
    double end = GetTime();
    printf( " \n  CPU:generate_particles: %lf ms\n", end - start );
}

const size_t CellByCellUniformSampling::calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    kvs::MersenneTwister* MT ) 
{
    const float N = density * volume_of_cell;
    const float R = MT->rand();
    //const float R = 0.5;

    size_t n = static_cast<size_t>( N ); 
    if ( N - n > R )
    {    
        ++n; 
    }    

    return ( n ); 
}


#ifdef ENABLE_MPI
void CellByCellUniformSampling::generate_particles_gt5d(
    const pbvr::UnstructuredVolumeObject* volume )
{
    // Vertex data arrays. (output)
    std::vector<kvs::Real32> vertex_coords;
    std::vector<kvs::UInt8>  vertex_colors;
    std::vector<kvs::Real32> vertex_normals;

    // Set a tetrahedral cell interpolator.
    pbvr::CellBase<kvs::Real32>* cell = NULL;
    switch ( volume.cellType() )
    {
    case pbvr::VolumeObjectBase::Tetrahedra:
    {
        cell = new pbvr::TetrahedralCell<kvs::Real32>( volume );
        break;
    }
    case pbvr::VolumeObjectBase::QuadraticTetrahedra:
    {
        cell = new pbvr::QuadraticTetrahedralCell<kvs::Real32>( volume );
        break;
    }
    case pbvr::VolumeObjectBase::Hexahedra:
    {
        cell = new pbvr::HexahedralCell<kvs::Real32>( volume );
        break;
    }
    case pbvr::VolumeObjectBase::QuadraticHexahedra:
    {
        cell = new pbvr::QuadraticHexahedralCell<kvs::Real32>( volume );
        break;
    }
    case pbvr::VolumeObjectBase::Prism:
    {
        cell = new pbvr::PrismaticCell<kvs::Real32>( volume );
        break;
    }
    case pbvr::VolumeObjectBase::Pyramid:
    {
        cell = new pbvr::PyramidalCell<kvs::Real32>( volume );
        break;
    }
    default:
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Unsupported cell type." );
        return;
    }
    }

    double start = GetTime();

    const kvs::ColorMap color_map( BaseClass::transferFunction().colorMap() );

    int m_sampling_method = MPIController::UNIFORM_SAMPLING;
    MPI_Bcast( &m_sampling_method, 1, MPI_INT, 0, MPI_COMM_WORLD );
    DistributedUniformSampling::generate_particles_gt5d_master_gpu(
        volume, m_density_map.pointer(), color_map.table().pointer(),
        BaseClass::transferFunction().resolution(),
        BaseClass::transferFunction().colorMap().minValue(),
        BaseClass::transferFunction().colorMap().maxValue(),
        SuperClass::m_coords,
        SuperClass::m_colors,
        SuperClass::m_normals );

    delete cell;
}
#endif

} // end of namespace pbvr

