/****************************************************************************/
/**
 *  @file CellByCellMinMax.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellByCellMinMax.cpp 633 2010-10-10 05:12:37Z naohisa.sakamoto $
 */
/****************************************************************************/
#include <vismodule/Compiler>
#ifdef VIS_MODULE_COMPILER_VC
#include <time.h>
#else
#include <sys/time.h>
#endif
#include "CellByCellMinMax.h"
#include <vector>
#include <stdlib.h>
#include <vismodule/DebugNew>
#include <vismodule/ObjectManager>
#include <vismodule/Camera>
//#include <vismodule/TrilinearInterpolator>
#include <vismodule/Value>
#include <vismodule/CellBase>
#include <vismodule/TrilinearInterpolator> 
#include <vismodule/TetrahedralCell>
#include <vismodule/QuadraticTetrahedralCell>
#include <vismodule/HexahedralCell>
#include <vismodule/QuadraticHexahedralCell>
#include <vismodule/PrismaticCell>
#include <vismodule/PyramidalCell>
//#include <vismodule/TriangleCell>
//#include <vismodule/QuadraticTriangleCell>
//#include <vismodule/SquareCell>
#include <vismodule/GlobalCore>
//#include <vismodule/CropRegion>

#include <vismodule/timer_simple>
#include <vismodule/FrequencyTable>

#ifdef ENABLE_MPI
#include <mpi.h>
#include "mpi_controller.h"
#include <vismodule/DistributedUniformSampling>
#endif

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

#include <vismodule/timer_simple>
#include "CalculateHistogramMinmax.h"

namespace Generator = vismodule::CellByCellParticleGenerator;

//using FuncParser::Variable;
//using FuncParser::Variables;
//using FuncParser::Function;
//using FuncParser::FunctionParser;


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellMinMax class.
 */
/*===========================================================================*/
CellByCellMinMax::CellByCellMinMax():
    vismodule::MapperBase(),
    vismodule::PointObject(),
    m_transfer_function_synthesizer( NULL ),
    m_normal_ingredient( 0 ),
    m_camera( 0 )
{
}
// unstruct
CellByCellMinMax::CellByCellMinMax( 
    domain_parameters_unstruct dom,
    Type** values, int nvariables,
    float* coordinates, int ncoords,
    unsigned int* connections, int ncells,
    const  vismodule::VolumeObjectBase::CellType& celltype, 
    const vismodule::TransferFunction& transfer_function,
    const std::vector<vismodule::TransferFunction>& transfer_function_array, 
    TransferFunctionSynthesizer* transfunc_synthesizer,
    vismodule::CoordSynthesizerStrings* coord_synthesizer_strings):
    vismodule::MapperBase( transfer_function ),
    vismodule::PointObject(),
    m_transfer_function_array( transfer_function_array ),
    m_transfer_function_synthesizer( transfunc_synthesizer ),
    m_normal_ingredient( nvariables ),
    m_coord_synthesizer_strings(coord_synthesizer_strings)
{
    this->generate_minmax_unstruct(dom, values, nvariables,
            coordinates, ncoords, connections, ncells, celltype);
}

//struct
CellByCellMinMax::CellByCellMinMax(
        domain_parameters_struct dom, 
        Type** values,  
        int nvariables, 
        const vismodule::TransferFunction& transfer_function,
        const std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        vismodule::CoordSynthesizerStrings* coord_synthesizer_strings):
    vismodule::MapperBase( transfer_function ),
    vismodule::PointObject(),
    m_transfer_function_array( transfer_function_array ),
    m_transfer_function_synthesizer( transfunc_synthesizer ),
    m_normal_ingredient( nvariables ),
    m_coord_synthesizer_strings(coord_synthesizer_strings)
{
    this->generate_minmax_struct(dom, values, nvariables);
}


/*===========================================================================*/
/**
 *  @brief  Destroys the CellByCellMetropolisSampling class.
 */
/*===========================================================================*/
CellByCellMinMax::~CellByCellMinMax()
{
    m_density_map.deallocate();
}

/*===========================================================================*/
/**
 *  @brief  Retruns the sub-pixel level.
 *  @return sub-pixel level
 */
/*===========================================================================*/
const size_t CellByCellMinMax::subpixelLevel() const
{
    return m_subpixel_level;
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampling step.
 *  @return sampling step
 */
/*===========================================================================*/
const float CellByCellMinMax::samplingStep() const
{
    return m_sampling_step;
}

/*===========================================================================*/
/**
 *  @brief  Returns the depth of the object at the center of the gravity.
 *  @return depth
 */
/*===========================================================================*/
const float CellByCellMinMax::objectDepth() const
{
    return m_object_depth;
}

/*===========================================================================*/
/**
 *  @brief  Attaches a camera.
 *  @param  camera [in] pointer to the camera
 */
/*===========================================================================*/
void CellByCellMinMax::attachCamera( const vismodule::Camera& camera )
{
    m_camera = &camera;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sub-pixel level.
 *  @param  m_subpixel_level [in] sub-pixel level
 */
/*===========================================================================*/
void CellByCellMinMax::setSubpixelLevel( const size_t subpixel_level )
{
    m_subpixel_level = subpixel_level;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sampling step.
 *  @param  m_sampling_step [in] sampling step
 */
/*===========================================================================*/
void CellByCellMinMax::setSamplingStep( const float sampling_step )
{
    m_sampling_step = sampling_step;
}

/*===========================================================================*/
/**
 *  @brief  Sets a depth of the object at the center of the gravity.
 *  @param  object_depth [in] depth
 */
/*===========================================================================*/
void CellByCellMinMax::setObjectDepth( const float object_depth )
{
    m_object_depth = object_depth;
}

void CellByCellMinMax::generate_minmax_struct(
    domain_parameters_struct dom,
    Type** values,
    int nvariables
)
{
    int nnodes = dom.resolution[0] * dom.resolution[1] * dom.resolution[2];
    const vismodule::Vector3ui resolution( dom.resolution[0], dom.resolution[1], dom.resolution[2] );
    std::cout << "resolution = " << resolution << std::endl;

    int tf_number                 = m_transfer_function_array.size();
    float sampling_volume_inverse = m_transfer_function_synthesizer->getSamplingVolumeInverse();
    float max_opacity             = m_transfer_function_synthesizer->getMaxOpacity();
    float max_density             = m_transfer_function_synthesizer->getMaxDensity();
    int max_nparticles            = (int)max_density + 1;

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

    std::vector<std::vector< vismodule::TrilinearInterpolator*>> interp;
    interp.resize( max_threads );

    for ( int i = 0; i < max_threads; i++ )
    {
        interp[i].resize( nvariables );
        for ( int j = 0; j < nvariables; j++ )
        {
             interp[i][j] = new vismodule::TrilinearInterpolator( values[j], resolution );
        }
    }

    if( mpi_rank == 0 ) std::cout << "******* max_nparticles=" << max_nparticles << std::endl;

    //ヒストグラム
    int nbins = 256;

    // 伝達関数から読み込む最大最小値
    vismodule::ValueArray<float> o_min( tf_number );
    vismodule::ValueArray<float> o_max( tf_number );
    vismodule::ValueArray<float> c_min( tf_number );
    vismodule::ValueArray<float> c_max( tf_number );

    SuperClass::m_c_histogram = vismodule::ValueArray<int> (tf_number * nbins); // 色ヒストグラムの配列
    SuperClass::m_o_histogram = vismodule::ValueArray<int> (tf_number * nbins); // 不透明度ヒストグラムの配列
    SuperClass::setTfnumber(tf_number);
    SuperClass::setNbins(nbins);
    
    m_o_histogram.fill(0x00);
    m_c_histogram.fill(0x00);

    for( size_t i = 0; i < tf_number; i++ )
    {
        o_min[i] = m_transfer_function_array[i].opacityMap().minValue();
        o_max[i] = m_transfer_function_array[i].opacityMap().maxValue();
        c_min[i] = m_transfer_function_array[i].colorMap().minValue();
        c_max[i] = m_transfer_function_array[i].colorMap().maxValue();
    }

    // min max
    vismodule::ValueArray<float> O_min( tf_number );
    vismodule::ValueArray<float> O_max( tf_number );
    vismodule::ValueArray<float> C_min( tf_number );
    vismodule::ValueArray<float> C_max( tf_number );

    // initialize
    for ( size_t i = 0; i < tf_number; i++ )
    {
        O_min[i] =  FLT_MAX;
        O_max[i] = -FLT_MAX;
        C_min[i] =  FLT_MAX;
        C_max[i] = -FLT_MAX;
    }

    TransferFunctionSynthesizer** th_tfs = new TransferFunctionSynthesizer*[max_threads];
    std::vector<std::vector<vismodule::TransferFunction>> th_tf;

    for ( int n = 0; n < max_threads; n++ )
    {
        th_tfs[n] = new TransferFunctionSynthesizer( *m_transfer_function_synthesizer );
    }

    th_tf.resize( max_threads );

    for ( int i = 0; i < max_threads; i++ )
    {
        th_tf[i].resize( tf_number );
        for ( int j = 0; j < tf_number; j++ )
        {
            th_tf[i][j] = m_transfer_function_array[j];
        }
    }

    float* opacity_volume = new float[resolution.x() * resolution.y() * resolution.z()];

    // time_parameters time;
    // timer.stop();
    // time.initialize = timer.sec();
    // timer.start();

    // 頂点解像度と格子解像度
    const int nx = resolution.x();
    const int ny = resolution.y();
    const int nz = resolution.z();
    const int nxy = nx * ny;
    const int nx_1 = nx-1;
    const int ny_1 = ny-1;
    const int nz_1 = nz-1;
    const int nxy_1 = nx_1 * ny_1;

    // int total_nparticles = 0;

    CoordSynthesizerStrings css;
    if ( m_coord_synthesizer_strings ) 
    {
        css = *m_coord_synthesizer_strings;
    }

    // static TimedScope td_gatherf("GatherF",1);
    // static TimedScope td_gather("gather",1);
    // static TimedScope td_kvsml("kvsml",1);
    // static TimedScope td_SynthOpacityScalars("SynthesizedOpacityScalars",max_threads);
    // static TimedScope td_SynthColorScalars("SynthesizedColorScalars",max_threads);
    // static TimedScope td_CalculateHistogram("CalculateHistogram",max_threads);
    // static TimedScope td_CalculateOpacity("CalculateOpacity",max_threads);
    // static TimedScope td_CalculateDensity("CalculateDensity",max_threads);
    // static TimedScope td_CalculateNumPar("Ccalculate_number_of_particles (Random)",max_threads);
    // static TimedScope td_CalculateColor("CalculateColor",max_threads);
    // static  TimedScope td_VectorPush("Vector Push",max_threads);
    // static TimedScope td_VectorIns("Vector Insert",max_threads);

    #pragma omp parallel
    {
#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif

        // th_tfs[thid]->set_debug_thid(thid,max_threads);

        // 各スレッド番号をシードにした乱数生成器
        vismodule::MersenneTwister MT( thid + mpi_rank * nthreads );

        // ヒストグラムの配列
        // float o_scalars[tf_number][SIMDW]; // 頂点の不透明度
        // float c_scalars[tf_number][SIMDW]; // 頂点の色

        float** o_scalars = new float* [tf_number];
        float** c_scalars = new float* [tf_number];

        for (int i = 0; i < tf_number; i++)
        {
            o_scalars[i] = new float[SIMDW];
            c_scalars[i] = new float[SIMDW];
        }

        vismodule::ValueArray<int> th_o_histogram( tf_number * nbins ); // 不透明度
        vismodule::ValueArray<int> th_c_histogram( tf_number * nbins ); // 色

        th_o_histogram.fill(0x00);
        th_c_histogram.fill(0x00);

        // 計算して得る最大最小値
        vismodule::ValueArray<float> th_O_min( tf_number );
        vismodule::ValueArray<float> th_O_max( tf_number );
        vismodule::ValueArray<float> th_C_min( tf_number );
        vismodule::ValueArray<float> th_C_max( tf_number );

        // 初期化
        for ( int i = 0; i < tf_number; i++ )
        {
            th_O_min[i] =  FLT_MAX;
            th_O_max[i] = -FLT_MAX;
            th_C_min[i] =  FLT_MAX;
            th_C_max[i] = -FLT_MAX;
        }

        // minmax coordの設定
        const vismodule::Vector3f min_vec( 
            dom.x_min, 
            dom.y_min, 
            dom.z_min
        ); 
        const vismodule::Vector3f max_vec( 
            dom.x_max, 
            dom.y_max, 
            dom.z_max
        ); 

        const vismodule::Vector3f cell_length(
            (max_vec.x() - min_vec.x() ) / nx_1,
            (max_vec.y() - min_vec.y() ) / ny_1,
            (max_vec.z() - min_vec.z() ) / nz_1
        );

        // Calculate MinMax
        {
            // Marge x-y-z loop
            const int nvertices = nx * ny * nz;
            // "+ 1" means remained loop
            const int outer_loop = (nvertices % SIMDW == 0) ? nvertices / SIMDW : nvertices / SIMDW + 1;

            #pragma omp for
            for ( int J = 0; J < outer_loop; J++ )
            {
                int ncells = ((J + 1) * SIMDW > nvertices) ? nvertices - J * SIMDW : SIMDW;
                float X_l[SIMDW], Y_l[SIMDW], Z_l[SIMDW];
                float X_g[SIMDW], Y_g[SIMDW], Z_g[SIMDW];

                #pragma ivdep
                for ( int I = 0; I < SIMDW; I++ )
                {
                    const int vertex_id = I + J * SIMDW;
                    const int k =  vertex_id / nxy;
                    const int j = (vertex_id - k * nxy) / nx;
                    const int i =  vertex_id - k * nxy - j * nx;

                    const float x_l = (float)i;
                    const float y_l = (float)j;
                    const float z_l = (float)k;
                    const float x_g = (x_l * cell_length.x()) + min_vec.x();
                    const float y_g = (y_l * cell_length.y()) + min_vec.y();
                    const float z_g = (z_l * cell_length.z()) + min_vec.z();

                    X_l[I] = x_l;
                    Y_l[I] = y_l;
                    Z_l[I] = z_l;
                    X_g[I] = x_g;
                    Y_g[I] = y_g;
                    Z_g[I] = z_g;
                }

                th_tfs[thid]->SynthesizedOpacityScalars(
                    interp[thid],
                    X_l, Y_l, Z_l,
                    X_g, Y_g, Z_g,
                    o_scalars
                );

                th_tfs[thid]->SynthesizedColorScalars(
                    interp[thid],
                    X_l, Y_l, Z_l,
                    X_g, Y_g, Z_g,
                    c_scalars
                );

                for( int i = 0; i < tf_number; i++ )
                {
                    for( int I = 0; I < ncells; I++ )
                    {
                        th_O_min[i] = th_O_min[i] < o_scalars[i][I] ? th_O_min[i] : o_scalars[i][I];
                        th_O_max[i] = th_O_max[i] > o_scalars[i][I] ? th_O_max[i] : o_scalars[i][I];
                        th_C_min[i] = th_C_min[i] < c_scalars[i][I] ? th_C_min[i] : c_scalars[i][I];
                        th_C_max[i] = th_C_max[i] > c_scalars[i][I] ? th_C_max[i] : c_scalars[i][I];
                    }
                }
            }
        } // end of Calculate MinMax
        
        #pragma omp critical
        {
            // 最大最小値
            for( size_t i = 0; i < tf_number; i++ )
            {
                // 不透明度
                O_min[i] = O_min[i] < th_O_min[i] ? O_min[i] : th_O_min[i];
                O_max[i] = O_max[i] > th_O_max[i] ? O_max[i] : th_O_max[i];
                // 色
                C_min[i] = C_min[i] < th_C_min[i] ? C_min[i] : th_C_min[i];
                C_max[i] = C_max[i] > th_C_max[i] ? C_max[i] : th_C_max[i];
            }
        } // end of omp critical
    }
        for( size_t i = 0; i < tf_number; i++ )
        {
            o_min[i] = O_min[i];
            o_max[i] = O_max[i];
            c_min[i] = C_min[i];
            c_max[i] = C_max[i];
        }

    // set minmax range
    m_transfer_function_synthesizer->m_o_min.resize(tf_number);
    m_transfer_function_synthesizer->m_o_max.resize(tf_number);
    m_transfer_function_synthesizer->m_c_min.resize(tf_number);
    m_transfer_function_synthesizer->m_c_max.resize(tf_number);

    for ( int i = 0; i < tf_number; i++ )
    {
        m_transfer_function_synthesizer->m_o_min[i] = O_min[i];
        m_transfer_function_synthesizer->m_o_max[i] = O_max[i];
        m_transfer_function_synthesizer->m_c_min[i] = C_min[i];
        m_transfer_function_synthesizer->m_c_max[i] = C_max[i];
    }

    // timer.stop();
    // time.sampling = timer.sec();
    // time.nparticles = vertex_coords.size()/3;
    // timer.start();

    for( int i = 0; i < max_threads; i++ )
    {
        delete th_tfs[i];
    }
    delete[] th_tfs;

    // delete tfs;
    delete[] opacity_volume;

    for ( int i = 0; i < max_threads; i++ )
    {
        for ( int j = 0; j < nvariables; j++ )
        {
             delete interp[i][j];
        }
    }

    return;
}

void CellByCellMinMax::generate_minmax_unstruct(
    domain_parameters_unstruct dom,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& cellType
)
{
    double start = GetTime();
    size_t resolution = DEFAULT_NBINS;

    int tf_number                  = m_transfer_function_array.size();
    float sampling_volume_inverse  = m_transfer_function_synthesizer->getSamplingVolumeInverse();
    float max_opacity              = m_transfer_function_synthesizer->getMaxOpacity();
    float max_density              = m_transfer_function_synthesizer->getMaxDensity();
    float particle_data_size_limit = m_transfer_function_synthesizer->getParticleDataSizeLimit();
    const int max_nparticles       = (int)max_density + 1;

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

    std::vector<std::vector<vismodule::CellBase<Type>*>> interp;
    interp.resize( max_threads );

    switch ( cellType )
    {
        case vismodule::VolumeObjectBase::Tetrahedra:
            {
                if (mpi_rank == 0) std::cout << "celltype: tetrahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new vismodule::TetrahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case vismodule::VolumeObjectBase::QuadraticTetrahedra:
            {
                if (mpi_rank == 0)std::cout << "Cell type : Quadratic tetrahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new vismodule::QuadraticTetrahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case vismodule::VolumeObjectBase::Hexahedra:
            {
                if (mpi_rank == 0) std::cout << "celltype: hexahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables  );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new vismodule::HexahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case vismodule::VolumeObjectBase::QuadraticHexahedra:
            {
                if (mpi_rank == 0) std::cout << "celltype: quadratichexahedra " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new vismodule::QuadraticHexahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case vismodule::VolumeObjectBase::Prism:
            {
                if (mpi_rank == 0) std::cout << "celltype: prism " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new vismodule::PrismaticCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
        case vismodule::VolumeObjectBase::Pyramid:
            {
                if (mpi_rank == 0) std::cout << "celltype: pyramid " << std::endl; 
                for ( int i = 0; i < max_threads; i++ )
                {
                    interp[ i ].resize( nvariables );
                    for ( int j = 0; j < nvariables; j++ )
                    {
                        interp[i][j]  = new vismodule::PyramidalCell<Type>( values[j], coordinates, ncoords, connections, ncells );
                    }
                }
                break;
            }
//        case vismodule::VolumeObjectBase::Triangle:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new vismodule::TriangleCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
//        case vismodule::VolumeObjectBase::QuadraticTriangle:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new vismodule::QuadraticTriangleCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
//        case vismodule::VolumeObjectBase::Square:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new vismodule::HexahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
//        case vismodule::VolumeObjectBase::QuadraticSquare:
//            {
//                for ( int i = 0; i < max_threads; i++ )
//                {
//                    interp[ i ].resize( nvariables );
//                    for ( int j = 0; j < nvariables; j++ )
//                    {
//                        interp[i][j]  = new vismodule::HexahedralCell<Type>( values[j], coordinates, ncoords, connections, ncells );
//                    }
//                }
//                break;
//            }
        default:
            {
                BaseClass::m_is_success = false;
                visModuleMessageError( "Unsupported cell type." );
                return;
            }
    }
    //    VIS_MODULE_TIMER_END( 270 );

    SuperClass::m_color_histogram   = vismodule::ValueArray<vismodule::FrequencyTable>( tf_number );
    SuperClass::m_opacity_histogram = vismodule::ValueArray<vismodule::FrequencyTable>( tf_number );

    std::cout<<"******* getMaxDensity() = " << m_transfer_function_synthesizer->getMaxDensity() << std::endl;

    if( mpi_rank == 0 ) std::cout << "******* max_nparticles=" << max_nparticles << std::endl;

    // ヒストグラム
    int nbins = 256;

    // 伝達関数から読み込む最大最小値
    vismodule::ValueArray<float> o_min( tf_number );
    vismodule::ValueArray<float> o_max( tf_number );
    vismodule::ValueArray<float> c_min( tf_number );
    vismodule::ValueArray<float> c_max( tf_number );
    
    SuperClass::m_c_histogram = vismodule::ValueArray<int> (tf_number * nbins); // 色ヒストグラムの配列
    SuperClass::m_o_histogram = vismodule::ValueArray<int> (tf_number * nbins); // 不透明度ヒストグラムの配列
    SuperClass::setTfnumber(tf_number);
    SuperClass::setNbins(nbins);

    m_o_histogram.fill(0x00);
    m_c_histogram.fill(0x00);

    for( size_t i = 0; i < tf_number; i++ )
    {
        o_min[i] = m_transfer_function_array[i].opacityMap().minValue();
        o_max[i] = m_transfer_function_array[i].opacityMap().maxValue();
        c_min[i] = m_transfer_function_array[i].colorMap().minValue();
        c_max[i] = m_transfer_function_array[i].colorMap().maxValue();
    }

    // min max
    vismodule::ValueArray<float> O_min( tf_number );
    vismodule::ValueArray<float> O_max( tf_number );
    vismodule::ValueArray<float> C_min( tf_number );
    vismodule::ValueArray<float> C_max( tf_number );

    // dynamic array for paritcle
    std::vector<float> vertex_coords;
    std::vector<Byte>  vertex_colors;
    std::vector<float> vertex_normals;

    // initialize
    for ( size_t i = 0; i < tf_number; i++ )
    {
        O_min[i] =  FLT_MAX;
        O_max[i] = -FLT_MAX;
        C_min[i] =  FLT_MAX;
        C_max[i] = -FLT_MAX;
    }

    TransferFunctionSynthesizer** th_tfs = new TransferFunctionSynthesizer*[max_threads];
    std::vector<std::vector<vismodule::TransferFunction>> th_tf;

    for ( int n = 0; n < max_threads; n++ )
    {
        th_tfs[n] = new TransferFunctionSynthesizer( *m_transfer_function_synthesizer );
    }

    th_tf.resize( max_threads );

    for ( int i = 0; i < max_threads; i++ )
    {
        th_tf[i].resize( tf_number );
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

    int particles_process_limit = static_cast<int>((particle_data_size_limit * 10E6) / (sizeof( float ) + sizeof( Byte ) + sizeof( float )));

    CoordSynthesizerStrings css;
    if ( m_coord_synthesizer_strings ) 
    {
        css = *m_coord_synthesizer_strings;
    }

#pragma omp parallel
    {
#if _OPENMP
        int nthreads = omp_get_num_threads();
        int thid     = omp_get_thread_num();
#else
        int nthreads = 1;
        int thid     = 0;
#endif

       vismodule::MersenneTwister MT( thid + mpi_rank * nthreads );

        // 動的な粒子データ配列
        std::vector<float> th_vertex_coords;
        std::vector<Byte>  th_vertex_colors;
        std::vector<float> th_vertex_normals;

        // ヒストグラムの配列
        std::vector<float> o_scalars( tf_number ); // 頂点の不透明度
        std::vector<float> c_scalars( tf_number ); // 頂点の色
        vismodule::ValueArray<int> th_o_histogram( tf_number * nbins ); // 不透明度
        vismodule::ValueArray<int> th_c_histogram( tf_number * nbins ); // 色

        th_o_histogram.fill(0x00);
        th_c_histogram.fill(0x00);

        // 計算して得る最大最小値
        vismodule::ValueArray<float> th_O_min( tf_number );
        vismodule::ValueArray<float> th_O_max( tf_number );
        vismodule::ValueArray<float> th_C_min( tf_number );
        vismodule::ValueArray<float> th_C_max( tf_number );

        // 初期化
        for ( int i = 0; i < tf_number; i++ )
        {
            th_O_min[i] =  FLT_MAX;
            th_O_max[i] = -FLT_MAX;
            th_C_min[i] =  FLT_MAX;
            th_C_max[i] = -FLT_MAX;
        }

        // 配列の追加
        vismodule::Vector3f local_center_array[ SIMDW ];
        vismodule::Vector3f global_center_array[ SIMDW ];
        vismodule::UInt32 cell_index[ SIMDW ];

        float cell_opacity_array[ SIMDW ];
        std::vector<float> o_scalars_array[ SIMDW ];
        std::vector<float> c_scalars_array[ SIMDW ];

        for (int i = 0; i < SIMDW; i++ )
        {
            o_scalars_array[i].resize( tf_number );
            c_scalars_array[i].resize( tf_number );
        }

        int nparticles_array[ SIMDW ];

        vismodule::Vector3f local_coord_array[ SIMDW ];
        vismodule::Vector3f global_coord_array[ SIMDW ];
        float density_array[ SIMDW ];

        vismodule::Vector3f l_plus_coord[ SIMDW ];
        vismodule::Vector3f l_minus_coord[ SIMDW ];
        vismodule::Vector3f g_plus_coord[ SIMDW ];
        vismodule::Vector3f g_minus_coord[ SIMDW ];
        float S_plus_opacity[ SIMDW ];
        float S_minus_opacity[ SIMDW ];
        float dsdx_array[ SIMDW ];
        float dsdy_array[ SIMDW ];
        float dsdz_array[ SIMDW ];
        vismodule::Vector3f grad_array[ SIMDW ];
        vismodule::RGBColor color_array[ SIMDW ];

        // Calculate MinMax
#pragma omp for schedule( dynamic ) nowait
        for( int cell_base = 0; cell_base < ncells; cell_base += SIMDW )
        {
            // ブロック内でのループ回数を取得
            int remain = ( ncells - cell_base > SIMDW )? SIMDW: ncells - cell_base;

            /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            // 一括でセルをバインドするための配列と、座標の取得
            for( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (vismodule::UInt32)(cell_base + cell_BLK);
                // local_center_array[cell_BLK] = vismodule::Vector3f ( 0.5, 0.5, 0.5 );
                local_center_array[cell_BLK] = vismodule::Vector3f ( 0, 0, 0 );
                //local_center_array[cell_BLK] = interp[thid][0]->localGravityPoint();
            }

            // 補間器にセルを一括でバインド
            for( int i = 0; i < nvariables; i++ )
            {
                interp[thid][i]->bindCellArray(remain, cell_index);
            }

            interp[thid][0]->setLocalPointArray( remain, local_center_array );
            interp[thid][0]->transformLocalToGlobalArray(
                remain,
                local_center_array,
                global_center_array
            );
            
            th_tfs[thid]->SynthesizedOpacityScalarsArray(
                interp[thid],
                remain,
                local_center_array,
                global_center_array,
                o_scalars_array
            );

            th_tfs[thid]->SynthesizedColorScalarsArray(
                interp[thid],
                remain,
                local_center_array,
                global_center_array,
                c_scalars_array
            );

            for( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                for( size_t i = 0; i < tf_number; i++ )
                {
                    th_O_min[i] = th_O_min[i] < o_scalars_array[cell_BLK][i] ? th_O_min[i] : o_scalars_array[cell_BLK][i];
                    th_O_max[i] = th_O_max[i] > o_scalars_array[cell_BLK][i] ? th_O_max[i] : o_scalars_array[cell_BLK][i];
                    th_C_min[i] = th_C_min[i] < c_scalars_array[cell_BLK][i] ? th_C_min[i] : c_scalars_array[cell_BLK][i];
                    th_C_max[i] = th_C_max[i] > c_scalars_array[cell_BLK][i] ? th_C_max[i] : c_scalars_array[cell_BLK][i];
                }
            }
        } // end of for cell (Calculate MinMax)

#pragma omp critical
        {
            // 最大最小値
            for( size_t i = 0; i < tf_number; i++ )
            {
                // 不透明度
                O_min[i] = O_min[i] < th_O_min[i] ? O_min[i] : th_O_min[i];
                O_max[i] = O_max[i] > th_O_max[i] ? O_max[i] : th_O_max[i];
                // 色
                C_min[i] = C_min[i] < th_C_min[i] ? C_min[i] : th_C_min[i];
                C_max[i] = C_max[i] > th_C_max[i] ? C_max[i] : th_C_max[i];
            }
        }
    }
        for( size_t i = 0; i < tf_number; i++ )
        {
            o_min[i] = O_min[i];
            o_max[i] = O_max[i];
            c_min[i] = C_min[i];
            c_max[i] = C_max[i];
        }

    // set minmax range
    m_transfer_function_synthesizer->m_o_min.resize(tf_number);
    m_transfer_function_synthesizer->m_o_max.resize(tf_number);
    m_transfer_function_synthesizer->m_c_min.resize(tf_number);
    m_transfer_function_synthesizer->m_c_max.resize(tf_number);

    for ( size_t i = 0; i < tf_number; i++ )
    {
        m_transfer_function_synthesizer->m_o_min[i] = O_min[i];
        m_transfer_function_synthesizer->m_o_max[i] = O_max[i];
        m_transfer_function_synthesizer->m_c_min[i] = C_min[i];
        m_transfer_function_synthesizer->m_c_max[i] = C_max[i];
    }

    for( int i = 0; i < max_threads; i++ )
    {
        delete th_tfs[i];
    }
    delete[] th_tfs;

    for ( int i = 0; i < max_threads; i++ )
    {
        for ( int j = 0; j < interp[i].size(); j++ )
        {
             if (interp[i][j] != NULL) delete interp[i][j];
        }
    }

    //TIMER_END( 290 );

    SuperClass::setSize( 1.0f );
    //TIMER_END( 280 );
    double end = GetTime();
    printf( " \n  CPU:generate_minmax: %lf ms\n", end - start );
}

/*===========================================================================*/
/**
 *  @brief  Calculate maximum dentiy value.
 *  @param  scalar0 [in] scalar value
 *  @param  scalar1 [in] scalar value
 *  @return density value
 */
/*===========================================================================*/

} // end of namespace vismodule

