/****************************************************************************/
/**
 *  @file CellByCellHistogram.cpp
 */
/*----------------------------------------------------------------------------
 *
 *  Copyright (c) Visualization Laboratory, Kyoto University.
 *  All rights reserved.
 *  See http://www.viz.media.kyoto-u.ac.jp/kvs/copyright/ for details.
 *
 *  $Id: CellByCellHistogram.cpp 633 2010-10-10 05:12:37Z naohisa.sakamoto $
 */
/****************************************************************************/
#include <vismodule/Compiler>
#ifdef VIS_MODULE_COMPILER_VC
#include <time.h>
#else
#include <sys/time.h>
#endif
#include "CellByCellHistogram.h"
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

using FuncParser::Variable;
using FuncParser::Variables;
using FuncParser::Function;
using FuncParser::FunctionParser;


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellHistogram class.
 */
/*===========================================================================*/
CellByCellHistogram::CellByCellHistogram():
    vismodule::MapperBase(),
    vismodule::PointObject(),
    m_transfer_function_synthesizer( NULL ),
    m_normal_ingredient( 0 ),
    m_camera( 0 )
{
}
// unstruct
CellByCellHistogram::CellByCellHistogram( 
    domain_parameters_unstruct dom,
    Type** values, int nvariables,
    float* coordinates, int ncoords,
    unsigned int* connections, int ncells,
    const  vismodule::VolumeObjectBase::CellType& celltype, 
    const vismodule::TransferFunction& transfer_function,
    std::vector<vismodule::TransferFunction>& transfer_function_array, 
    TransferFunctionSynthesizer* transfunc_synthesizer,
    const float                  particle_density,
    vismodule::CoordSynthesizerStrings* coord_synthesizer_strings):
    vismodule::MapperBase( transfer_function ),
    vismodule::PointObject(),
    m_transfer_function_array( transfer_function_array ),
    m_transfer_function_synthesizer( transfunc_synthesizer ),
    m_normal_ingredient( nvariables ),
    m_particle_density( particle_density ),
    m_coord_synthesizer_strings(coord_synthesizer_strings)
{
    this->generate_histogram_unstruct(dom, values, nvariables,
            coordinates, ncoords, connections, ncells, celltype);
}

//struct
CellByCellHistogram::CellByCellHistogram(
        domain_parameters_struct dom, 
        Type** values,  
        int nvariables, 
        const vismodule::TransferFunction& transfer_function,
        std::vector<vismodule::TransferFunction>& transfer_function_array,
        TransferFunctionSynthesizer* transfunc_synthesizer,
        const float                  particle_density,
        vismodule::CoordSynthesizerStrings* coord_synthesizer_strings):
    vismodule::MapperBase( transfer_function ),
    vismodule::PointObject(),
    m_transfer_function_array( transfer_function_array ),
    m_transfer_function_synthesizer( transfunc_synthesizer ),
    m_normal_ingredient( nvariables ),
    m_particle_density( particle_density), 
    m_coord_synthesizer_strings(coord_synthesizer_strings)
{
    this->generate_histogram_struct(dom, values, nvariables);
}


/*===========================================================================*/
/**
 *  @brief  Destroys the CellByCellMetropolisSampling class.
 */
/*===========================================================================*/
CellByCellHistogram::~CellByCellHistogram()
{
    m_density_map.deallocate();
}

/*===========================================================================*/
/**
 *  @brief  Retruns the sub-pixel level.
 *  @return sub-pixel level
 */
/*===========================================================================*/
const size_t CellByCellHistogram::subpixelLevel() const
{
    return m_subpixel_level;
}

/*===========================================================================*/
/**
 *  @brief  Returns the sampling step.
 *  @return sampling step
 */
/*===========================================================================*/
const float CellByCellHistogram::samplingStep() const
{
    return m_sampling_step;
}

/*===========================================================================*/
/**
 *  @brief  Returns the depth of the object at the center of the gravity.
 *  @return depth
 */
/*===========================================================================*/
const float CellByCellHistogram::objectDepth() const
{
    return m_object_depth;
}

/*===========================================================================*/
/**
 *  @brief  Attaches a camera.
 *  @param  camera [in] pointer to the camera
 */
/*===========================================================================*/
void CellByCellHistogram::attachCamera( const vismodule::Camera& camera )
{
    m_camera = &camera;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sub-pixel level.
 *  @param  m_subpixel_level [in] sub-pixel level
 */
/*===========================================================================*/
void CellByCellHistogram::setSubpixelLevel( const size_t subpixel_level )
{
    m_subpixel_level = subpixel_level;
}

/*===========================================================================*/
/**
 *  @brief  Sets a sampling step.
 *  @param  m_sampling_step [in] sampling step
 */
/*===========================================================================*/
void CellByCellHistogram::setSamplingStep( const float sampling_step )
{
    m_sampling_step = sampling_step;
}

/*===========================================================================*/
/**
 *  @brief  Sets a depth of the object at the center of the gravity.
 *  @param  object_depth [in] depth
 */
/*===========================================================================*/
void CellByCellHistogram::setObjectDepth( const float object_depth )
{
    m_object_depth = object_depth;
}

CellByCellHistogram::SuperClass* CellByCellHistogram::generate_histogram_struct(  domain_parameters_struct dom, Type** values, int nvariables )
{

    int nnodes = dom.resolution[0]*dom.resolution[1]*dom.resolution[2];
    const vismodule::Vector3ui resolution(dom.resolution[0],dom.resolution[1],dom.resolution[2] );
#if 1
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

    std::vector< std::vector< vismodule::TrilinearInterpolator* > >  interp;
    interp.resize( max_threads );
    for ( int i = 0; i < max_threads; i++ )
    {
        interp[ i ].resize( nvariables );
        for ( int j = 0; j < nvariables; j++ )
        {
             interp[i][j]  = new vismodule::TrilinearInterpolator( values[j], resolution);
        }
    }

    static bool parameter_file_opened=true;
    const int max_nparticles = (int)m_transfer_function_synthesizer->getMaxDensity() + 1;
    if(mpi_rank==0) std::cout<<"******* max_nparticles="<<max_nparticles<<std::endl;

    //ｿｿｿｿｿｿ
    int nbins = 256;
    vismodule::ValueArray<float> o_min( tf_number );//TFSｿｿｿｿｿｿｿｿｿｿｿ
    vismodule::ValueArray<float> o_max( tf_number );
    vismodule::ValueArray<float> c_min( tf_number );
    vismodule::ValueArray<float> c_max( tf_number );

    // 2023/07/31 add by shimomura
    SuperClass::m_c_histogram = vismodule::ValueArray<int> (tf_number * nbins);
    SuperClass::m_o_histogram = vismodule::ValueArray<int> (tf_number * nbins);
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

    //ｿｿｿｿｿ
    vismodule::ValueArray<float> O_min( tf_number );//ｿｿｿｿｿｿｿｿｿｿｿ
    vismodule::ValueArray<float> O_max( tf_number );
    vismodule::ValueArray<float> C_min( tf_number );
    vismodule::ValueArray<float> C_max( tf_number );

    // ｿｿｿｿｿｿｿｿｿｿ
    std::vector<float> vertex_coords;
    std::vector<Byte>  vertex_colors;
    std::vector<float> vertex_normals;

    if( parameter_file_opened )
    {
        for ( size_t i = 0; i < tf_number; i++ ) //ｿｿｿ
        {
            O_min[ i ] =  FLT_MAX;
            O_max[ i ] = -FLT_MAX;
            C_min[ i ] =  FLT_MAX;
            C_max[ i ] = -FLT_MAX;
        }
    }

    TransferFunctionSynthesizer** th_tfs = new TransferFunctionSynthesizer*[max_threads];
    std::vector< std::vector<vismodule::TransferFunction> > th_tf;

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

    float* opacity_volume = new float[ resolution.x()*resolution.y()*resolution.z() ];

//    time_parameters time;
//    timer.stop();
//    time.initialize = timer.sec();
//    timer.start();

    //ｿｿｿｿｿｿｿｿｿｿｿ
    const int nx = resolution.x();
    const int ny = resolution.y();
    const int nz = resolution.z();
    const int nxy = nx * ny;
    const int nx_1 = nx-1;
    const int ny_1 = ny-1;
    const int nz_1 = nz-1;
    const int nxy_1 = nx_1 * ny_1;

    int total_nparticles = 0;

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

        int th_total_nparticles = 0;
        //ｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿ
        vismodule::MersenneTwister MT( thid + mpi_rank * nthreads );

        // ｿｿｿｿｿｿｿｿｿｿ
        std::vector<float> th_vertex_coords;
        std::vector<Byte>  th_vertex_colors;
        std::vector<float> th_vertex_normals;

        //ｿｿｿｿｿｿｿｿｿ
//        float o_scalars[tf_number][SIMDW];//ｿｿｿｿｿｿｿ
//        float c_scalars[tf_number][SIMDW];//ｿｿｿｿ

        float** o_scalars = new float* [tf_number];
        float** c_scalars = new float* [tf_number];

        for (int i = 0; i < tf_number; i++)
        {
            o_scalars[i] = new float[SIMDW];
            c_scalars[i] = new float[SIMDW];
        }

        vismodule::ValueArray<int> th_o_histogram( tf_number * nbins );//ｿｿｿｿ
        vismodule::ValueArray<int> th_c_histogram( tf_number * nbins );//ｿ

        if( parameter_file_opened )
        {
            th_o_histogram.fill(0x00);
            th_c_histogram.fill(0x00);
        }

        //ｿｿｿｿｿ
        vismodule::ValueArray<float> th_O_min( tf_number );//ｿｿｿｿｿｿｿｿｿｿｿ
        vismodule::ValueArray<float> th_O_max( tf_number );
        vismodule::ValueArray<float> th_C_min( tf_number );
        vismodule::ValueArray<float> th_C_max( tf_number );

        if( parameter_file_opened )
        {
            for ( int i = 0; i < tf_number; i++ ) //ｿｿｿ
            {
                th_O_min[ i ] =  FLT_MAX;
                th_O_max[ i ] = -FLT_MAX;
                th_C_min[ i ] =  FLT_MAX;
                th_C_max[ i ] = -FLT_MAX;
            }
        }

        // minmax coordの設定
        const vismodule::Vector3f min_vec( 
                dom.x_min, 
                dom.y_min, 
                dom.z_min); 
       const  vismodule::Vector3f max_vec( 
                dom.x_max, 
                dom.y_max, 
                dom.z_max ); 

//        const vismodule::Vector3f min_vec = volume.minObjectCoord(); 
//        const vismodule::Vector3f max_vec = volume.maxObjectCoord(); 
        const vismodule::Vector3f cell_length( (max_vec.x() - min_vec.x() )/ nx_1,
                                         (max_vec.y() - min_vec.y() )/ ny_1,
                                         (max_vec.z() - min_vec.z() )/ nz_1) ;

#if 1
       calculateMinmax_struct( nx, ny, nz, min_vec, cell_length,
               nvariables, thid, tf_number,
               interp, th_tfs, 
               o_scalars, c_scalars,
               th_O_min, th_O_max, th_C_min, th_C_max,  
               O_min, O_max, C_min, C_max ); 

#endif
            for( int i = 0; i < tf_number; i++ )
            {
                o_min[i] = O_min[i];
                o_max[i] = O_max[i];
                c_min[i] = C_min[i];
                c_max[i] = C_max[i];
            }

#if 1
            calculateHistogram_struct( nx, ny, nz, min_vec, cell_length,
               nvariables, thid, tf_number,
               interp, th_tfs, 
               o_scalars, c_scalars,
               o_min, o_max, c_min, c_max, 
               th_o_histogram,th_c_histogram );
#endif
//        timed_section_start(td_VectorIns,thid);
        #pragma omp critical
        {
            if( parameter_file_opened )
            {

                for( int n = 0; n < tf_number * nbins; n++ )
                {
                    m_o_histogram[n] += th_o_histogram[n];
                    m_c_histogram[n] += th_c_histogram[n];
                }
            }

        } // end of omp critical
//        timed_section_end(td_VectorIns,thid);
    } // end of omp parallel

    //if(mpi->rank == 0) std::cout<<"total_nparticles="<<total_nparticles<<std::endl;
    //std::cout<<"rank="<<mpi_rank<<",total_nparticles="<<total_nparticles<<std::endl;

    // add by shimomura 
    // set variable range
    m_transfer_function_synthesizer->m_o_min.resize(tf_number);        
    m_transfer_function_synthesizer->m_o_max.resize(tf_number);        
    m_transfer_function_synthesizer->m_c_min.resize(tf_number);        
    m_transfer_function_synthesizer->m_c_max.resize(tf_number);        
    for (int i = 0; i < tf_number; i++)
    {
        m_transfer_function_synthesizer->m_o_min[i] = m_transfer_function_synthesizer->m_o_min[i] < O_min[i] ? m_transfer_function_synthesizer->m_o_min[i] : O_min[i];
        m_transfer_function_synthesizer->m_o_max[i] = m_transfer_function_synthesizer->m_o_max[i] > O_max[i] ? m_transfer_function_synthesizer->m_o_max[i] : O_max[i];
        m_transfer_function_synthesizer->m_c_min[i] = m_transfer_function_synthesizer->m_c_min[i] < C_min[i] ? m_transfer_function_synthesizer->m_c_min[i] : C_min[i];
        m_transfer_function_synthesizer->m_c_max[i] = m_transfer_function_synthesizer->m_c_max[i] > C_max[i] ? m_transfer_function_synthesizer->m_c_max[i] : C_max[i];
    }

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
 
#endif
}

CellByCellHistogram::SuperClass* CellByCellHistogram::generate_histogram_unstruct( domain_parameters_unstruct dom,Type** values, int nvariables,
        float* coordinates, int ncoords,
        unsigned int* connections, int ncells,
        const  vismodule::VolumeObjectBase::CellType& cellType )
{
    double start = GetTime();
    size_t resolution = DEFAULT_NBINS;

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

//    int mpi_rank;
//    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );

    //if(mpi->rank==0)std::cout<<"start generate_particles\n";
    static bool start_flag = true;
    static bool parameter_file_opened=true;

    std::vector< std::vector< vismodule::CellBase<Type>* > >  interp;

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

    int tf_number = m_transfer_function_array.size();

    SuperClass::m_color_histogram  = vismodule::ValueArray<vismodule::FrequencyTable>( tf_number );
    SuperClass::m_opacity_histogram  = vismodule::ValueArray<vismodule::FrequencyTable>( tf_number );

    std::cout<<"******* getMaxDensity()="<<m_transfer_function_synthesizer->getMaxDensity()<<std::endl;
    const int max_nparticles = (int)m_transfer_function_synthesizer->getMaxDensity() + 1;

    if(mpi_rank==RANK) std::cout<<"******* max_nparticles="<<max_nparticles<<std::endl;

    int nbins = 256;
    vismodule::ValueArray<float> o_min( tf_number );//read minmax from TFS
    vismodule::ValueArray<float> o_max( tf_number );
    vismodule::ValueArray<float> c_min( tf_number );
    vismodule::ValueArray<float> c_max( tf_number );


    vismodule::ValueArray<int> o_histogram( tf_number * nbins );// opacity histogram
    vismodule::ValueArray<int> c_histogram( tf_number * nbins );// color histogram
    
    // 2023/07/31 add by shimomura
    SuperClass::m_c_histogram = vismodule::ValueArray<int> (tf_number * nbins);
    SuperClass::m_o_histogram = vismodule::ValueArray<int> (tf_number * nbins);
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
    //min max
    vismodule::ValueArray<float> O_min( tf_number );// 4 calculate
    vismodule::ValueArray<float> O_max( tf_number );
    vismodule::ValueArray<float> C_min( tf_number );
    vismodule::ValueArray<float> C_max( tf_number );

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
    std::vector< std::vector<vismodule::TransferFunction> > th_tf;

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
//        std::cout << "nthreads = " << nthreads <<std::endl;

       vismodule::MersenneTwister MT( thid + mpi_rank * nthreads );

        std::vector<float> th_vertex_coords;
        std::vector<Byte>  th_vertex_colors;
        std::vector<float> th_vertex_normals;


        //ｿｿｿｿｿｿｿｿｿ
        std::vector<float> o_scalars( tf_number );//vertex opacity
        std::vector<float> c_scalars( tf_number );//vertex color
        vismodule::ValueArray<int> th_o_histogram( tf_number * nbins );// opacity
        vismodule::ValueArray<int> th_c_histogram( tf_number * nbins );// color

        if( parameter_file_opened )
        {
            th_o_histogram.fill(0x00);
            th_c_histogram.fill(0x00);
        }

        //ｿｿｿｿｿ
        vismodule::ValueArray<float> th_O_min( tf_number );//ｿｿｿｿｿｿｿｿｿｿｿ
        vismodule::ValueArray<float> th_O_max( tf_number );
        vismodule::ValueArray<float> th_C_min( tf_number );
        vismodule::ValueArray<float> th_C_max( tf_number );

        if( parameter_file_opened )
        {
            for ( int i = 0; i < tf_number; i++ ) //ｿｿｿ
            {
                th_O_min[ i ] =  FLT_MAX;
                th_O_max[ i ] = -FLT_MAX;
                th_C_min[ i ] =  FLT_MAX;
                th_C_max[ i ] = -FLT_MAX;
            }
        }

        // -----------------------------------
        //ｿｿｿｿｿ
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
        // -----------------------------------
        calculateMinmax_unstruct(cell_index, ncells, local_center_array,global_center_array,
               nvariables, thid, tf_number,
               interp, th_tfs, 
               o_scalars_array, c_scalars_array,
               th_O_min, th_O_max, th_C_min, th_C_max,  
               O_min, O_max, C_min, C_max ); 

        for( int i = 0; i < tf_number; i++ )
        {
            o_min[i] = O_min[i] ;
            o_max[i] = O_max[i] ;
            c_min[i] = C_min[i] ;
            c_max[i] = C_max[i] ;
        }

        calculateHistogram_unstruct(
        cell_index, ncells, local_center_array, global_center_array,
        nvariables, thid, tf_number,
        interp, th_tfs, 
        o_scalars_array, c_scalars_array,
        o_min, o_max, c_min, c_max, 
        th_o_histogram,th_c_histogram );

#pragma omp critical
        {
            for( int n = 0; n < tf_number * nbins; n++ )
            {
                m_o_histogram[n] += th_o_histogram[n];
                m_c_histogram[n] += th_c_histogram[n];
            }
        }

    } //#pragma omp parallel

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

    for(int i=0; i<max_threads; i++)
    {
        delete th_tfs[i];
    }
    delete[] th_tfs;
    for ( int i = 0; i < max_threads; i++ )
    {
        for ( int j = 0; j < nvariables; j++ )
        {
             delete interp[i][j];
        }
    }

    //TIMER_END( 290 );

    SuperClass::setSize( 1.0f );
    //TIMER_END( 280 );
    double end = GetTime();
    printf( " \n  CPU:generate_histogram: %lf ms\n", end - start );
}

const size_t CellByCellHistogram::calculate_number_of_particles(
    const float density,
    const float volume_of_cell,
    vismodule::MersenneTwister* MT ) 
{
    const float N = density * volume_of_cell;
    const float R = MT->rand();

    size_t n = static_cast<size_t>( N ); 
    if ( N - n > R )
    {    
        ++n; 
    }    

    return ( n ); 
}


/*===========================================================================*/
/**
 *  @brief  Calculate number of particles.
 *  @param  density [in] density value
 *  @param  volume_of_cell [in] volume of cell
 *  @return number of particles
 */
/*===========================================================================*/
const size_t CellByCellHistogram::calculate_number_of_particles(
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
void CellByCellHistogram::calculate_histogram( vismodule::ValueArray<int>&   th_o_histogram,
                          vismodule::ValueArray<int>&   th_c_histogram,
                          vismodule::ValueArray<float>& th_O_min,
                          vismodule::ValueArray<float>& th_O_max,
                          vismodule::ValueArray<float>& th_C_min,
                          vismodule::ValueArray<float>& th_C_max,
                          // ｿｿｿｿoutput, ｿｿinput
                          const int nbins, // TFSｿｿｿｿｿｿｿｿｿｿｿ
                          const vismodule::ValueArray<float>& o_min,
                          const vismodule::ValueArray<float>& o_max,
                          const vismodule::ValueArray<float>& c_min,
                          const vismodule::ValueArray<float>& c_max,
                          //const float o_scalars[][SIMDW], // 蜷域・蛟､
                          //const float c_scalars[][SIMDW],
                          float** o_scalars, // åæå¤
                          float** c_scalars,
                          const int tf_number ,
                          const int ncells )
{
    for( int i = 0; i < tf_number; i++ )
    {
        for( int I = 0; I < ncells; I++ )
        {
            //ｿｿｿｿｿｿｿｿｿｿｿ
            float h = (o_scalars[i][I] - o_min[i])/( o_max[i] - o_min[i] )*nbins;
            int H = (int)h;
            if( 0 <= H && H <= nbins )
            {
                if( H == nbins ) H--;
                th_o_histogram[ H + nbins*i]++;
            }

            //ｿｿｿｿｿｿｿｿ
            h = (c_scalars[i][I] - c_min[i])/( c_max[i] - c_min[i] )*nbins;
            H = (int)h;
            if( 0 <= H && H <= nbins )
            {
                if( H == nbins ) H--;
                th_c_histogram[ H + nbins*i]++;
            }

//            //ｿｿｿｿｿｿｿｿｿｿ
//            th_O_min[i] = th_O_min[i] < o_scalars[i][I] ? th_O_min[i] : o_scalars[i][I];
//            th_O_max[i] = th_O_max[i] > o_scalars[i][I] ? th_O_max[i] : o_scalars[i][I];
//            //ｿｿｿｿｿｿｿ
//            th_C_min[i] = th_C_min[i] < c_scalars[i][I] ? th_C_min[i] : c_scalars[i][I];
//            th_C_max[i] = th_C_max[i] > c_scalars[i][I] ? th_C_max[i] : c_scalars[i][I];
        }
    }
}

vismodule::Vector3f CellByCellHistogram::RandomSamplingInCube( const vismodule::Vector3f vertex, vismodule::MersenneTwister* MT  )
{
    const float x = (float)MT->rand();
    const float y = (float)MT->rand();
    const float z = (float)MT->rand();
    const vismodule::Vector3f d( x, y, z );

    return vertex + d;
}

} // end of namespace vismodule

