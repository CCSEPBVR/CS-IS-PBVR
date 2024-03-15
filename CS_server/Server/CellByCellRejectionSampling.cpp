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
//#include <kvs/TrilinearInterpolator>
#include <kvs/Value>
#include "CellBase.h"
#include "TrilinearInterpolator.h" 
#include "TetrahedralCell.h"
#include "QuadraticTetrahedralCell.h"
#include "HexahedralCell.h"
#include "QuadraticHexahedralCell.h"
#include "PrismaticCell.h"
#include "PyramidalCell.h"
#include "GlobalCore.h"

#include <kvs/Timer>
#include "common.h"

#include <cmath>
//#include <mpi.h>
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
    m_transfer_function_synthesizer( transfunc_synthesizer ),
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
    m_transfer_function_synthesizer( transfunc_synthesizer ),
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
    std::vector<pbvr::TransferFunction>& transfer_function_array, 
    //std::vector<NamedTransferFunction>& transfer_function_array, 
    TransferFunctionSynthesizer* transfunc_synthesizer,
    const CropRegion&            crop,
    const float                  particle_density,
    const bool                   batch,
    const float                  object_depth ):
    pbvr::MapperBase( transfer_function ),
    pbvr::PointObject(),
    m_transfer_function_array( transfer_function_array ),
    m_transfer_function_synthesizer( transfunc_synthesizer ),
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
//        const kvs::Camera* camera = ( !m_camera ) ? pbvr::GlobalCore::camera : m_camera;
//        this->mapping( camera, reinterpret_cast<const pbvr::StructuredVolumeObject*>( object ) );
        const pbvr::StructuredVolumeObject* svo_p = static_cast<const pbvr::StructuredVolumeObject*>( &object );
        if ( m_camera )
        {
//            this->mapping( m_camera, reinterpret_cast<const pbvr::StructuredVolumeObject*>( object ) );
            this->mapping( *m_camera, *svo_p );
        }
        else
        {
            // Generate particles by using default camera parameters.
            if ( pbvr::GlobalCore::m_camera )
            {
                if ( pbvr::GlobalCore::m_camera->windowWidth() != 0 && pbvr::GlobalCore::m_camera->windowHeight() )
                {
                    const kvs::Camera* camera = pbvr::GlobalCore::m_camera;
//                    this->mapping( camera, reinterpret_cast<const pbvr::StructuredVolumeObject*>( object ) );
                      this->mapping( *camera, *svo_p );
                }
            }
            else
            {
                kvs::Camera* camera = new kvs::Camera();
//                this->mapping( camera, reinterpret_cast<const pbvr::StructuredVolumeObject*>( object ) );
                this->mapping( *camera, *svo_p );
                delete camera;
            }
        }
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

/*===========================================================================*/
/**
 *  @brief  Mapping for the structured volume object.
 *  @param  camera [in] pointer to the camera
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
void CellByCellRejectionSampling::mapping( const kvs::Camera& camera, const pbvr::StructuredVolumeObject& volume )
{
    // Attach the pointer to the volume object and set the min/max coordinates.
    BaseClass::attach_volume( volume );
    BaseClass::set_range( volume );
    BaseClass::set_min_max_coords( volume, this );

    const pbvr::VolumeObjectBase *object = BaseClass::volume();
    // Calculate the density map.
    // if ( m_transfunc_synthesizer )
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
        m_density_map = Generator::CalculateDensityMap(
                            camera,
                            *object,
                            static_cast<float>( m_subpixel_level ),
                            m_sampling_step,
                            BaseClass::transferFunction().opacityMap() );
    }

//   m_density_map = Generator::CalculateDensityMap(
//                        camera,
////                        BaseClass::volume(),
//                        *object,
//                        static_cast<float>( m_subpixel_level ),
//                        m_sampling_step,
//                        BaseClass::transferFunction().opacityMap() );

    // Generate the particles.
    const std::type_info& type = volume.values().typeInfo()->type();
    if (      type == typeid( kvs::UInt8  ) ) this->generate_particles<kvs::UInt8>( volume );
    else if ( type == typeid( kvs::UInt16 ) ) this->generate_particles<kvs::UInt16>( volume );
    else if ( type == typeid( kvs::Int16  ) ) this->generate_particles<kvs::Int16>( volume );
    else if ( type == typeid( kvs::Real32 ) ) this->generate_particles<kvs::Real32>( volume );
    else if ( type == typeid( kvs::Real64 ) ) this->generate_particles<kvs::Real64>( volume );
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Unsupported data type '%s'.", volume.values().typeInfo()->typeName() );
    }
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
    //else if ( type == typeid( kvs::Real64 ) ) this->generate_particles<kvs::Real32>( volume );
    else
    {
        BaseClass::m_is_success = false;
        kvsMessageError( "Unsupported data type '%s'.", volume.values().typeInfo()->typeName() );
    }
}

/*===========================================================================*/
/**
 *  @brief  Generates particles for the structured volume object.
 *  @param  volume [in] pointer to the input volume object
 */
/*===========================================================================*/
template <typename T>
void CellByCellRejectionSampling::generate_particles( const pbvr::StructuredVolumeObject& volume )
{
#if 1

    kvs::AnyValueArray valueArray = volume.values(); 
    int nnodes = volume.nnodes();
    
    const kvs::Vector3ui resolution( volume.resolution() );
    const int nvariables = volume.veclen();
    Type** values;
    values = new Type * [nvariables];

    for ( int j = 0; j < nvariables; j++ )
    {
        values[j] = new float[nnodes];
        for ( int i = 0; i < nnodes; i++ )
        {
            int  it = j * nnodes  + i;
            values[j][i] = (float)(valueArray.at<double>(it));  
            // データ値は double型で出力されている。これをfloat型に変更する必要がある。float型で読み込もうとすると、値がバグる。
        }
    } 

    int tf_number = m_transfer_function_array.size();
    float sampling_volume_inverse = m_transfer_function_synthesizer -> getSamplingVolumeInverse()  ;
    float max_opacity = m_transfer_function_synthesizer -> getMaxOpacity();
    float max_density = m_transfer_function_synthesizer -> getMaxDensity();

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

    std::vector< std::vector< TFS::TrilinearInterpolator* > >  interp;
    interp.resize( max_threads );
    for ( int i = 0; i < max_threads; i++ )
    {
        interp[ i ].resize( nvariables );
        for ( int j = 0; j < nvariables; j++ )
        {
             interp[i][j]  = new TFS::TrilinearInterpolator( values[j], resolution);
        }
    }

    static bool parameter_file_opened=true;
    const int max_nparticles = (int)m_transfer_function_synthesizer->getMaxDensity() + 1;
    if(mpi_rank==0) std::cout<<"******* max_nparticles="<<max_nparticles<<std::endl;

    //ヒストグラム
    int nbins = 256;
    kvs::ValueArray<float> o_min( tf_number );//TFSから読み込む最大最小値
    kvs::ValueArray<float> o_max( tf_number );
    kvs::ValueArray<float> c_min( tf_number );
    kvs::ValueArray<float> c_max( tf_number );

    kvs::ValueArray<int> o_histogram( tf_number * nbins );//不透明度ヒストグラムの配列
    kvs::ValueArray<int> c_histogram( tf_number * nbins );//色ヒストグラムの配列

    if( parameter_file_opened )
    {
        o_histogram.fill(0x00);
        c_histogram.fill(0x00);
    }

    for( size_t i = 0; i < tf_number; i++ )
    {
        o_min[i] = m_transfer_function_array[i].opacityMap().minValue();
        o_max[i] = m_transfer_function_array[i].opacityMap().maxValue();
        c_min[i] = m_transfer_function_array[i].colorMap().minValue();
        c_max[i] = m_transfer_function_array[i].colorMap().maxValue();
    }

    //最大最小値
    kvs::ValueArray<float> O_min( tf_number );//計算して得る最大最小値
    kvs::ValueArray<float> O_max( tf_number );
    kvs::ValueArray<float> C_min( tf_number );
    kvs::ValueArray<float> C_max( tf_number );

    // 動的な粒子データ配列
    std::vector<float> vertex_coords;
    std::vector<Byte>  vertex_colors;
    std::vector<float> vertex_normals;

    if( parameter_file_opened )
    {
        for ( size_t i = 0; i < tf_number; i++ ) //初期化
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

    TFS::TrilinearInterpolator** interp_opacity  = new TFS::TrilinearInterpolator*[max_threads] ;

    th_tf.resize( max_threads );
    for ( int i = 0; i < max_threads; i++ )
    {
        th_tf[ i ].resize( tf_number );
        for ( int j = 0; j < tf_number; j++ )
        {
            th_tf[i][j] = m_transfer_function_array[j];
        }
    }

    float* opacity_volume = new float[ resolution.x()*resolution.y()*resolution.z() ];

//    time_parameters time;
//    timer.stop();
//    time.initialize = timer.sec();
//    timer.start();

    //頂点解像度と格子解像度
    const int nx = resolution.x();
    const int ny = resolution.y();
    const int nz = resolution.z();
    const int nxy = nx * ny;
    const int nx_1 = nx-1;
    const int ny_1 = ny-1;
    const int nz_1 = nz-1;
    const int nxy_1 = nx_1 * ny_1;

    int total_nparticles = 0;

//    static TimedScope td_gatherf("GatherF",1);
//    static TimedScope td_gather("gather",1);
//    static TimedScope td_kvsml("kvsml",1);
//    static TimedScope td_SynthOpacityScalars("SynthesizedOpacityScalars",max_threads);
//    static TimedScope td_SynthColorScalars("SynthesizedColorScalars",max_threads);
//    static TimedScope td_CalculateHistogram("CalculateHistogram",max_threads);
//    static TimedScope td_CalculateOpacity("CalculateOpacity",max_threads);
//    static TimedScope td_CalculateDensity("CalculateDensity",max_threads);
//    static TimedScope td_CalculateNumPar("Ccalculate_number_of_particles (Random)",max_threads);
//    static TimedScope td_CalculateColor("CalculateColor",max_threads);
//    static  TimedScope td_VectorPush("Vector Push",max_threads);
//    static TimedScope td_VectorIns("Vector Insert",max_threads);
    #pragma omp parallel
    {
#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif

        //th_tfs[thid]->set_debug_thid(thid,max_threads);

        int th_total_nparticles = 0;
        //各スレッド番号をシードにした乱数生成器
        kvs::MersenneTwister MT( thid + mpi_rank * nthreads );

        // 動的な粒子データ配列
        std::vector<float> th_vertex_coords;
        std::vector<Byte>  th_vertex_colors;
        std::vector<float> th_vertex_normals;

        //ヒストグラムの配列
        float o_scalars[tf_number][SIMDW];//頂点の不透明度
        float c_scalars[tf_number][SIMDW];//頂点の色

        kvs::ValueArray<int> th_o_histogram( tf_number * nbins );//不透明度
        kvs::ValueArray<int> th_c_histogram( tf_number * nbins );//色

        if( parameter_file_opened )
        {
            th_o_histogram.fill(0x00);
            th_c_histogram.fill(0x00);
        }

        //最大最小値
        kvs::ValueArray<float> th_O_min( tf_number );//計算して得る最大最小値
        kvs::ValueArray<float> th_O_max( tf_number );
        kvs::ValueArray<float> th_C_min( tf_number );
        kvs::ValueArray<float> th_C_max( tf_number );

        if( parameter_file_opened )
        {
            for ( int i = 0; i < tf_number; i++ ) //初期化
            {
                th_O_min[ i ] =  FLT_MAX;
                th_O_max[ i ] = -FLT_MAX;
                th_C_min[ i ] =  FLT_MAX;
                th_C_max[ i ] = -FLT_MAX;
            }
        }

        const kvs::Vector3f min_vec = volume.minObjectCoord(); 
        const kvs::Vector3f max_vec = volume.maxObjectCoord(); 
        const kvs::Vector3f cell_length( (max_vec.x() - min_vec.x() )/ nx_1,
                                         (max_vec.y() - min_vec.y() )/ ny_1,
                                         (max_vec.z() - min_vec.z() )/ nz_1) ;
        //-----------------------------------------//
        //----------------Histogram----------------//
        //-----------------------------------------//
        {
            // Marge x-y-z loop
            const int nvertices = nx * ny * nz;
            // "+ 1" means remained loop
            const int outer_loop = (nvertices % SIMDW == 0) ?
                nvertices / SIMDW : nvertices / SIMDW + 1;

            #pragma omp for
            for( int J=0; J<outer_loop; J++ )
            {
                float X_l[SIMDW], Y_l[SIMDW], Z_l[SIMDW];
                float X_g[SIMDW], Y_g[SIMDW], Z_g[SIMDW];
                #pragma ivdep
                for( int I=0; I<SIMDW; I++ )
                {
                    const int vertex_id = I + J * SIMDW;
                    // vertex_id = i + j * nx + k * nx * ny
                    const int k =  vertex_id / nxy;
                    const int j = (vertex_id - k * nxy) / nx;
                    const int i =  vertex_id - k * nxy - j * nx;

                    const float x_l = (float)i;
                    const float y_l = (float)j;
                    const float z_l = (float)k;
                    const float x_g = (x_l * cell_length.x())+min_vec.x();
                    const float y_g = (y_l * cell_length.y())+min_vec.y();
                    const float z_g = (z_l * cell_length.z())+min_vec.z();

                    X_l[I] = x_l;
                    Y_l[I] = y_l;
                    Z_l[I] = z_l;
                    X_g[I] = x_g;
                    Y_g[I] = y_g;
                    Z_g[I] = z_g;
                }
//                timed_section_start(td_SynthOpacityScalars,thid);
                th_tfs[thid]->SynthesizedOpacityScalars(
                    interp[thid], X_l, Y_l, Z_l, X_g, Y_g, Z_g, o_scalars );
//                timed_section_end(td_SynthOpacityScalars,thid);
//                timed_section_start(td_SynthColorScalars,thid);
                th_tfs[thid]->SynthesizedColorScalars(
                    interp[thid], X_l, Y_l, Z_l, X_g, Y_g, Z_g, c_scalars );
//                timed_section_end(td_SynthColorScalars,thid);
//                timed_section_start(td_CalculateHistogram,thid);
                calculate_histogram( th_o_histogram, th_c_histogram,
                                     th_O_min, th_O_max, th_C_min, th_C_max,
                                     nbins,
                                     o_min, o_max, c_min, c_max,
                                     o_scalars, c_scalars,
                                     tf_number );
//                timed_section_end(td_CalculateHistogram,thid);
            }
        } // end of Histogram

        //-----------------------------------------//
        //--------------粒子生成ループ開始------------//
        //------------------------------------------//
        //不透明度ボリュームの生成
        {
            // Marge x-y-z loop
            const int nvertices = nx * ny * nz;
            // "+ 1" means remained loop
            const int outer_loop = (nvertices % SIMDW == 0) ?
                nvertices / SIMDW : nvertices / SIMDW + 1;

            #pragma omp for
            for( int J=0; J<outer_loop; J++ )
            {
                float X_l[SIMDW], Y_l[SIMDW], Z_l[SIMDW];//interp用の相対座標
                float X_g[SIMDW], Y_g[SIMDW], Z_g[SIMDW];//TFS用の全体座標
                #pragma ivdep
                for( int I=0; I<SIMDW; I++ )
                {
                    const int vertex_id = I + J * SIMDW;
                    const int k =  vertex_id / nxy;
                    const int j = (vertex_id - k * nxy) / nx;
                    const int i =  vertex_id - k * nxy - j * nx;

                    const float x_l = (float)i;
                    const float y_l = (float)j;
                    const float z_l = (float)k;
                    const float x_g = (x_l * cell_length.x())+min_vec.x();
                    const float y_g = (y_l * cell_length.y())+min_vec.y();
                    const float z_g = (z_l * cell_length.z())+min_vec.z();

                    X_l[I] = x_l;
                    Y_l[I] = y_l;
                    Z_l[I] = z_l;
                    X_g[I] = x_g;
                    Y_g[I] = y_g;
                    Z_g[I] = z_g;
                }

                float vertex_opacity[SIMDW];

//                timed_section_start(td_CalculateOpacity,thid);
                th_tfs[thid]->CalculateOpacity( interp[thid], th_tf[thid],
                                                X_l, Y_l, Z_l, X_g, Y_g, Z_g, vertex_opacity );
//                timed_section_end(td_CalculateOpacity,thid);

#pragma ivdep
                for( int I = 0; I <SIMDW; I++ )
                {
                    const int vertex_id = I + J * SIMDW;
                    if( vertex_id < nvertices )
                    {
                        //ov[ vertex_id ] = vertex_opacity[I];
                        opacity_volume[ vertex_id ] = vertex_opacity[I];
                    }
                }
            }
        }// end of 不透明度ボリュームの生成

#pragma omp critical
        {
            interp_opacity[thid] = new TFS::TrilinearInterpolator( opacity_volume, resolution );
        }

        //粒子生成ループ開始
        {
            // Marge x-y-z loop
            const int ncells = nx_1 * ny_1 * nz_1;
            // "+ 1" means remained loop
            const int outer_loop = (ncells % SIMDW == 0) ?
                ncells / SIMDW : ncells / SIMDW + 1;

            #pragma omp for schedule(dynamic)
            for( int J=0; J<outer_loop; J++ )
            {
                float X_l[SIMDW], Y_l[SIMDW], Z_l[SIMDW];//interp用の相対座標
                float X_g[SIMDW], Y_g[SIMDW], Z_g[SIMDW];//TFS用の全体座標
                #pragma ivdep
                for( int I=0; I<SIMDW; I++ )
                {
                    const int cell_id = I + J * SIMDW;
                    const int k =  cell_id / nxy_1;
                    const int j = (cell_id - k * nxy_1) / nx_1;
                    const int i =  cell_id - k * nxy_1 - j * nx_1;

                    const float x_l = (float)i + 0.5;
                    const float y_l = (float)j + 0.5;
                    const float z_l = (float)k + 0.5;
                    const float x_g = (x_l * cell_length.x())+min_vec.x();
                    const float y_g = (y_l * cell_length.y())+min_vec.y();
                    const float z_g = (z_l * cell_length.z())+min_vec.z();

                    X_l[I] =  x_l;
                    Y_l[I] =  y_l;
                    Z_l[I] =  z_l;
                    X_g[I] =  x_g;
                    Y_g[I] =  y_g;
                    Z_g[I] =  z_g;
                }

                float cell_opacity[SIMDW];
//                timed_section_start(td_CalculateOpacity,thid);
                th_tfs[thid]->CalculateOpacity( interp[thid], th_tf[thid],
                                                X_l, Y_l, Z_l, X_g, Y_g, Z_g, cell_opacity );
//                timed_section_end(td_CalculateOpacity,thid);

                //int nparticles[SIMDW];
                int nparticles[SIMDW+1];
                #pragma ivdep
                for( int I=0; I<SIMDW; I++)
                {
//                     timed_section_start(td_CalculateDensity,thid);
                    const float density = Generator::CalculateDensity( cell_opacity[I],
                                                                       sampling_volume_inverse,
                                                                       max_opacity, max_density );


//                    timed_section_end(td_CalculateDensity,thid);
//                    timed_section_start(td_CalculateNumPar,thid);

                    const int np = calculate_number_of_particles( density, 1, &MT ) * m_particle_density;
//                    const int np = calculate_number_of_particles( density, 1, &MT ) * 0.01;
//                    timed_section_end(td_CalculateNumPar,thid);

                    const int cell_id = I + J * SIMDW;

                    nparticles[I] = cell_id < ncells ? np : 0;
//                    nparticles[I] = cell_id < ncells ? 10 : 0;
                    th_total_nparticles += nparticles[I];
//                    std::cout << "th_total_nparticles =" << th_total_nparticles <<std::endl;
                }

                // 乱数生成はSIMD化できない
                // 粒子位置を逐次計算
                int p_id = 0;
                float p_x_l[SIMDW], p_y_l[SIMDW], p_z_l[SIMDW];
                float p_x_g[SIMDW], p_y_g[SIMDW], p_z_g[SIMDW];
                float grad_x[SIMDW], grad_y[SIMDW], grad_z[SIMDW];
                kvs::UInt8 red[SIMDW], green[SIMDW], blue[SIMDW];
                float particle_opacity[SIMDW];
                // the last loop "I==SIMDW" is used for occupy ramained array.
                    for(int I=0; I<SIMDW+1; I++)
                    {
                        const int cell_id = I + J * SIMDW;
                        const int k =  cell_id / nxy_1;
                        const int j = (cell_id - k * nxy_1) / nx_1;
                        const int i =  cell_id - k * nxy_1 - j * nx_1;

                        const int nparticles_I  = I<SIMDW ? nparticles[I] : 0;
//                        const int nparticles_I  = I<SIMDW ? nparticles[I] : SIMDW - pp_id;
                        const int zero_id = I<SIMDW ? SIMDW : p_id;
                        int nparticles_count =0;  
                        while (nparticles_count < nparticles_I)
                        {
                            const kvs::Vector3f vertex( (float)i, (float)j, (float)k );
                            const kvs::Vector3f coord_l( RandomSamplingInCube( vertex, &MT ) );
                            const kvs::Vector3f coord_g(
                                    (coord_l.x()*cell_length.x())+min_vec.x(),
                                    (coord_l.y()*cell_length.y())+min_vec.y(),
                                    (coord_l.z()*cell_length.z())+min_vec.z() );
                            p_x_l[ p_id ] = coord_l.x();
                            p_y_l[ p_id ] = coord_l.y();
                            p_z_l[ p_id ] = coord_l.z();
                            p_x_g[ p_id ] = coord_g.x();
                            p_y_g[ p_id ] = coord_g.y();
                            p_z_g[ p_id ] = coord_g.z();
                            p_id++;

                            if( p_id == SIMDW )
                            {
                                p_id = 0;

                                //                            timed_section_start(td_CalculateOpacity,thid);
                                th_tfs[thid]->CalculateOpacity( interp[thid], th_tf[thid],
                                        p_x_l, p_y_l, p_z_l,
                                        p_x_g, p_y_g, p_z_g,
                                        particle_opacity );
                                //                            timed_section_end(td_CalculateOpacity,thid); 
                                interp_opacity[thid]->attachPoint( p_x_l, p_y_l, p_z_l );
                                interp_opacity[thid]->gradient( grad_x, grad_y, grad_z );
                                //                            timed_section_start(td_CalculateColor,thid);
                                th_tfs[thid]->CalculateColor( interp[thid], th_tf[thid],
                                        p_x_l, p_y_l, p_z_l,
                                        p_x_g, p_y_g, p_z_g,
                                        red, green, blue );
                                //                            timed_section_end(td_CalculateColor,thid); 
                                //SIMDループ
                                //for( int pp=0; pp<SIMDW; pp++)
                                for( int pp=0; pp<zero_id; pp++)
                                {
                                    //                                timed_section_start(td_CalculateDensity,thid);
                                    const float density =
                                        Generator::CalculateDensity( particle_opacity[pp],
                                                sampling_volume_inverse,
                                                max_opacity, max_density );


                                    //                                timed_section_end(td_CalculateDensity,thid);
                                const float random = (float)MT();
                                if( density > max_density * random )
                                {
                                        th_vertex_coords.push_back( p_x_g[pp] );
                                        th_vertex_coords.push_back( p_y_g[pp] );
                                        th_vertex_coords.push_back( p_z_g[pp] );

                                        th_vertex_colors.push_back( red  [pp] );
                                        th_vertex_colors.push_back( green[pp] );
                                        th_vertex_colors.push_back( blue [pp] );

                                        th_vertex_normals.push_back( grad_x[pp] );
                                        th_vertex_normals.push_back( grad_y[pp] );
                                        th_vertex_normals.push_back( grad_z[pp] );
                                        nparticles_count ++;
                                       //                                    timed_section_end(td_VectorPush,thid);
                                } // end of if pp
                                } // end of for pp
                            } // end of if p_id
                        } // end of while loop 
                    } // end of for I 粒子位置を逐次計算
                } // end of omp for J outer_loop
        } // end of 粒子生成ループ

//        timed_section_start(td_VectorIns,thid);
        #pragma omp critical
        {
            if( parameter_file_opened )
            {
                //最大最小値
                for( int i = 0; i < tf_number; i++ )
                {
                    //不透明度
                    O_min[i] = O_min[i] < th_O_min[i] ? O_min[i] : th_O_min[i];
                    O_max[i] = O_max[i] > th_O_max[i] ? O_max[i] : th_O_max[i];
                    //色
                    C_min[i] = C_min[i] < th_C_min[i] ? C_min[i] : th_C_min[i];
                    C_max[i] = C_max[i] > th_C_max[i] ? C_max[i] : th_C_max[i];

                }

                for( int n = 0; n < tf_number * nbins; n++ )
                {
                    o_histogram[n] += th_o_histogram[n];
                    c_histogram[n] += th_c_histogram[n];
                }
            }

            total_nparticles += th_total_nparticles;
            vertex_coords.insert ( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            vertex_colors.insert ( vertex_colors.end(), th_vertex_colors.begin(), th_vertex_colors.end() );
            vertex_normals.insert( vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end() );

            delete interp_opacity[thid];

        } // end of omp critical
//        timed_section_end(td_VectorIns,thid);
    } // end of omp parallel

    //if(mpi->rank == 0) std::cout<<"total_nparticles="<<total_nparticles<<std::endl;
    std::cout<<"rank="<<mpi_rank<<",total_nparticles="<<total_nparticles<<std::endl;

    // add by shimomura 
    // set variable range
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

    SuperClass::m_coords  = kvs::ValueArray<kvs::Real32>( vertex_coords );
    SuperClass::m_colors  = kvs::ValueArray<kvs::UInt8>( vertex_colors );
    SuperClass::m_normals = kvs::ValueArray<kvs::Real32>( vertex_normals );

//    timer.stop();
//    time.sampling = timer.sec();
//    time.nparticles = vertex_coords.size()/3;
//    timer.start();

    for(int i=0; i<max_threads; i++)
    {
        delete th_tfs[i];
    }
    delete[] th_tfs;

//    delete tfs;

    delete[] opacity_volume;

    for ( int i = 0; i < max_threads; i++ )
    {
        for ( int j = 0; j < nvariables; j++ )
        {
             delete interp[i][j];
        }
    }
    delete[] interp_opacity;


#endif
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
//   case pbvr::VolumeObjectBase::Tetrahedra:
//   {
//       cell = new pbvr::TetrahedralCell<T>( volume );
//       break;
//   }
//   case pbvr::VolumeObjectBase::QuadraticTetrahedra:
//   {
//       cell = new pbvr::QuadraticTetrahedralCell<T>( volume );
//       break;
//   }
    case pbvr::VolumeObjectBase::Hexahedra:
    {
        cell = new pbvr::HexahedralCell<T>( volume );
        break;
    }
//    case pbvr::VolumeObjectBase::QuadraticHexahedra:
//    {
//        cell = new pbvr::QuadraticHexahedralCell<T>( volume );
//        break;
//    }
//    case pbvr::VolumeObjectBase::Prism:
//    {
//        cell = new pbvr::PrismaticCell<T>( volume );
//        break;
//    }
//    case pbvr::VolumeObjectBase::Pyramid:
//    {
//        cell = new pbvr::PyramidalCell<T>( volume );
//        break;
//    }
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
    double start = GetTime();
    size_t resolution = DEFAULT_NBINS;
    kvs::ValueArray<float> O_min_recv;
    kvs::ValueArray<float> O_max_recv;
    kvs::ValueArray<float> C_min_recv;
    kvs::ValueArray<float> C_max_recv;
    kvs::ValueArray<int> o_histogram_recv;
    kvs::ValueArray<int> c_histogram_recv;


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

    //ヒストグラム
    int nbins = 256;
    kvs::ValueArray<float> o_min( tf_number );//TFSから読み込む最大最小値
    kvs::ValueArray<float> o_max( tf_number );
    kvs::ValueArray<float> c_min( tf_number );
    kvs::ValueArray<float> c_max( tf_number );

    // 2023/07/31 add by shimomura
    SuperClass::m_c_histogram = kvs::ValueArray<int> (tf_number * nbins);
    SuperClass::m_o_histogram = kvs::ValueArray<int> (tf_number * nbins);
    SuperClass::setTfnumber(tf_number);
    SuperClass::setNbins(nbins);
    
    m_o_histogram.fill(0x00);
    m_c_histogram.fill(0x00);
    if( parameter_file_opened )
    {
        O_min_recv.allocate(tf_number);
        O_max_recv.allocate(tf_number);
        C_min_recv.allocate(tf_number);
        C_max_recv.allocate(tf_number);
        o_histogram_recv.allocate(tf_number * nbins);
        c_histogram_recv.allocate(tf_number * nbins);
    }

    for( size_t i = 0; i < tf_number; i++ )
    {
        o_min[i] = m_transfer_function_array[i].opacityMap().minValue();
        o_max[i] = m_transfer_function_array[i].opacityMap().maxValue();
        c_min[i] = m_transfer_function_array[i].colorMap().minValue();
        c_max[i] = m_transfer_function_array[i].colorMap().maxValue();
    }

    //最大最小値
    kvs::ValueArray<float> O_min( tf_number );//計算して得る最大最小値
    kvs::ValueArray<float> O_max( tf_number );
    kvs::ValueArray<float> C_min( tf_number );
    kvs::ValueArray<float> C_max( tf_number );

    // 動的な粒子データ配列
    std::vector<float> vertex_coords;
    std::vector<Byte>  vertex_colors;
    std::vector<float> vertex_normals;

    if( parameter_file_opened )
    {
        for ( size_t i = 0; i < tf_number; i++ ) //初期化
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

        // 動的な粒子データ配列
        std::vector<float> th_vertex_coords;
        std::vector<Byte>  th_vertex_colors;
        std::vector<float> th_vertex_normals;

        //ヒストグラムの配列
        std::vector<float> o_scalars( tf_number );//頂点の不透明度
        std::vector<float> c_scalars( tf_number );//頂点の色
        kvs::ValueArray<int> th_o_histogram( tf_number * nbins );//不透明度
        kvs::ValueArray<int> th_c_histogram( tf_number * nbins );//色

        if( parameter_file_opened )
        {
            th_o_histogram.fill(0x00);
            th_c_histogram.fill(0x00);
        }

        //最大最小値
        kvs::ValueArray<float> th_O_min( tf_number );//計算して得る最大最小値
        kvs::ValueArray<float> th_O_max( tf_number );
        kvs::ValueArray<float> th_C_min( tf_number );
        kvs::ValueArray<float> th_C_max( tf_number );

        if( parameter_file_opened )
        {
            for ( int i = 0; i < tf_number; i++ ) //初期化
            {
                th_O_min[ i ] =  FLT_MAX;
                th_O_max[ i ] = -FLT_MAX;
                th_C_min[ i ] =  FLT_MAX;
                th_C_max[ i ] = -FLT_MAX;
            }
        }

        // -----------------------------------
        //配列の追加
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

        //粒子生成ループ開始
#pragma omp for schedule( dynamic ) nowait  
        //#pragma omp for schedule( static ) nowait
        //#pragma omp for schedule( static, 1 ) nowait
        for( int cell_base = 0; cell_base < ncells; cell_base += SIMDW )
        {
            //ブロック内でのループ回数を取得
            int remain = ( ncells - cell_base > SIMDW )? SIMDW: ncells - cell_base;

            /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            //一括でセルをバインドするための配列と、座標の取得
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (kvs::UInt32)(cell_base + cell_BLK);
                local_center_array[cell_BLK] = kvs::Vector3f ( 0.5, 0.5, 0.5 );
            }

            //補間器にセルを一括でバインド
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
                        // 20190128 修正
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

            //生成粒子数を計算
            int nparticles_num = 0;
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                    const float density = Generator::CalculateDensity( cell_opacity_array[cell_BLK],
                            sampling_volume_inverse,
                            max_opacity, max_density );
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
                    //ブロック内でのループ回数を取得
                    int remain_BLK = ( nparticles_array[cell_BLK] - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: nparticles_array[cell_BLK] - i;

                    //一括でセルをバインドするための配列と、座標の取得
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = cell_base + cell_BLK;
                        while(1)
                        {
                            local_coord_array[j] = interp[thid][0] -> randomSampling_MT( &MT);

                            //補間器にセルを一括でバインド
                            for( int k = 0; k < nvariables; k++ )
                            {
                                interp[thid][k]->bindCell( cell_index[j] );
                            }

                            interp[thid][0]->setLocalPoint( local_coord_array[j] );
                            global_coord_array[j] = interp[thid][0]->transformLocalToGlobal( local_coord_array[j] );

                            cell_opacity_array[j] = th_tfs[thid]->CalculateOpacity( interp[thid],
                                    local_coord_array[j],
                                    global_coord_array[j],
                                    th_tf[thid]);

                            density_array[j] = Generator::CalculateDensity( cell_opacity_array[j],
                                    sampling_volume_inverse,
                                    max_opacity, max_density );
                            if(density_array[j] > 0)
                            {
//                            std::cout << "break!!!!!" <<std::endl; 
                                break;
                            }
                        }
                    }

                    //densityの条件に適合するnparticlesの個数の取得
                    //そのときのcell_index, local_coordを再配置
                    int nparticles_count = 0;
                    for( int j = 0; j < remain_BLK; j++ )
                    {
                        while(1)
                        {
                            if( density_array[j] > max_density * (float)MT.rand() )
                            {
                                cell_index[ nparticles_count ] = cell_index[j];
                                local_coord_array[ nparticles_count ] = local_coord_array[j];
                                global_coord_array[ nparticles_count ] = global_coord_array[j];
                                nparticles_count +=1;
                                break;
                            }
                        }
                    }

                    // ------------------------------------------------

                    //補間器にセルを一括でバインド
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
                    //grad_arrayの算出
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        //JacobiMatrixでメンバ変数を使用しているので再度バインド
                        interp[thid][0]->bindCell( cell_index[j] );

                        const kvs::Vector3f g( -dsdx_array[j], -dsdy_array[j], -dsdz_array[j] );
                        const kvs::Matrix33f J = interp[thid][0]->JacobiMatrix();
                        float determinant = 0.0f;
                        const kvs::Vector3f G = J.inverse( &determinant ) * g;
                        grad_array[j] = kvs::Math::IsZero( determinant ) ? kvs::Vector3f( 0.0f, 0.0f, 0.0f ) : G;
                    }

                    //色の計算
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

//        if (thid == 0 )std::cout << __FILE__ << ":" << __LINE__ <<  ":" << __func__ << std::endl;
#pragma omp critical
        {
            if( parameter_file_opened )
            {
                //最大最小値
                for( int i = 0; i < tf_number; i++ )
                {
                    //不透明度
                    O_min[i] = O_min[i] < th_O_min[i] ? O_min[i] : th_O_min[i];
                    O_max[i] = O_max[i] > th_O_max[i] ? O_max[i] : th_O_max[i];
                    //色
                    C_min[i] = C_min[i] < th_C_min[i] ? C_min[i] : th_C_min[i];
                    C_max[i] = C_max[i] > th_C_max[i] ? C_max[i] : th_C_max[i];

                }

            }   

            for( int n = 0; n < tf_number * nbins; n++ )
            {
                m_o_histogram[n] += th_o_histogram[n];
                m_c_histogram[n] += th_c_histogram[n];
            }


            vertex_coords.insert ( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            vertex_colors.insert ( vertex_colors.end(), th_vertex_colors.begin(), th_vertex_colors.end() );
            vertex_normals.insert( vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end() );
        }

    } //#pragma omp parallel

        O_min_recv.fill(0x00);
        O_max_recv.fill(0x00);
        C_min_recv.fill(0x00);
        C_max_recv.fill(0x00);

        //ヒストグラムの集計
        o_histogram_recv.fill(0x00);
        c_histogram_recv.fill(0x00);

    // add by shimomura 
    // set variable range
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


    //delete tfs;  // メンバ変数にしたが、リセットする必要はあるか？

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
    printf( "\nCPU:generate_particles: %lf ms\n", end - start );

}

const size_t CellByCellRejectionSampling::calculate_number_of_particles(
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

void CellByCellRejectionSampling::calculate_histogram( kvs::ValueArray<int>&   th_o_histogram,
                          kvs::ValueArray<int>&   th_c_histogram,
                          kvs::ValueArray<float>& th_O_min,
                          kvs::ValueArray<float>& th_O_max,
                          kvs::ValueArray<float>& th_C_min,
                          kvs::ValueArray<float>& th_C_max,
                          // ここまでoutput, 以下input
                          const int nbins, // TFSから読み込む最大最小値
                          const kvs::ValueArray<float>& o_min,
                          const kvs::ValueArray<float>& o_max,
                          const kvs::ValueArray<float>& c_min,
                          const kvs::ValueArray<float>& c_max,
                          const float o_scalars[][SIMDW], // åæå¤
                          const float c_scalars[][SIMDW],
                          const int tf_number  )
{
    //ヒストグラムと最大最小値
    for( int i = 0; i < tf_number; i++ )
    {
        for( int I = 0; I < SIMDW; I++ )
        {
            //不透明度のヒストグラム
            float h = (o_scalars[i][I] - o_min[i])/( o_max[i] - o_min[i] )*nbins;
            int H = (int)h;
            if( 0 <= H && H <= nbins )
            {
                if( H == nbins ) H--;
                th_o_histogram[ H + nbins*i]++;
            }

            //色のヒストグラム
            h = (c_scalars[i][I] - c_min[i])/( c_max[i] - c_min[i] )*nbins;
            H = (int)h;
            if( 0 <= H && H <= nbins )
            {
                if( H == nbins ) H--;
                th_c_histogram[ H + nbins*i]++;
            }

            //不透明度の最大最小値
            th_O_min[i] = th_O_min[i] < o_scalars[i][I] ? th_O_min[i] : o_scalars[i][I];
            th_O_max[i] = th_O_max[i] > o_scalars[i][I] ? th_O_max[i] : o_scalars[i][I];
            //色の最大最小値
            th_C_min[i] = th_C_min[i] < c_scalars[i][I] ? th_C_min[i] : c_scalars[i][I];
            th_C_max[i] = th_C_max[i] > c_scalars[i][I] ? th_C_max[i] : c_scalars[i][I];
        }
    }
}

kvs::Vector3f CellByCellRejectionSampling::RandomSamplingInCube( const kvs::Vector3f vertex, kvs::MersenneTwister* MT  )
{
    const float x = (float)MT->rand();
    const float y = (float)MT->rand();
    const float z = (float)MT->rand();
    const kvs::Vector3f d( x, y, z );

    return vertex + d;
}


} // end of namespace pbvr
