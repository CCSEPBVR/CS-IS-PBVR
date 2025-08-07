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
#include <vismodule/Compiler>
#ifdef VIS_MODULE_COMPILER_VC
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <vismodule/CellByCellUniformSampling>
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
//#include "SFMT/SFMT.h" 

#ifdef ENABLE_MPI
#include <mpi.h>
#include "mpi_controller.h"
#include <vismodule/DistributedUniformSampling>
#endif

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

#include <vismodule/timer_simple>

namespace Generator = vismodule::CellByCellParticleGenerator;

using FuncParser::Variable;
using FuncParser::Variables;
using FuncParser::Function;
using FuncParser::FunctionParser;


namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellUniformSampling class.
 */
/*===========================================================================*/
CellByCellUniformSampling::CellByCellUniformSampling():
    vismodule::MapperBase(),
    vismodule::PointObject(),
    m_transfer_function_synthesizer( NULL ),
    m_normal_ingredient( 0 ),
    m_camera( 0 )
{
}

// unstruct
CellByCellUniformSampling::CellByCellUniformSampling( 
    const vismodule::Camera&           camera,
    domain_parameters_unstruct dom,
    Type** values, int nvariables,
    float* coordinates, int ncoords,
    unsigned int* connections, int ncells,
    const  vismodule::VolumeObjectBase::CellType& celltype, 
    const size_t                 subpixel_level,
    const float                  sampling_step,
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
    this->setSubpixelLevel( subpixel_level );
    this->setSamplingStep( sampling_step );
    this->generate_particles_unstruct(dom, values, nvariables,
            coordinates, ncoords, connections, ncells, celltype);
}

//struct
CellByCellUniformSampling::CellByCellUniformSampling(
        const vismodule::Camera&  camera,
        domain_parameters_struct dom, 
        Type** values,  
        int nvariables, 
        const size_t                 subpixel_level,
        const float                  sampling_step,
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
    this->setSubpixelLevel( subpixel_level );
    this->setSamplingStep( sampling_step );
    //this->exec( dom, values, nvariables);
    this->generate_particles_struct(dom, values, nvariables);
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
void CellByCellUniformSampling::attachCamera( const vismodule::Camera& camera )
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
// MapperBaseクラスのvirtual 関数をオーバーライドするため空関数を宣言
CellByCellUniformSampling::SuperClass* CellByCellUniformSampling::exec( const vismodule::ObjectBase& object )
{
}
//template <>
//void CellByCellUniformSampling::generate_particles_unstruct<vismodule::Real32>( const vismodule::UnstructuredVolumeObject& volume );
CellByCellUniformSampling::SuperClass* CellByCellUniformSampling::generate_particles_struct( domain_parameters_struct dom, Type** values, int nvariables)
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

    vismodule::TrilinearInterpolator** interp_opacity  = new vismodule::TrilinearInterpolator*[max_threads] ;

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

//    const vismodule::CoordSynthesizerStrings* pCrdSynthStr = volume.getCoordSynthesizerStrings();
    CoordSynthesizerStrings css;
    if ( m_coord_synthesizer_strings ) 
    {
        css = *m_coord_synthesizer_strings;
    }

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

        const vismodule::Vector3f cell_length( (max_vec.x() - min_vec.x() )/ nx_1,
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
                int ncells = ((J+1) * SIMDW > nvertices) ? nvertices - J*SIMDW  : SIMDW ;
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
                                     tf_number, ncells );
//                timed_section_end(td_CalculateHistogram,thid);
            }
        } // end of Histogram

        //-----------------------------------------//
        //--------------ｿｿｿｿｿｿｿｿｿ------------//
        //------------------------------------------//
        //ｿｿｿｿｿｿｿｿｿｿｿｿ
        {
            // Marge x-y-z loop
            const int nvertices = nx * ny * nz;
            // "+ 1" means remained loop
            const int outer_loop = (nvertices % SIMDW == 0) ?
                nvertices / SIMDW : nvertices / SIMDW + 1;

            #pragma omp for
            for( int J=0; J<outer_loop; J++ )
            {
                float X_l[SIMDW], Y_l[SIMDW], Z_l[SIMDW];//interpｿｿｿｿｿｿ
                float X_g[SIMDW], Y_g[SIMDW], Z_g[SIMDW];//TFSｿｿｿｿｿｿ
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
        }// end of ｿｿｿｿｿｿｿｿｿｿｿｿ

#pragma omp critical
        {
            interp_opacity[thid] = new vismodule::TrilinearInterpolator( opacity_volume, resolution );
        }

        //ｿｿｿｿｿｿｿｿｿ
        {
            // Marge x-y-z loop
            const int ncells = nx_1 * ny_1 * nz_1;
            // "+ 1" means remained loop
            const int outer_loop = (ncells % SIMDW == 0) ?
                ncells / SIMDW : ncells / SIMDW + 1;

                interp_opacity[thid] -> setCellLength(1);
            #pragma omp for schedule(dynamic)
            for( int J=0; J<outer_loop; J++ )
            {
                float X_l[SIMDW], Y_l[SIMDW], Z_l[SIMDW];//interpｿｿｿｿｿｿ
                float X_g[SIMDW], Y_g[SIMDW], Z_g[SIMDW];//TFSｿｿｿｿｿｿ
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
//                    const int np = calculate_number_of_particles( density, 1, &MT ) * 0.1;
//                    timed_section_end(td_CalculateNumPar,thid);

                    const int cell_id = I + J * SIMDW;

                    nparticles[I] = cell_id < ncells ? np : 0;
//                    nparticles[I] = cell_id < ncells ? 10 : 0;
                    th_total_nparticles += nparticles[I] ;
                }

                // ｿｿｿｿｿSIMDｿｿｿｿｿ
                // ｿｿｿｿｿｿｿｿｿ
                int p_id = 0;
                float p_x_l[SIMDW], p_y_l[SIMDW], p_z_l[SIMDW];
                float p_x_g[SIMDW], p_y_g[SIMDW], p_z_g[SIMDW];
                float grad_x[SIMDW], grad_y[SIMDW], grad_z[SIMDW];
                vismodule::UInt8 red[SIMDW], green[SIMDW], blue[SIMDW];
                float particle_opacity[SIMDW];
                // the last loop "I==SIMDW" is used for occupy ramained array.
                    for(int I=0; I<SIMDW+1; I++)
                    {
                        const int cell_id = I + J * SIMDW;
                        const int k =  cell_id / nxy_1;
                        const int j = (cell_id - k * nxy_1) / nx_1;
                        const int i =  cell_id - k * nxy_1 - j * nx_1;

                        const int nparticles_I  = I<SIMDW ? nparticles[I] : 1;
                        const int zero_id = I<SIMDW ? SIMDW : p_id;
                        int nparticles_count =0; 
                       for(int p=0; p < nparticles_I; p++)
                        {
                            if (I < SIMDW)
                            {
                            const vismodule::Vector3f vertex( (float)i, (float)j, (float)k );
                            const vismodule::Vector3f coord_l( RandomSamplingInCube( vertex, &MT ) );
                            const vismodule::Vector3f coord_g(
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
                            }

                            if( p_id == SIMDW || I == SIMDW)
                            {
                                //p_id = 0;

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
#if 1                               
                                float np_x_g[ SIMDW ];
                                float np_y_g[ SIMDW ];
                                float np_z_g[ SIMDW ];
                                if ( !css.m_x_coord_synthesizer_string.empty() || 
                                     !css.m_y_coord_synthesizer_string.empty() || 
                                     !css.m_z_coord_synthesizer_string.empty()  ) 
                                {
                                    th_tfs[thid]->CalculateCoordArray( interp[thid],
                                            SIMDW,
                                            p_x_l, p_y_l, p_z_l,
                                            p_x_g, p_y_g, p_z_g,
                                            //local_coord_array,
                                            //global_coord_array,
                                            th_tf[thid],
                                            /*CoordSynthesizerStrings*/        css,
                                            np_x_g, np_y_g, np_z_g  );
                                }
                                else
                                {
                                    for( int j = 0; j < SIMDW; j++ )
                                    {
                                        np_x_g[j] = p_x_g[j];
                                        np_y_g[j] = p_y_g[j];
                                        np_z_g[j] = p_z_g[j];
                                    }
                                }
#endif

                                //SIMDｿｿｿ
                                //for( int pp=0; pp<SIMDW; pp++)
                                for( int pp=0; pp<p_id; pp++)
                                {
                                    //                                timed_section_start(td_CalculateDensity,thid);
//                                    const float density =
//                                        pp < zero_id ?
//                                        Generator::CalculateDensity( particle_opacity[pp],
//                                                sampling_volume_inverse,
//                                                max_opacity, max_density ) : 0;
                                    const float density =
                                        Generator::CalculateDensity( particle_opacity[pp],
                                                sampling_volume_inverse,
                                                max_opacity, max_density );


                                    //                                timed_section_end(td_CalculateDensity,thid);
                                        th_vertex_coords.push_back( np_x_g[pp] );
                                        th_vertex_coords.push_back( np_y_g[pp] );
                                        th_vertex_coords.push_back( np_z_g[pp] );

                                        th_vertex_colors.push_back( red  [pp] );
                                        th_vertex_colors.push_back( green[pp] );
                                        th_vertex_colors.push_back( blue [pp] );

                                        th_vertex_normals.push_back( grad_x[pp] );
                                        th_vertex_normals.push_back( grad_y[pp] );
                                        th_vertex_normals.push_back( grad_z[pp] );
                                       //                                    timed_section_end(td_VectorPush,thid);
                                } // end of for pp
                                p_id = 0;
                            } // end of if p_id
                            

                        } // end of for p
                    } // end of for I ｿｿｿｿｿｿｿｿｿ
                } // end of omp for J outer_loop
        } // end of ｿｿｿｿｿｿｿ

//        timed_section_start(td_VectorIns,thid);
        #pragma omp critical
        {
            if( parameter_file_opened )
            {
                //ｿｿｿｿｿ
                for( int i = 0; i < tf_number; i++ )
                {
                    //ｿｿｿｿ
                    O_min[i] = O_min[i] < th_O_min[i] ? O_min[i] : th_O_min[i];
                    O_max[i] = O_max[i] > th_O_max[i] ? O_max[i] : th_O_max[i];
                    //ｿ
                    C_min[i] = C_min[i] < th_C_min[i] ? C_min[i] : th_C_min[i];
                    C_max[i] = C_max[i] > th_C_max[i] ? C_max[i] : th_C_max[i];

                }

                for( int n = 0; n < tf_number * nbins; n++ )
                {
                    m_o_histogram[n] += th_o_histogram[n];
                    m_c_histogram[n] += th_c_histogram[n];
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

    SuperClass::m_coords  = vismodule::ValueArray<vismodule::Real32>( vertex_coords );
    SuperClass::m_colors  = vismodule::ValueArray<vismodule::UInt8>( vertex_colors );
    SuperClass::m_normals = vismodule::ValueArray<vismodule::Real32>( vertex_normals );

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
  return this;
}

CellByCellUniformSampling::SuperClass* CellByCellUniformSampling::generate_particles_unstruct( domain_parameters_unstruct dom,Type** values, int nvariables,
        float* coordinates, int ncoords,
        unsigned int* connections, int ncells,
        const  vismodule::VolumeObjectBase::CellType& cellType )
{
#if 1
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
//    const vismodule::CoordSynthesizerStrings* pCrdSynthStr = volume.getCoordSynthesizerStrings();
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

        //ｿｿｿｿｿｿｿｿｿ
#pragma omp for schedule( dynamic ) nowait  
        //#pragma omp for schedule( static ) nowait
        //#pragma omp for schedule( static, 1 ) nowait
        for( int cell_base = 0; cell_base < ncells; cell_base += SIMDW )
        {
            //ｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿ
            int remain = ( ncells - cell_base > SIMDW )? SIMDW: ncells - cell_base;

            /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////
            //ｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿ
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = (vismodule::UInt32)(cell_base + cell_BLK);
                local_center_array[cell_BLK] = vismodule::Vector3f ( 0.5, 0.5, 0.5 );
            }

            //ｿｿｿｿｿｿｿｿｿｿｿｿｿｿ
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
                        // 20190128 ｿｿ
                        th_O_min[i] = th_O_min[i] < o_scalars_array[cell_BLK][i] ? th_O_min[i] : o_scalars_array[cell_BLK][i];
                        th_O_max[i] = th_O_max[i] > o_scalars_array[cell_BLK][i] ? th_O_max[i] : o_scalars_array[cell_BLK][i];
                        th_C_min[i] = th_C_min[i] < c_scalars_array[cell_BLK][i] ? th_C_min[i] : c_scalars_array[cell_BLK][i];
                        th_C_max[i] = th_C_max[i] > c_scalars_array[cell_BLK][i] ? th_C_max[i] : c_scalars_array[cell_BLK][i];
                        
                    }
                }
            }

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
                nparticles_array[cell_BLK] *= m_particle_density ;
                //nparticles_array[cell_BLK] *= m_particle_density * 0.1 ;
                nparticles_num += nparticles_array[cell_BLK];
            }
            /////////////////////////////// Synthesized~ (), CalculateOpacity() ///////////////////////////////////

            /////////////////////////////// CalculateOpacity(), CalculateColor() ///////////////////////////////////
            for(int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                // ------------------------------------------------

                for( int i = 0; i < nparticles_array[cell_BLK]; i+=SIMD_BLK_SIZE )
                {
                    //ｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿ
                    int remain_BLK = ( nparticles_array[cell_BLK] - i > SIMD_BLK_SIZE )
                                                        ? SIMD_BLK_SIZE: nparticles_array[cell_BLK] - i;

                    //ｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿ
                    for( int j = 0; j < remain_BLK; j++ ) 
                    {
                        cell_index[j] = cell_base + cell_BLK;
                            local_coord_array[j] = interp[thid][0] -> randomSampling_MT( &MT);

                            //ｿｿｿｿｿｿｿｿｿｿｿｿｿｿ
                            for( int k = 0; k < nvariables; k++ )
                            {
                                interp[thid][k]->bindCell( cell_index[j] );
                            }

                            interp[thid][0]->setLocalPoint( local_coord_array[j] );
                            global_coord_array[j] = interp[thid][0]->transformLocalToGlobal( local_coord_array[j] );
                    }
 
                  
                    //ｿｿｿｿｿcell_index, local_coordｿｿｿｿ
                    int nparticles_count = 0;
                    for( int j = 0; j < remain_BLK; j++ )
                    {
                        cell_index[ nparticles_count ] = cell_index[j];
                        local_coord_array[ nparticles_count ] = local_coord_array[j];
                        global_coord_array[ nparticles_count ] = global_coord_array[j];
                        nparticles_count +=1;
//                        std::cout << "global_coord_array[ nparticles_count ]  =" << global_coord_array[ nparticles_count ]  <<std::endl;
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
                        l_plus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0.1,0,0);
                        l_minus_coord[j] = local_coord_array[j] + vismodule::Vector3f(-0.1,0,0);
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
                        l_plus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,0.1,0);
                        l_minus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,-0.1,0);
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
                        l_plus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,0,0.1);
                        l_minus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,0,-0.1);
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
                    //grad_arrayｿｿｿ
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        //JacobiMatrixｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿｿ
                        interp[thid][0]->bindCell( cell_index[j] );

                        const vismodule::Vector3f g( -dsdx_array[j], -dsdy_array[j], -dsdz_array[j] );
                        const vismodule::Matrix33f J = interp[thid][0]->JacobiMatrix();
                        float determinant = 0.0f;
                        const vismodule::Vector3f G = J.inverse( &determinant ) * g;
                        grad_array[j] = vismodule::Math::IsZero( determinant ) ? vismodule::Vector3f( 0.0f, 0.0f, 0.0f ) : G;
                    }

                    //color
                    th_tfs[thid]->CalculateColorArray( interp[thid],
                            nparticles_count,
                            local_coord_array,
                            global_coord_array,
                            th_tf[thid],
                            color_array );

                    vismodule::Vector3f new_coord_array[ SIMDW ];
                    //if ( pCrdSynthStr )
                    if ( !css.m_x_coord_synthesizer_string.empty() || !css.m_y_coord_synthesizer_string.empty() || !css.m_z_coord_synthesizer_string.empty()  ) 
                    {
                        th_tfs[thid]->CalculateCoordArray( interp[thid],
                                nparticles_count,
                                local_coord_array,
                                global_coord_array,
                                th_tf[thid],
//      /*CoordSynthesizerTokens*/        cst,
      /*CoordSynthesizerStrings*/        css,
                                new_coord_array );
                    }
                    else
                    {
                        for( int j = 0; j < nparticles_count; j++ )
                        {
                            new_coord_array[j] = global_coord_array[j];
                        }
                    }

                    //2023 shimomura 
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        vismodule::Vector3f new_coord = new_coord_array[j];
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
                //ｿｿｿｿｿ
                for( int i = 0; i < tf_number; i++ )
                {
                    //ｿｿｿｿ
                    O_min[i] = O_min[i] < th_O_min[i] ? O_min[i] : th_O_min[i];
                    O_max[i] = O_max[i] > th_O_max[i] ? O_max[i] : th_O_max[i];
                    //ｿ
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
        th_vertex_coords.clear();
        th_vertex_colors.clear();
        th_vertex_normals.clear();

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

    SuperClass::m_coords  = vismodule::ValueArray<vismodule::Real32>( vertex_coords );
    SuperClass::m_colors  = vismodule::ValueArray<vismodule::UInt8>( vertex_colors );
    SuperClass::m_normals = vismodule::ValueArray<vismodule::Real32>( vertex_normals );

    SuperClass::setSize( 1.0f );
    //TIMER_END( 280 );
    double end = GetTime();
    printf( " \n  CPU:generate_particles: %lf ms\n", end - start );
#endif
    return this;
}

const size_t CellByCellUniformSampling::calculate_number_of_particles(
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


#ifdef ENABLE_MPI
void CellByCellUniformSampling::generate_particles_gt5d(
    const vismodule::UnstructuredVolumeObject& volume )
{
    // Vertex data arrays. (output)
    std::vector<vismodule::Real32> vertex_coords;
    std::vector<vismodule::UInt8>  vertex_colors;
    std::vector<vismodule::Real32> vertex_normals;

    // Set a tetrahedral cell interpolator.
    vismodule::CellBase<vismodule::Real32>* cell = NULL;
    switch ( volume.cellType() )
    {
    case vismodule::VolumeObjectBase::Tetrahedra:
    {
        cell = new vismodule::TetrahedralCell<vismodule::Real32>( volume );
        break;
    }
    case vismodule::VolumeObjectBase::QuadraticTetrahedra:
    {
        cell = new vismodule::QuadraticTetrahedralCell<vismodule::Real32>( volume );
        break;
    }
    case vismodule::VolumeObjectBase::Hexahedra:
    {
        cell = new vismodule::HexahedralCell<vismodule::Real32>( volume );
        break;
    }
    case vismodule::VolumeObjectBase::QuadraticHexahedra:
    {
        cell = new vismodule::QuadraticHexahedralCell<vismodule::Real32>( volume );
        break;
    }
    case vismodule::VolumeObjectBase::Prism:
    {
        cell = new vismodule::PrismaticCell<vismodule::Real32>( volume );
        break;
    }
    case vismodule::VolumeObjectBase::Pyramid:
    {
        cell = new vismodule::PyramidalCell<vismodule::Real32>( volume );
        break;
    }
    default:
    {
        BaseClass::m_is_success = false;
        visModuleMessageError( "Unsupported cell type." );
        return;
    }
    }

    double start = GetTime();

    const vismodule::ColorMap color_map( BaseClass::transferFunction().colorMap() );

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

/*===========================================================================*/
/**
 *  @brief  Calculate density value.
 *  @param  scalar [in] scalar value
 *  @return density value
 */
/*===========================================================================*/
const float CellByCellUniformSampling::calculate_density( const float scalar )
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
    index1 = vismodule::Math::Clamp<size_t>( index1, 0, BaseClass::transferFunction().resolution() - 1 );
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
const size_t CellByCellUniformSampling::calculate_number_of_particles(
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
const float CellByCellUniformSampling::calculate_maximum_density( const float scalar0, const float scalar1 )
{
    if ( scalar0 > scalar1 )
    {
        visModuleMessageError( "undefined use of calculate_maximum_density." );
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
    index0 = vismodule::Math::Clamp<size_t>( index0, 0, BaseClass::transferFunction().resolution() - 1 );

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

void CellByCellUniformSampling::calculate_histogram( vismodule::ValueArray<int>&   th_o_histogram,
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
                          const int tf_number,
                          const int ncells  )
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

            //ｿｿｿｿｿｿｿｿｿｿ
            th_O_min[i] = th_O_min[i] < o_scalars[i][I] ? th_O_min[i] : o_scalars[i][I];
            th_O_max[i] = th_O_max[i] > o_scalars[i][I] ? th_O_max[i] : o_scalars[i][I];
            //ｿｿｿｿｿｿｿ
            th_C_min[i] = th_C_min[i] < c_scalars[i][I] ? th_C_min[i] : c_scalars[i][I];
            th_C_max[i] = th_C_max[i] > c_scalars[i][I] ? th_C_max[i] : c_scalars[i][I];
        }
    }
}

vismodule::Vector3f CellByCellUniformSampling::RandomSamplingInCube( const vismodule::Vector3f vertex, vismodule::MersenneTwister* MT  )
{
    const float x = (float)MT->rand();
    const float y = (float)MT->rand();
    const float z = (float)MT->rand();
    const vismodule::Vector3f d( x, y, z );

    return vertex + d;
}

} // end of namespace vismodule

