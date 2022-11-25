/****************************************************************************/
/**
 *  @file CellByCellRejectionSampling.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellByCellRejectionSampling.cpp 633 2010-10-10 05:12:37Z naohisa.sakamoto $
 */
/****************************************************************************/
#include "CellByCellRejectionSampling.h"
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
#include "GlobalCore.h"

#include "common.h"

#include <cmath>

#ifdef ENABLE_MPI
#include <mpi.h>
#include "mpi_controller.h"
#include <kvs/DistributedRejectionSampling>
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
 *  @brief  Constructs a new CellByCellRejectionSampling class.
 */
/*===========================================================================*/
CellByCellRejectionSampling::CellByCellRejectionSampling():
    pbvr::MapperBase(),
    pbvr::PointObject(),
    m_camera( 0 )
{
}

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellRejectionSampling class.
 *  @param  volume [in] pointer to the volume object
 *  @param  m_subpixel_level [in] sub-pixel level
 *  @param  m_sampling_step [in] sapling step
 *  @param  m_transfer_function [in] transfer function
 *  @param  object_depth [in] depth value of the input volume at the CoG
 */
/*===========================================================================*/
CellByCellRejectionSampling::CellByCellRejectionSampling(
    const pbvr::VolumeObjectBase& volume,
    const size_t                 subpixel_level,
    const float                  sampling_step,
    const pbvr::TransferFunction& transfer_function,
    TransferFunctionSynthesizer* transfunc_synthesizer,
    const CropRegion&            crop,
    const float                  object_depth ):
    pbvr::MapperBase( transfer_function ),
    pbvr::PointObject(),
    m_transfunc_synthesizer( transfunc_synthesizer ),
    m_camera( 0 ),
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
 *  @brief  Constructs a new CellByCellRejectionSampling class.
 *  @param  camera [in] pointer to the camera
 *  @param  volume [in] pointer to the volume object
 *  @param  subpixel_level [in] sub-pixel level
 *  @param  sampling_step [in] sapling step
 *  @param  transfer_function [in] transfer function
 *  @param  object_depth [in] depth value of the input volume at the CoG
 */
/*===========================================================================*/
CellByCellRejectionSampling::CellByCellRejectionSampling(
    const kvs::Camera&           camera,
    const pbvr::VolumeObjectBase& volume,
    const size_t                 subpixel_level,
    const float                  sampling_step,
    const pbvr::TransferFunction& transfer_function,
    TransferFunctionSynthesizer* transfunc_synthesizer,
    const CropRegion&            crop,
    const float                  particle_density,
    const float                  object_depth ):
    pbvr::MapperBase( transfer_function ),
    pbvr::PointObject(),
    m_transfunc_synthesizer( transfunc_synthesizer ),
    m_particle_density( particle_density ),
    m_batch( false )
{
    m_crop = crop;
    this->attachCamera( camera ),
         this->setSubpixelLevel( subpixel_level );
    this->setSamplingStep( sampling_step );
    this->setObjectDepth( object_depth );
    this->exec( volume );
}

CellByCellRejectionSampling::CellByCellRejectionSampling(
    const kvs::Camera&           camera,
    const pbvr::VolumeObjectBase& volume,
    const size_t                 subpixel_level,
    const float                  sampling_step,
    const pbvr::TransferFunction& transfer_function,
    TransferFunctionSynthesizer* transfunc_synthesizer,
    const CropRegion&            crop,
    const float                  particle_density,
    const bool                   batch,
    const float                  object_depth ):
    pbvr::MapperBase( transfer_function ),
    pbvr::PointObject(),
    m_transfunc_synthesizer( transfunc_synthesizer ),
    m_particle_density( particle_density ),
    m_batch( batch )
{
    m_crop = crop;
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
CellByCellRejectionSampling::~CellByCellRejectionSampling()
{
    m_density_map.deallocate();
}

/*===========================================================================*/
/**
 *  @brief  Retruns the sub-pixel level.
 *  @return sub-pixel level
 */
/*===========================================================================*/
const size_t CellByCellRejectionSampling::subpixelLevel() const
{
    return m_subpixel_level;
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampling step.
 *  @return sampling step
 */
/*===========================================================================*/
const float CellByCellRejectionSampling::samplingStep() const
{
    return m_sampling_step;
}

/*===========================================================================*/
/**
 *  @brief  Returns the depth of the object at the center of the gravity.
 *  @return depth
 */
/*===========================================================================*/
const float CellByCellRejectionSampling::objectDepth() const
{
    return m_object_depth;
}

/*===========================================================================*/
/**
 *  @brief  Attaches a camera.
 *  @param  camera [in] pointer to the camera
 */
/*===========================================================================*/
void CellByCellRejectionSampling::attachCamera( const kvs::Camera& camera )
{
    m_camera = &camera;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sub-pixel level.
 *  @param  m_subpixel_level [in] sub-pixel level
 */
/*===========================================================================*/
void CellByCellRejectionSampling::setSubpixelLevel( const size_t subpixel_level )
{
    m_subpixel_level = subpixel_level;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sampling step.
 *  @param  m_sampling_step [in] sampling step
 */
/*===========================================================================*/
void CellByCellRejectionSampling::setSamplingStep( const float sampling_step )
{
    m_sampling_step = sampling_step;
}

/*===========================================================================*/
/**
 *  @brief  Sets a depth of the object at the center of the gravity.
 *  @param  object_depth [in] depth
 */
/*===========================================================================*/
void CellByCellRejectionSampling::setObjectDepth( const float object_depth )
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
CellByCellRejectionSampling::SuperClass* CellByCellRejectionSampling::exec( const pbvr::ObjectBase& object )
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
void CellByCellRejectionSampling::generate_particles<kvs::Real32>( const pbvr::UnstructuredVolumeObject& volume );

/*===========================================================================*/
/**
 *  @brief  Mapping for the unstructured volume object.
 *  @param  camera [in] pointer to the camera
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
void CellByCellRejectionSampling::mapping( const kvs::Camera& camera, const pbvr::UnstructuredVolumeObject& volume )
{
    // Attach the pointer to the volume object and set the min/max coordinates.
    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );

    const pbvr::VolumeObjectBase *object = BaseClass::volume();

    // Calculate the density map.
    if ( m_transfunc_synthesizer )
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

        m_transfunc_synthesizer->setMaxOpacity( max_opacity );
        m_transfunc_synthesizer->setMaxDensity( max_density );
        m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );
    }
    else
    {
        m_density_map = Generator::CalculateDensityMap(
                            camera,
                            *object,
                            static_cast<float>( m_subpixel_level ),
                            m_sampling_step,
                            BaseClass::transferFunction().opacityMap() );
    }

    // Generate the particles.
    const std::type_info& type = volume.values().typeInfo()->type();
    if (      type == typeid( kvs::Int8   ) ) this->generate_particles<kvs::Int8>( volume );
    else if ( type == typeid( kvs::Int16  ) ) this->generate_particles<kvs::Int16>( volume );
    else if ( type == typeid( kvs::Int32  ) ) this->generate_particles<kvs::Int32>( volume );
    else if ( type == typeid( kvs::UInt8  ) ) this->generate_particles<kvs::UInt8>( volume );
    else if ( type == typeid( kvs::UInt16 ) ) this->generate_particles<kvs::UInt16>( volume );
    else if ( type == typeid( kvs::UInt32 ) ) this->generate_particles<kvs::UInt32>( volume );
    else if ( type == typeid( kvs::Real32 ) ) this->generate_particles<kvs::Real32>( volume );
    else if ( type == typeid( kvs::Real64 ) ) this->generate_particles<kvs::Real64>( volume );
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
void CellByCellRejectionSampling::generate_particles( const pbvr::UnstructuredVolumeObject& volume )
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
    default:
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Unsupported cell type." );
        return;
    }
    }

    const size_t veclen = volume.veclen();
    const kvs::ColorMap color_map( BaseClass::transferFunction().colorMap() );

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

    // Generate particles for each cell.
    const size_t ncells = volume.ncells();
    for ( size_t index = 0; index < ncells; ++index )
    {
        // add by @hira by 2016/12/01
        if (cell == NULL) continue;

        // Bind the cell which is indicated by 'index'.
        cell->bindCell( index );

        // Calculate a number of particles in this cell.
        const float averaged_scalar = cell->averagedScalar();
        float density = this->calculate_density( averaged_scalar );
#ifdef KVS_PLATFORM_MACOSX
        if ( std::isnan( density ) )
#else
	if ( std::isnan( density ) )
#endif
        {
            density = 0;
        }
        const size_t nparticles = this->calculate_number_of_particles( density, cell->volume() );

        const T* S = cell->scalars();
        const size_t nnodes = volume.cellType();
        T S_min = S[0];
        T S_max = S[0];
        for ( size_t i = 1; i < nnodes; i++ )
        {
            S_min = kvs::Math::Min( S_min, S[i] );
            S_max = kvs::Math::Max( S_max, S[i] );
        }
        const float s_min = static_cast<float>( S_min );
        const float s_max = static_cast<float>( S_max );
        const float p_max = this->calculate_maximum_density( s_min, s_max ) / nparticles;

        // Generate a set of particles in this cell represented by v0,...,v3 and s0,...,s3.
        size_t count = 0;
        while ( count < nparticles )
        {
            const kvs::Vector3f coord = cell->randomSampling();
            const float scalar = cell->scalar();
            const float density = this->calculate_density( scalar );

            const float p = density / nparticles;
            const float R = Generator::GetRandomNumber();
            if ( p > p_max * R )
            {
                // Calculate a color.
                const kvs::RGBColor color( color_map.at( scalar ) );

                // Calculate a normal.
                const kvs::Vector3f normal( cell->gradient() );

                // using coord synthesizer
                kvs::Vector3f new_coord = coord;
                if ( pCrdSynthStr )
                {
                    X = x = coord.x();
                    Y = y = coord.y();
                    Z = z = coord.z();
                    size_t qn;
                    for ( qn = 0; qn < veclen; qn++ )
                    {
                        cell->bindCell( index, qn );
                        synth_vars[qn + 8] = cell->scalar();
                    }
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

                count++;
            }
        } // end of 'paricle' while-loop
    } // end of 'cell' for-loop

    SuperClass::m_coords  = kvs::ValueArray<kvs::Real32>( vertex_coords );
    SuperClass::m_colors  = kvs::ValueArray<kvs::UInt8>( vertex_colors );
    SuperClass::m_normals = kvs::ValueArray<kvs::Real32>( vertex_normals );
    SuperClass::setSize( 1.0f );

    delete cell;
}

template <>
void CellByCellRejectionSampling::generate_particles<kvs::Real32>( const pbvr::UnstructuredVolumeObject& volume )
{
    float particle_density = m_particle_density;
    /*****generate histogram******/
    int tf;
    size_t c_tf_count                           = m_transfunc_synthesizer->colorTransferFunctionMap().size();
    TransferFunctionMap::const_iterator c_begin = m_transfunc_synthesizer->colorTransferFunctionMap().begin();
    TransferFunctionMap::const_iterator c_end   = m_transfunc_synthesizer->colorTransferFunctionMap().end();

    size_t o_tf_count                           = m_transfunc_synthesizer->opacityTransferFunctionMap().size();
    TransferFunctionMap::const_iterator o_begin = m_transfunc_synthesizer->opacityTransferFunctionMap().begin();
    TransferFunctionMap::const_iterator o_end   = m_transfunc_synthesizer->opacityTransferFunctionMap().end();

    size_t resolution = DEFAULT_NBINS;

    SuperClass::m_color_histogram  = kvs::ValueArray<pbvr::FrequencyTable>( c_tf_count );
    SuperClass::m_opacity_histogram  = kvs::ValueArray<pbvr::FrequencyTable>( o_tf_count );

    kvs::ValueArray<std::string> name_tf_c_var( c_tf_count );
    kvs::ValueArray<std::string> name_tf_o_var( o_tf_count );

    kvs::ValueArray<size_t> c_bin_count( c_tf_count );
    kvs::ValueArray<size_t> o_bin_count( o_tf_count );

    kvs::ValueArray<float> c_min( c_tf_count );
    kvs::ValueArray<float> c_max( c_tf_count );
    kvs::ValueArray<float> o_min( o_tf_count );
    kvs::ValueArray<float> o_max( o_tf_count );

    if ( !m_batch )
    {
        tf = 0;
        for ( TransferFunctionMap::const_iterator t = c_begin; t != c_end; t++ )
        {
            c_min[tf] = t->second.colorMap().minValue();
            c_max[tf] = t->second.colorMap().maxValue();
            c_bin_count[tf] = 0;
            name_tf_c_var[ tf ] = t->first + "_var_c";
            tf++;
        }

        tf = 0;
        for ( TransferFunctionMap::const_iterator t = o_begin; t != o_end; t++ )
        {
            o_min[tf] = t->second.opacityMap().minValue();
            o_max[tf] = t->second.opacityMap().maxValue();
            o_bin_count[tf] = 0;
            name_tf_o_var[ tf ] = t->first + "_var_o";
            tf++;
        }
    }
    /*****************************/

    if ( volume.ncells() < 1 ) return;
    const size_t veclen = volume.veclen();

    const pbvr::CoordSynthesizerStrings* pCrdSynthStr = volume.getCoordSynthesizerStrings();
    CoordSynthesizerStrings css;
    if ( pCrdSynthStr ) css = *pCrdSynthStr;

    PBVR_TIMER_STA( 268 );
    // Vertex data arrays. (output)
    std::vector<kvs::Real32> vertex_coords;
    std::vector<kvs::UInt8>  vertex_colors;
    std::vector<kvs::Real32> vertex_normals;
    PBVR_TIMER_END( 268 );

#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif

    size_t nnodes = volume.cellType();
    PBVR_TIMER_STA( 270 );
    // Set a tetrahedral cell interpolator.
    pbvr::CellBase<kvs::Real32>** cell = new pbvr::CellBase<kvs::Real32>* [max_threads];
    pbvr::CellBase<kvs::Real32>** cell_opacity = new pbvr::CellBase<kvs::Real32>* [max_threads];
    switch ( volume.cellType() )
    {
    case pbvr::VolumeObjectBase::Tetrahedra:
    {
        for ( int n = 0; n < max_threads; n++ )
        {
            cell[n] = new pbvr::TetrahedralCell<kvs::Real32>( volume );
            cell_opacity[n] = new pbvr::TetrahedralCell<kvs::Real32>( volume );
        }
        break;
    }
    case pbvr::VolumeObjectBase::QuadraticTetrahedra:
    {
        for ( int n = 0; n < max_threads; n++ )
        {
            cell[n] = new pbvr::QuadraticTetrahedralCell<kvs::Real32>( volume );
            cell_opacity[n] = new pbvr::QuadraticTetrahedralCell<kvs::Real32>( volume );
        }
        break;
    }
    case pbvr::VolumeObjectBase::Hexahedra:
    {
        for ( int n = 0; n < max_threads; n++ )
        {
            cell[n] = new pbvr::HexahedralCell<kvs::Real32>( volume );
            cell_opacity[n] = new pbvr::HexahedralCell<kvs::Real32>( volume );
        }
        break;
    }
    case pbvr::VolumeObjectBase::QuadraticHexahedra:
    {
        for ( int n = 0; n < max_threads; n++ )
        {
            cell[n] = new pbvr::QuadraticHexahedralCell<kvs::Real32>( volume );
            cell_opacity[n] = new pbvr::QuadraticHexahedralCell<kvs::Real32>( volume );
        }
        break;
    }
    case pbvr::VolumeObjectBase::Prism:
    {
        for ( int n = 0; n < max_threads; n++ )
        {
            cell[n] = new pbvr::PrismaticCell<kvs::Real32>( volume );
            cell_opacity[n] = new pbvr::PrismaticCell<kvs::Real32>( volume );
        }
        break;
    }
    case pbvr::VolumeObjectBase::Pyramid:
    {
        for ( int n = 0; n < max_threads; n++ )
        {
            cell[n] = new pbvr::PyramidalCell<kvs::Real32>( volume );
            cell_opacity[n] = new pbvr::PyramidalCell<kvs::Real32>( volume );
        }
        break;
    }
    case pbvr::VolumeObjectBase::Triangle:
    {
        for ( int n = 0; n < max_threads; n++ )
        {
            cell[n] = new pbvr::TriangleCell<kvs::Real32>( volume );
            cell_opacity[n] = new pbvr::TriangleCell<kvs::Real32>( volume );
        }
        break;
    }
    case pbvr::VolumeObjectBase::QuadraticTriangle:
    {
        for ( int n = 0; n < max_threads; n++ )
        {
            cell[n] = new pbvr::QuadraticTriangleCell<kvs::Real32>( volume );
            cell_opacity[n] = new pbvr::QuadraticTriangleCell<kvs::Real32>( volume );
        }
        nnodes = 6;
        break;
    }
    case pbvr::VolumeObjectBase::Square:
    {
        for ( int n = 0; n < max_threads; n++ )
        {
            cell[n] = new pbvr::SquareCell<kvs::Real32>( volume );
            cell_opacity[n] = new pbvr::SquareCell<kvs::Real32>( volume );
        }
        nnodes = 4;
        break;
    }
    case pbvr::VolumeObjectBase::QuadraticSquare:
    {
        for ( int n = 0; n < max_threads; n++ )
        {
            cell[n] = new pbvr::QuadraticSquareCell<kvs::Real32>( volume );
            cell_opacity[n] = new pbvr::QuadraticSquareCell<kvs::Real32>( volume );
        }
        nnodes = 8;
        break;
    }
    default:
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Unsupported cell type." );
        return;
    }
    }
    PBVR_TIMER_END( 270 );

    PBVR_TIMER_STA( 280 );
//     double start = GetTime();
    double start = GetTime();
    const kvs::ColorMap color_map( BaseClass::transferFunction().colorMap() );
#ifdef ENABLE_MPI
    int m_sampling_method = MPIController::REJECTION_SAMPLING;
    MPI_Bcast( &m_sampling_method, 1, MPI_INT, 0, MPI_COMM_WORLD );
    DistributedRejectionSampling::generate_particles_master_cpu(
        volume, m_density_map.pointer(), color_map.table().pointer(),
        BaseClass::transferFunction().resolution(),
        BaseClass::transferFunction().colorMap().minValue(),
        BaseClass::transferFunction().colorMap().maxValue(),
        SuperClass::m_coords,
        SuperClass::m_colors,
        SuperClass::m_normals );
#else
    TransferFunctionSynthesizer** tfs = new TransferFunctionSynthesizer*[max_threads];
    for ( int n = 0; n < max_threads; n++ )
    {
        tfs[n] = new TransferFunctionSynthesizer( *m_transfunc_synthesizer );
    }

    PBVR_TIMER_STA( 290 );
#ifdef OLD_PARALLEL
    #pragma omp parallel
    {
        kvs::Timer timer;
        std::vector<kvs::Real32> th_vertex_coords;
        std::vector<kvs::UInt8>  th_vertex_colors;
        std::vector<kvs::Real32> th_vertex_normals;

#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif

        //std::cout << "thread " << thid << " start" << std::endl;
        timer.start();
        PBVR_TIMER_STA( 300 );

        // Generate particles for each cell.
        const size_t ncells = volume.ncells();
        for ( size_t index = thid; index < ncells; index += nthreads )
        {
            PBVR_TIMER_STA( 291 );
            // Bind the cell which is indicated by 'index'.
            cell[thid]->bindCell( index );
            if ( !m_crop.isinner( cell[thid]->vertices()[0] ) ) continue;

            // Calculate a number of particles in this cell.
            const float averaged_scalar = cell[thid]->averagedScalar();
            float density = this->calculate_density( averaged_scalar );
#ifdef KVS_PLATFORM_MACOSX
            if ( std::isnan( density ) )
#else
	    if ( std::isnan( density ) )
#endif
            {
                density = 0;
            }

            const size_t nparticles = this->calculate_number_of_particles( density, cell[thid]->volume() );
            PBVR_TIMER_END( 291 );
            PBVR_TIMER_STA( 292 );

            const kvs::Real32* S = cell[thid]->scalars();
            kvs::Real32 S_min = S[0];
            kvs::Real32 S_max = S[0];
            for ( size_t i = 1; i < nnodes; i++ )
            {
                S_min = kvs::Math::Min( S_min, S[i] );
                S_max = kvs::Math::Max( S_max, S[i] );
            }
            const float s_min = static_cast<float>( S_min );
            const float s_max = static_cast<float>( S_max );
            float maximum_density = this->calculate_maximum_density( s_min, s_max );
            const float p_max = maximum_density / nparticles;

            // Generate a set of particles in this cell represented by v0,...,v3 and s0,...,s3.
            size_t count = 0;
            while ( count < nparticles )
            {
                const kvs::Vector3f coord = cell[thid]->randomSampling();
                const float scalar = cell[thid]->scalar();
                float density = this->calculate_density( scalar );

                const float p = density / nparticles;
                const float R = Generator::GetRandomNumber();
                if ( p > p_max * R )
                {
                    // Calculate a color.
                    const kvs::RGBColor color( color_map.at( scalar ) );

                    // Calculate a normal.
                    const kvs::Vector3f normal( cell[thid]->gradient() );

                    // using coord synthesizer
                    kvs::Vector3f new_coord = coord;
                    if ( pCrdSynthStr )
                    {
                        X = x = coord.x();
                        Y = y = coord.y();
                        Z = z = coord.z();
                        size_t qn;
                        for ( qn = 0; qn < veclen; qn++ )
                        {
                            cell[thid]->bindCell( index, qn );
                            synth_vars[qn + 8] = cell[thid]->scalar();
                        }
                        if ( ! css.m_xcSynthStr.empty() )
                        {
                            float d = ( float )synth_funcX.eval();
                            TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                            new_coord[0] = d;
                        }
                        if ( ! css.m_ycSynthStr.empty() )
                        {
                            float d = ( float )synth_funcY.eval();
                            TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                            new_coord[1] = d;
                        }
                        if ( ! css.m_zcSynthStr.empty() )
                        {
                            float d = ( float )synth_funcZ.eval();
                            TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                            new_coord[2] = d;
                        }
                    }

                    // set coord, color, and normal to point object( this ).
                    th_vertex_coords.push_back( new_coord.x() );
                    th_vertex_coords.push_back( new_coord.y() );
                    th_vertex_coords.push_back( new_coord.z() );

                    th_vertex_colors.push_back( color.r() );
                    th_vertex_colors.push_back( color.g() );
                    th_vertex_colors.push_back( color.b() );

                    th_vertex_normals.push_back( normal.x() );
                    th_vertex_normals.push_back( normal.y() );
                    th_vertex_normals.push_back( normal.z() );

                    count++;
                }
            } // end of 'paricle' while-loop
            PBVR_TIMER_END( 292 );
        } // end of 'cell' for-loop

        PBVR_TIMER_END( 300 );
        timer.stop();
        std::cout << "[" << thid << "] gen-particles[ms]:" << timer.msec() << std::endl;
        timer.start();
        PBVR_TIMER_STA( 350 );
        PBVR_TIMER_STA( 293 );
        #pragma omp critical
        {
            vertex_coords.insert ( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            vertex_colors.insert ( vertex_colors.end(), th_vertex_colors.begin(), th_vertex_colors.end() );
            vertex_normals.insert( vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end() );
        }
        PBVR_TIMER_END( 293 );
        PBVR_TIMER_END( 350 );
        timer.stop();
        std::cout << "[" << thid << "] merge vertexs[ms]:" << timer.msec() << std::endl;
    }
#else //NEW PARALLEL
    const size_t ncells = volume.ncells();
    kvs::Real32* tf_c_var_array = NULL;
    kvs::Real32* tf_o_var_array = NULL;
    size_t nparticles_all;
    std::string what_except;

    if ( !m_batch )
    {
        tf_c_var_array = ( kvs::Real32* )malloc( sizeof( kvs::Real32 ) * ncells * c_tf_count );
        tf_o_var_array = ( kvs::Real32* )malloc( sizeof( kvs::Real32 ) * ncells * o_tf_count );
    }

    float maximun_synthesized_density = 0;

    FuncParser::Variables* synth_vars = new FuncParser::Variables[max_threads];
    FunctionParser** synth_func_parseX = new FunctionParser*[max_threads];
    FuncParser::Function* synth_funcX = new FuncParser::Function[max_threads];
    FunctionParser** synth_func_parseY = new FunctionParser*[max_threads];
    FuncParser::Function* synth_funcY = new FuncParser::Function[max_threads];
    FunctionParser** synth_func_parseZ = new FunctionParser*[max_threads];
    FuncParser::Function* synth_funcZ = new FuncParser::Function[max_threads];

    FuncParser::Variable Tm; Tm.tag( "T" );
    FuncParser::Variable t;  t.tag( "t" );
    Tm = t = ( float )pCrdSynthStr->m_time_step;
    FuncParser::Variable X; X.tag( "X" );
    FuncParser::Variable x; x.tag( "x" );
    FuncParser::Variable Y; Y.tag( "Y" );
    FuncParser::Variable y; y.tag( "y" );
    FuncParser::Variable Z; Z.tag( "Z" );
    FuncParser::Variable z; z.tag( "z" );
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
    } // end of for(i)

    for ( int n = 0; n < max_threads; n++ )
    {
    synth_vars[n].push_back( Tm );
    synth_vars[n].push_back( t );
    synth_vars[n].push_back( X );
    synth_vars[n].push_back( x );
    synth_vars[n].push_back( Y );
    synth_vars[n].push_back( y );
    synth_vars[n].push_back( Z );
    synth_vars[n].push_back( z );
    for ( size_t i = 0; i < veclen; i++ )
        synth_vars[n].push_back( FuncVars[i] );

    synth_func_parseX[n] = new FunctionParser( css.m_x_coord_synthesizer_string,
                           css.m_x_coord_synthesizer_string.size() + 1 );
    if ( ! css.m_x_coord_synthesizer_string.empty() )
    {
        FunctionParser::Error err = synth_func_parseX[n]->express( synth_funcX[n], synth_vars[n] );
        if ( err != FunctionParser::ERR_NONE )
        {
        std::stringstream ess;
        ess << "Function : " << synth_funcX[n].str() << " error: "
            << FunctionParser::error_type_to_string[err].c_str() << std::endl;
        throw TransferFunctionSynthesizer::NumericException( ess.str() );
        }
    }
    synth_func_parseY[n] = new FunctionParser( css.m_y_coord_synthesizer_string,
                           css.m_y_coord_synthesizer_string.size() + 1 );
    if ( ! css.m_y_coord_synthesizer_string.empty() )
    {
        FunctionParser::Error err = synth_func_parseY[n]->express( synth_funcY[n], synth_vars[n] );
        if ( err != FunctionParser::ERR_NONE )
        {
        std::stringstream ess;
        ess << "Function : " << synth_funcY[n].str() << " error: "
            << FunctionParser::error_type_to_string[err].c_str() << std::endl;
        throw TransferFunctionSynthesizer::NumericException( ess.str() );
        }
    }
    synth_func_parseZ[n] = new FunctionParser( css.m_z_coord_synthesizer_string,
                           css.m_z_coord_synthesizer_string.size() + 1 );
    if ( ! css.m_z_coord_synthesizer_string.empty() )
    {
        FunctionParser::Error err = synth_func_parseZ[n]->express( synth_funcZ[n], synth_vars[n] );
        if ( err != FunctionParser::ERR_NONE )
        {
        std::stringstream ess;
        ess << "Function : " << synth_funcZ[n].str() << " error: "
            << FunctionParser::error_type_to_string[err].c_str() << std::endl;
        throw TransferFunctionSynthesizer::NumericException( ess.str() );
        }
    }
    }

    #pragma omp parallel
    {
        std::vector<kvs::Real32> th_vertex_coords;
        std::vector<kvs::UInt8>  th_vertex_colors;
        std::vector<kvs::Real32> th_vertex_normals;

#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif

        // Calculate Max Density with TransferFunction Synthesize
        const float max_opacity = tfs[thid]->getMaxOpacity();
        const float max_density = tfs[thid]->getMaxDensity();
        const float sampling_volume_inverse = tfs[thid]->getSamplingVolumeInverse();
        bool catch_flag = false;

        if ( m_transfunc_synthesizer )
        {
            cell[thid]->bindCell( 0 );
            #pragma omp for schedule(dynamic)
            for ( long index = 0; index < ncells; index += 1 )
            {
                // add by @hira at 2016/12/01
                if (cell[thid] == NULL) continue;

                bool loop_continuable;
#pragma omp critical(catch_flag)
                loop_continuable = catch_flag;
                if ( loop_continuable ) continue;

                try
                {
                const kvs::Vector3f coord = cell[thid]->vertices()[0];
                    const float opacity = tfs[thid]->calculateAveragedOpacityBatch( cell[thid], index, coord );

                const float density = Generator::CalculateDensity( opacity,
                                                                   sampling_volume_inverse,
                                                                   max_opacity, max_density );
#pragma omp critical
                {
                    if ( density > maximun_synthesized_density )
                    {
                        maximun_synthesized_density = density;
                        }
                    }
                }
                catch ( const TransferFunctionSynthesizer::NumericException& e )
                {
                    #pragma omp critical(catch_flag)
                    {
                        what_except = e.what();
                        catch_flag = true;
                    }
                }
            }
        }

        #pragma omp for schedule(dynamic)
        for ( long index = 0; index < ncells; index++ )
        {
            // add by @hira at 2016/12/01
            if (cell[thid] == NULL) continue;

            bool loop_continuable;
            #pragma omp critical(catch_flag)
            loop_continuable = catch_flag;
            if ( loop_continuable ) continue;

            try
            {
            // Bind the cell which is indicated by 'index'.
            cell[thid]->bindCell( index );
            cell_opacity[thid]->bindCell( index );
            if ( !m_crop.isInner( cell[thid]->vertices()[0] ) )
            {
                continue;
            }


            const kvs::Vector3f coord = cell[thid]->vertices()[0];

            /*****generate histogram******/
            if ( !m_batch )
            {
                    const size_t nnodes = volume.cellType();

                tfs[thid]->multivariateMinMaxValues( cell[thid], index, nnodes );
                    tfs[thid]->calculateAveragedOpacity( cell[thid], index, coord );
                    tfs[thid]->calculateAveragedColor( cell[thid], index, coord );

                for ( int i = 0; i < c_tf_count; i++ )
                {
                    size_t c_index = index + i * ncells;
                    float c_val = tfs[thid]->getValue( name_tf_c_var[ i ] );
                    tf_c_var_array[ c_index ] = c_val;
                }

                for ( int i = 0; i < o_tf_count; i++ )
                {
                    size_t o_index = index + i * ncells;
                    float o_val = tfs[thid]->getValue( name_tf_o_var[ i ] );
                    tf_o_var_array[ o_index ] = o_val;
                }
            }
            /****************************/
            float density;
            if ( m_transfunc_synthesizer )
            {
                const kvs::Vector3f coord = cell[thid]->vertices()[0];
                    const float opacity = tfs[thid]->calculateAveragedOpacityBatch( cell[thid], index, coord );

                density = Generator::CalculateDensity( opacity,
                                                       sampling_volume_inverse,
                                                       max_opacity, max_density );
                tfs[thid]->calculateOpacityOfNode( cell[thid], cell_opacity[thid], index );
            }
            else
            {
                // Calculate a number of particles in this cell.
                const float averaged_scalar = cell[thid]->averagedScalar();
                float density = this->calculate_density( averaged_scalar );
#ifdef KVS_PLATFORM_MACOSX
                if ( std::isnan( density ) )
#else
	        if ( std::isnan( density ) )
#endif
                {
                    density = 0;
                }
            }
            const long nparticles_in_cell = this->calculate_number_of_particles( density, cell[thid]->volume() ) * particle_density;

            float p_max;
            if ( m_transfunc_synthesizer )
            {
                p_max = maximun_synthesized_density / nparticles_in_cell;
            }
            else
            {
                const kvs::Real32* S = cell[thid]->scalars();
                    const size_t nnodes = volume.cellType();
                kvs::Real32 S_min = S[0];
                kvs::Real32 S_max = S[0];
                for ( size_t i = 1; i < nnodes; i++ )
                {
                    S_min = kvs::Math::Min( S_min, S[i] );
                    S_max = kvs::Math::Max( S_max, S[i] );
                }
                const float s_min = static_cast<float>( S_min );
                const float s_max = static_cast<float>( S_max );
                p_max = this->calculate_maximum_density( s_min, s_max ) / nparticles_in_cell;
            }

            for ( long particle_id = 0; particle_id < nparticles_in_cell; particle_id++ )
            {

                // Generate a set of particles in this cell represented by v0,...,v3 and s0,...,s3.
                while ( 1 )
                {

                    float scalar;
                    float density;
                    const kvs::Vector3f coord = cell[thid]->randomSampling();

                    if ( m_transfunc_synthesizer )
                    {
                            const float opacity = tfs[thid]->calculateOpacity( cell[thid], index, coord );

                        density = Generator::CalculateDensity( opacity,
                                                               sampling_volume_inverse,
                                                               max_opacity, max_density );
                    }
                    else
                    {
                        scalar = cell[thid]->scalar();
                        density = this->calculate_density( scalar );
                    }

                    const float p = density / nparticles_in_cell;
                    const float R = Generator::GetRandomNumber();
                    if ( p > p_max * R )
                    {
                        size_t offset = particle_id * 3;
                        // Calculate a color.
                        kvs::RGBColor color;
                        if ( m_transfunc_synthesizer )
                        {
                                color = tfs[thid]->calculateColor( cell[thid], index, coord );
                        }
                        else
                        {
                            color = color_map.at( scalar );
                        }

                        // using coord synthesizer
                        kvs::Vector3f new_coord = coord;
                        if ( pCrdSynthStr )
                        {
                            X = x = coord.x();
                            Y = y = coord.y();
                            Z = z = coord.z();
                            size_t qn;
                            for ( qn = 0; qn < veclen; qn++ )
                            {
                                cell[thid]->bindCell( index, qn );
                                synth_vars[thid][qn + 8] = cell[thid]->scalar();
                            }
                            if ( ! css.m_x_coord_synthesizer_string.empty() )
                            {
                                float d = ( float )synth_funcX[thid].eval();
                                TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                                new_coord[0] = d;
                            }
                            if ( ! css.m_y_coord_synthesizer_string.empty() )
                            {
                                float d = ( float )synth_funcY[thid].eval();
                                TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                                new_coord[1] = d;
                            }
                            if ( ! css.m_z_coord_synthesizer_string.empty() )
                            {
                                float d = ( float )synth_funcZ[thid].eval();
                                TransferFunctionSynthesizer::AssertValid( d, __FILE__, __LINE__ );
                                new_coord[2] = d;
                            }
                        }

                        // Calculate a normal.
                        cell_opacity[thid]->setLocalPoint( cell[thid]->localPoint() );
                        const kvs::Vector3f normal( -cell_opacity[thid]->gradient() );

                        // set coord, color, and normal to point object( this ).
                        th_vertex_coords.push_back( new_coord.x() );
                        th_vertex_coords.push_back( new_coord.y() );
                        th_vertex_coords.push_back( new_coord.z() );

                        th_vertex_colors.push_back( color.r() );
                        th_vertex_colors.push_back( color.g() );
                        th_vertex_colors.push_back( color.b() );

                        th_vertex_normals.push_back( normal.x() );
                        th_vertex_normals.push_back( normal.y() );
                        th_vertex_normals.push_back( normal.z() );

                        break;
                    }
                } // while
            } // end of 'paricle' while-loop
            }
            catch ( const TransferFunctionSynthesizer::NumericException& e )
            {
                #pragma omp critical(catch_flag)
                {
                    what_except = e.what();
                    catch_flag = true;
                }
            }
        } // end of 'cell' for-loop

        #pragma omp critical
        {
            vertex_coords.insert ( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            vertex_colors.insert ( vertex_colors.end(), th_vertex_colors.begin(), th_vertex_colors.end() );
            vertex_normals.insert( vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end() );
        }
    } // omp parallel

    delete [] synth_vars;
    for ( int n = 0; n < max_threads; n++ )
    {
    delete synth_func_parseX[n];
    delete synth_func_parseY[n];
    delete synth_func_parseZ[n];
    }
    delete [] synth_func_parseX;
    delete [] synth_funcX;
    delete [] synth_func_parseY;
    delete [] synth_funcY;
    delete [] synth_func_parseZ;
    delete [] synth_funcZ;

    if ( what_except != "" )
    {
        for ( int n = 0; n < max_threads; n++ )
        {
            if (cell[n] != NULL) delete cell[n];
            if (cell_opacity[n] != NULL) delete cell_opacity[n];
            delete tfs[n];
        }
        delete[] cell;
        delete[] cell_opacity;
        delete[] tfs;

        throw TransferFunctionSynthesizer::NumericException( what_except );
    }

#endif
    /*****generate histogram******/
    if ( !m_batch )
    {
        for ( int i = 0; i < c_tf_count; i++ )
        {
            size_t index = i * ncells;
            kvs::ValueArray<kvs::Real32> c_var_array( &( tf_c_var_array[ index ] ), ncells );

            // modify by @hira at 2016/12/01 : tf削除, m_color_histogram[ tf ]->m_color_histogram[i]
            // tf = atoi( name_tf_c_var[ i ].substr( 1 ).c_str() ) - 1;
            m_color_histogram[ i ].setRange( c_min[ i ], c_max[ i ] );
            m_color_histogram[ i ].setNBins( resolution );
            m_color_histogram[ i ].allocate();
            m_color_histogram[ i ].create( c_var_array );
        }

        for ( int i = 0; i < o_tf_count; i++ )
        {
            size_t index = i * ncells;
            kvs::ValueArray<kvs::Real32> o_var_array( &( tf_o_var_array[ index ] ), ncells );

            // modify by @hira at 2016/12/01 : tf削除, m_opacity_histogram[ tf ]->m_opacity_histogram[i]
            // tf = atoi( name_tf_o_var[ i ].substr( 1 ).c_str() ) - 1;
            m_opacity_histogram[ i ].setRange( o_min[ i ], o_max[ i ] );
            m_opacity_histogram[ i ].setNBins( resolution );
            m_opacity_histogram[ i ].allocate();
            m_opacity_histogram[ i ].create( o_var_array );
        }
    }
    /*****************************/
    PBVR_TIMER_END( 290 );

    SuperClass::m_coords  = kvs::ValueArray<kvs::Real32>( vertex_coords );
    SuperClass::m_colors  = kvs::ValueArray<kvs::UInt8>( vertex_colors );
    SuperClass::m_normals = kvs::ValueArray<kvs::Real32>( vertex_normals );
    if ( tf_c_var_array      != NULL ) free( tf_c_var_array );
    if ( tf_o_var_array      != NULL ) free( tf_o_var_array );

#endif
    SuperClass::setSize( 1.0f );
    PBVR_TIMER_END( 280 );
    double end = GetTime();
    printf( "\nCPU:generate_particles: %lf ms\n", end - start );

    PBVR_TIMER_STA( 360 );
    for ( int n = 0; n < max_threads; n++ )
    {
        m_transfunc_synthesizer->mergeVarRange( *tfs[n] );
        if (cell[n] != NULL) delete cell[n];
        if (cell_opacity[n] != NULL) delete cell_opacity[n];
        delete tfs[n];
    }
    delete[] cell;
    delete[] cell_opacity;
    delete[] tfs;
    PBVR_TIMER_END( 360 );
}

/*===========================================================================*/
/**
 *  @brief  Calculate density value.
 *  @param  scalar [in] scalar value
 *  @return density value
 */
/*===========================================================================*/
const float CellByCellRejectionSampling::calculate_density( const float scalar )
{
    const float min_value = BaseClass::transferFunction().colorMap().minValue();
    const float max_value = BaseClass::transferFunction().colorMap().maxValue();
    const float max_range = static_cast<float>( BaseClass::transferFunction().resolution() - 1 );
    const float normalize_factor = max_range / ( max_value - min_value );
    const float normalized_scalar = ( scalar - min_value ) * normalize_factor;
    size_t index0 = 0;
    if ( normalized_scalar < 0 )
    {
        index0 = 0; // round to 0.
    }
    else
    {
        index0 = static_cast<size_t>( normalized_scalar );
    }
    size_t index1 = index0 + 1;
    index1 = kvs::Math::Clamp<size_t>( index1, 0, BaseClass::transferFunction().resolution() - 1 );
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
 *  @brief  Calculate number of particles.
 *  @param  density [in] density value
 *  @param  volume_of_cell [in] volume of cell
 *  @return number of particles
 */
/*===========================================================================*/
const size_t CellByCellRejectionSampling::calculate_number_of_particles(
    const float density,
    const float volume_of_cell )
{
    const float N = density * volume_of_cell;
    const float R = Generator::GetRandomNumber();

    size_t n = static_cast<size_t>( N );
    if ( N - n > R )
    {
        ++n;
    }

    return n;
}

/*===========================================================================*/
/**
 *  @brief  Calculate maximum dentiy value.
 *  @param  scalar0 [in] scalar value
 *  @param  scalar1 [in] scalar value
 *  @return density value
 */
/*===========================================================================*/
const float CellByCellRejectionSampling::calculate_maximum_density( const float scalar0, const float scalar1 )
{
    if ( scalar0 > scalar1 )
    {
        kvsMessageError( "undefined use of calculate_maximum_density." );
        return 0.0f;
    }
    const float min_value = BaseClass::transferFunction().colorMap().minValue();
    const float max_value = BaseClass::transferFunction().colorMap().maxValue();
    const float max_range = static_cast<float>( BaseClass::transferFunction().resolution() - 1 );
    const float normalize_factor = max_range / ( max_value - min_value );
    const float index0_float = ( scalar0 - min_value ) * normalize_factor;
    size_t index0 = 0;
    if ( index0_float < 0 )
    {
        index0 = 0; // round to 0.
    }
    else
    {
        index0 = static_cast<size_t>( index0_float );
    }
    index0 += 1;
    index0 = kvs::Math::Clamp<size_t>( index0, 0, BaseClass::transferFunction().resolution() - 1 );

    const float index1_float = ( scalar1 - min_value ) * normalize_factor;
    size_t index1 = 0;
    if ( index1_float < 0 )
    {
        index1 = 0; // round to 0.
    }
    else
    {
        index1 = static_cast<size_t>( index1_float );
    }

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

} // end of namespace pbvr
