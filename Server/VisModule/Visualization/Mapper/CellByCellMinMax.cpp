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
#include <vismodule/Value>
#include <vismodule/CellBase>
#include <vismodule/TrilinearInterpolator> 
#include <vismodule/TetrahedralCell>
#include <vismodule/QuadraticTetrahedralCell>
#include <vismodule/HexahedralCell>
#include <vismodule/QuadraticHexahedralCell>
#include <vismodule/PrismaticCell>
#include <vismodule/PyramidalCell>
#include <vismodule/GlobalCore>

#include <vismodule/timer_simple>

#ifdef ENABLE_MPI
#include <mpi.h>
#include "mpi_controller.h"
#include <vismodule/DistributedUniformSampling>
#endif

#ifdef _OPENMP
#  include <omp.h>
#endif // _OPENMP

#include "CalculateHistogramMinmax.h"

namespace vismodule
{

/*===========================================================================*/
/**
 *  @brief  Constructs a new CellByCellMinMax class.
 */
/*===========================================================================*/

// unstruct
CellByCellMinMax::CellByCellMinMax( 
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype, 
    const std::vector<vismodule::TransferFunction>& transfer_function_array, 
    TransferFunctionSynthesizer* transfunc_synthesizer
):
m_transfer_function_array( transfer_function_array ),
m_transfer_function_synthesizer( transfunc_synthesizer )
{
    this->generate_minmax_unstruct( values, nvariables, coordinates, ncoords, connections, ncells, celltype );
}

//struct
CellByCellMinMax::CellByCellMinMax(
    domain_parameters_struct dom, 
    Type** values,  
    int nvariables, 
    const vismodule::TransferFunction& transfer_function,
    const std::vector<vismodule::TransferFunction>& transfer_function_array,
    TransferFunctionSynthesizer* transfunc_synthesizer
):
m_transfer_function_array( transfer_function_array ),
m_transfer_function_synthesizer( transfunc_synthesizer )
{
    this->generate_minmax_struct( dom, values, nvariables );
}


/*===========================================================================*/
/**
 *  @brief  Destroys the CellByCellMetropolisSampling class.
 */
/*===========================================================================*/
CellByCellMinMax::~CellByCellMinMax(){}

void CellByCellMinMax::generate_minmax_struct(
    domain_parameters_struct dom,
    Type** values,
    int nvariables
)
{
    const vismodule::Vector3ui resolution( dom.resolution[0], dom.resolution[1], dom.resolution[2] );
    std::cout << "resolution = " << resolution << std::endl;

    int tf_number      = m_transfer_function_array.size();
    float max_density  = m_transfer_function_synthesizer->getMaxDensity();
    int max_nparticles = (int)max_density + 1;

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

    // min max
    vismodule::ValueArray<float> O_min( tf_number );
    vismodule::ValueArray<float> O_max( tf_number );
    vismodule::ValueArray<float> C_min( tf_number );
    vismodule::ValueArray<float> C_max( tf_number );

    // initialize
    for ( std::size_t i = 0; i < tf_number; i++ )
    {
        O_min[i] =  FLT_MAX;
        O_max[i] = -FLT_MAX;
        C_min[i] =  FLT_MAX;
        C_max[i] = -FLT_MAX;
    }

    TransferFunctionSynthesizer** th_tfs = new TransferFunctionSynthesizer*[max_threads];

    for ( int n = 0; n < max_threads; n++ )
    {
        th_tfs[n] = new TransferFunctionSynthesizer( *m_transfer_function_synthesizer );
    }

    // 頂点解像度と格子解像度
    const int nx = resolution.x();
    const int ny = resolution.y();
    const int nz = resolution.z();
    const int nxy = nx * ny;
    const int nx_1 = nx-1;
    const int ny_1 = ny-1;
    const int nz_1 = nz-1;

    #pragma omp parallel
    {
#if _OPENMP
        int thid     = omp_get_thread_num();
#else
        int thid     = 0;
#endif

        // th_tfs[thid]->set_debug_thid(thid,max_threads);
        std::vector<std::vector<float>> o_scalar_storage( tf_number, std::vector<float>( SIMDW ) );
        std::vector<std::vector<float>> c_scalar_storage( tf_number, std::vector<float>( SIMDW ) );
        std::vector<float*> o_scalars( tf_number );
        std::vector<float*> c_scalars( tf_number );

        for (int i = 0; i < tf_number; i++)
        {
            o_scalars[i] = &o_scalar_storage[i][0];
            c_scalars[i] = &c_scalar_storage[i][0];
        }

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
                    &o_scalars[0]
                );

                th_tfs[thid]->SynthesizedColorScalars(
                    interp[thid],
                    X_l, Y_l, Z_l,
                    X_g, Y_g, Z_g,
                    &c_scalars[0]
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
            for( std::size_t i = 0; i < tf_number; i++ )
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

    for( int i = 0; i < max_threads; i++ )
    {
        delete th_tfs[i];
    }
    delete[] th_tfs;

    // delete tfs;

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

    int tf_number            = m_transfer_function_array.size();
    float max_density        = m_transfer_function_synthesizer->getMaxDensity();
    const int max_nparticles = (int)max_density + 1;

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
        default:
            {
                BaseClass::m_is_success = false;
                visModuleMessageError( "Unsupported cell type." );
                return;
            }
    }
    //    VIS_MODULE_TIMER_END( 270 );

    std::cout<<"******* getMaxDensity() = " << m_transfer_function_synthesizer->getMaxDensity() << std::endl;

    if( mpi_rank == 0 ) std::cout << "******* max_nparticles=" << max_nparticles << std::endl;

    // min max
    vismodule::ValueArray<float> O_min( tf_number );
    vismodule::ValueArray<float> O_max( tf_number );
    vismodule::ValueArray<float> C_min( tf_number );
    vismodule::ValueArray<float> C_max( tf_number );

    // initialize
    for ( std::size_t i = 0; i < tf_number; i++ )
    {
        O_min[i] =  FLT_MAX;
        O_max[i] = -FLT_MAX;
        C_min[i] =  FLT_MAX;
        C_max[i] = -FLT_MAX;
    }

    TransferFunctionSynthesizer** th_tfs = new TransferFunctionSynthesizer*[max_threads];

    for ( int n = 0; n < max_threads; n++ )
    {
        th_tfs[n] = new TransferFunctionSynthesizer( *m_transfer_function_synthesizer );
    }

#pragma omp parallel
    {
#if _OPENMP
        int thid     = omp_get_thread_num();
#else
        int thid     = 0;
#endif

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

        std::vector<float> o_scalars_array[ SIMDW ];
        std::vector<float> c_scalars_array[ SIMDW ];

        for (int i = 0; i < SIMDW; i++ )
        {
            o_scalars_array[i].resize( tf_number );
            c_scalars_array[i].resize( tf_number );
        }

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
                for( std::size_t i = 0; i < tf_number; i++ )
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
            for( std::size_t i = 0; i < tf_number; i++ )
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

    // set minmax range
    m_transfer_function_synthesizer->m_o_min.resize(tf_number);
    m_transfer_function_synthesizer->m_o_max.resize(tf_number);
    m_transfer_function_synthesizer->m_c_min.resize(tf_number);
    m_transfer_function_synthesizer->m_c_max.resize(tf_number);

    for ( std::size_t i = 0; i < tf_number; i++ )
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

    SuperClass::setSize( 1.0f );
    double end = GetTime();
    printf( " \n  CPU:generate_minmax: %lf ms\n", end - start );
}

} // end of namespace vismodule
