#ifndef PBVR_ENSEMBLE_PARTICLE_GENERATOR_H
#define PBVR_ENSEMBLE_PARTICLE_GENERATOR_H

// アンサンブルPBVR 粒子生成(計算)の分離ファイル。
// ensemble_generate_particles() の計算本体を pbvr::GenerateEnsembleParticles に切り出す。
// タイマー機構(ENABLE_ENSEMBLE_TIMER)は wrapper と共有するため公開する(指示書 §3.4-a)。

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cfloat>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <vismodule/VolumeObjectBase>
#include <vismodule/ParticleProperty>

#include "EnsembleCellHistogram.h"   // Type, pbvr::EnsembleStatisticRange, CPU_VER/MPI handling

#ifdef ENABLE_ENSEMBLE_TIMER
#include <vismodule/Timer>
#endif

#ifndef CPU_VER
#include <mpi.h>
#endif

typedef unsigned char Byte;

namespace pbvr
{

#ifdef ENABLE_ENSEMBLE_TIMER
enum EnsembleTimerSection
{
    EnsembleTimerTotal = 0,
    EnsembleTimerSetParameterPath,
    EnsembleTimerReadParameterFile,
    EnsembleTimerInitTransferFunctions,
    EnsembleTimerCreateCells,
    EnsembleTimerSamplingPrepare,
    EnsembleTimerOmpUniformSampling,
    EnsembleTimerUniformThreadSetup,
    EnsembleTimerUniformCellIndexSetup,
    EnsembleTimerUniformBindCellArray,
    EnsembleTimerUniformVolumeCalculation,
    EnsembleTimerUniformParticleCountCalculation,
    EnsembleTimerUniformParticleSamplingLoop,
    EnsembleTimerUniformLocalCoordGeneration,
    EnsembleTimerUniformFlushPrepare,
    EnsembleTimerUniformCalculateScalars,
    EnsembleTimerUniformCalcScalarGrad,
    EnsembleTimerUniformQGradSetup,
    EnsembleTimerUniformTfScalarEval,
    EnsembleTimerUniformChainRuleDfdq,
    EnsembleTimerUniformNormalNormalize,
    EnsembleTimerUniformStoreParticleData,
    EnsembleTimerThreadParticleMerge,
    EnsembleTimerMpiShiftExchange,
    EnsembleTimerMpiShiftSizeExchange,
    EnsembleTimerMpiShiftAllocRecvBuffer,
    EnsembleTimerMpiShiftPayloadAll,
    EnsembleTimerMpiShiftPayloadCellids,
    EnsembleTimerMpiShiftPayloadScalars,
    EnsembleTimerMpiShiftPayloadCoords,
    EnsembleTimerMpiShiftPayloadNormals,
    EnsembleTimerMpiShiftPayloadSq,
    EnsembleTimerMpiShiftPayloadTmp,
    EnsembleTimerOmpShiftInterpolation,
    EnsembleTimerShiftCalculateScalars,
    EnsembleTimerShiftCalcScalarGrad,
    EnsembleTimerShiftQGradSetup,
    EnsembleTimerShiftTfScalarEval,
    EnsembleTimerShiftChainRuleDfdq,
    EnsembleTimerShiftNormalNormalize,
    EnsembleTimerShiftRecoverRecv,
    EnsembleTimerShiftFlushPrepare,
    EnsembleTimerShiftStoreAccumulate,
    EnsembleTimerStatAverageVariance,
    EnsembleTimerStatHistogram,
    EnsembleTimerOmpRejection,
    EnsembleTimerRejectionThreadMerge,
    EnsembleTimerCleanupTfs,
    EnsembleTimerOutputCoordMinmax,
    EnsembleTimerOutputParticlesAve,
    EnsembleTimerOutputParticlesVar,
    EnsembleTimerOutputParticlesCov,
    EnsembleTimerOutputHistory,
    EnsembleTimerCleanupCells,
    EnsembleTimerWriteTfFile,
    EnsembleTimerAsyncIoWait,
    EnsembleTimerFinalBarrierState,
    EnsembleTimerSectionCount
};

struct EnsembleTimerSectionDef
{
    const char* parent;
    const char* section;
    int level;
};

inline const EnsembleTimerSectionDef EnsembleTimerSections[EnsembleTimerSectionCount] =
{
    { "root", "ensemble_generate_particles_total", 0 },
    { "ensemble_generate_particles_total", "set_parameter_path", 1 },
    { "ensemble_generate_particles_total", "read_parameter_file", 1 },
    { "ensemble_generate_particles_total", "init_transfer_functions", 1 },
    { "ensemble_generate_particles_total", "create_cells", 1 },
    { "ensemble_generate_particles_total", "sampling_prepare", 1 },
    { "ensemble_generate_particles_total", "omp_uniform_sampling", 1 },
    { "omp_uniform_sampling", "uniform_thread_setup", 2 },
    { "omp_uniform_sampling", "uniform_cell_index_setup", 2 },
    { "omp_uniform_sampling", "uniform_bind_cell_array", 2 },
    { "omp_uniform_sampling", "uniform_volume_calculation", 2 },
    { "omp_uniform_sampling", "uniform_particle_count_calculation", 2 },
    { "omp_uniform_sampling", "uniform_particle_sampling_loop", 2 },
    { "uniform_particle_sampling_loop", "uniform_local_coord_generation", 3 },
    { "uniform_particle_sampling_loop", "uniform_flush_prepare", 3 },
    { "uniform_particle_sampling_loop", "uniform_calculate_scalars_array", 3 },
    { "uniform_calculate_scalars_array", "uniform_calc_scalar_grad", 4 },
    { "uniform_calculate_scalars_array", "uniform_q_values_grad_q_setup", 4 },
    { "uniform_calculate_scalars_array", "uniform_tf_scalar_eval", 4 },
    { "uniform_calculate_scalars_array", "uniform_chain_rule_dfdq", 4 },
    { "uniform_calculate_scalars_array", "uniform_normal_normalize", 4 },
    { "uniform_particle_sampling_loop", "uniform_store_particle_data", 3 },
    { "omp_uniform_sampling", "thread_particle_merge", 2 },
    { "ensemble_generate_particles_total", "mpi_shift_exchange", 1 },
    { "mpi_shift_exchange", "mpi_shift_size_exchange", 2 },
    { "mpi_shift_exchange", "mpi_shift_alloc_recv_buffer", 2 },
    { "mpi_shift_exchange", "mpi_shift_payload_all", 2 },
    { "mpi_shift_exchange", "mpi_shift_payload_cellids", 2 },
    { "mpi_shift_exchange", "mpi_shift_payload_scalars", 2 },
    { "mpi_shift_exchange", "mpi_shift_payload_coords", 2 },
    { "mpi_shift_exchange", "mpi_shift_payload_normals", 2 },
    { "mpi_shift_exchange", "mpi_shift_payload_sq", 2 },
    { "mpi_shift_exchange", "mpi_shift_payload_tmp", 2 },
    { "mpi_shift_exchange", "omp_shift_interpolation", 2 },
    { "omp_shift_interpolation", "shift_calculate_scalars_array", 3 },
    { "shift_calculate_scalars_array", "shift_calc_scalar_grad", 4 },
    { "shift_calculate_scalars_array", "shift_q_values_grad_q_setup", 4 },
    { "shift_calculate_scalars_array", "shift_tf_scalar_eval", 4 },
    { "shift_calculate_scalars_array", "shift_chain_rule_dfdq", 4 },
    { "shift_calculate_scalars_array", "shift_normal_normalize", 4 },
    { "omp_shift_interpolation", "shift_recover_recv", 3 },
    { "omp_shift_interpolation", "shift_flush_prepare", 3 },
    { "omp_shift_interpolation", "shift_store_accumulate", 3 },
    { "ensemble_generate_particles_total", "stat_average_variance", 1 },
    { "ensemble_generate_particles_total", "stat_histogram", 1 },
    { "ensemble_generate_particles_total", "omp_rejection", 1 },
    { "omp_rejection", "rejection_thread_merge", 2 },
    { "ensemble_generate_particles_total", "cleanup_tfs", 1 },
    { "ensemble_generate_particles_total", "output_coord_minmax", 1 },
    { "ensemble_generate_particles_total", "output_particles_ave", 1 },
    { "ensemble_generate_particles_total", "output_particles_var", 1 },
    { "ensemble_generate_particles_total", "output_particles_cov", 1 },
    { "ensemble_generate_particles_total", "output_history", 1 },
    { "ensemble_generate_particles_total", "cleanup_cells", 1 },
    { "ensemble_generate_particles_total", "write_tf_file", 1 },
    { "ensemble_generate_particles_total", "async_io_wait", 1 },
    { "ensemble_generate_particles_total", "final_barrier_state", 1 }
};

inline bool EnsembleTimerVerbose()
{
    const char* value = std::getenv( "PBVR_ENSEMBLE_TIMER_VERBOSE" );
    if ( value == NULL ) return false;
    return std::strcmp( value, "0" ) != 0 &&
           std::strcmp( value, "false" ) != 0 &&
           std::strcmp( value, "FALSE" ) != 0;
}

class EnsembleTimerCollector
{
private:
    int m_step;
    int m_max_threads;
    std::vector<double> m_seconds;
    std::vector<std::vector<double> > m_thread_seconds;
    unsigned long long m_uniform_particle_count;
    unsigned long long m_average_particle_count;
    unsigned long long m_variance_particle_count;
    unsigned long long m_coefficient_particle_count;

public:
    EnsembleTimerCollector( const int step, const int max_threads ):
        m_step( step ),
        m_max_threads( max_threads ),
        m_seconds( EnsembleTimerSectionCount, 0.0 ),
        m_thread_seconds( EnsembleTimerSectionCount, std::vector<double>( max_threads, 0.0 ) ),
        m_uniform_particle_count( 0 ),
        m_average_particle_count( 0 ),
        m_variance_particle_count( 0 ),
        m_coefficient_particle_count( 0 )
    {
    }

    void add( const EnsembleTimerSection section, const double seconds )
    {
        m_seconds[section] += seconds;
    }

    void addThread( const EnsembleTimerSection section, const int thread, const double seconds )
    {
        if ( thread < 0 || thread >= m_max_threads ) return;
        m_thread_seconds[section][thread] += seconds;
    }

    void setUniformParticleCount( const unsigned long long count )
    {
        m_uniform_particle_count = count;
    }

    void setStatisticParticleCounts(
        const unsigned long long average_count,
        const unsigned long long variance_count,
        const unsigned long long coefficient_count )
    {
        m_average_particle_count = average_count;
        m_variance_particle_count = variance_count;
        m_coefficient_particle_count = coefficient_count;
    }

    void printCsv( const int mpi_rank, const int mpi_size ) const
    {
        static bool summary_header_written = false;
        std::ofstream summary_ofs;
        std::ostream* summary_out = &std::cout;
        if ( mpi_rank == 0 )
        {
            summary_ofs.open( "ensemble_timer_summary.csv", std::ios::app );
            if ( summary_ofs.is_open() ) summary_out = &summary_ofs;
            if ( !summary_header_written )
            {
                *summary_out
                    << "step,scope,parent_section,section,level,mpi_avg_sec,mpi_max_sec,mpi_min_sec,"
                    << "thread_avg_sec,thread_max_sec,thread_min_sec,"
                    << "uniform_particle_count,"
                    << "ave_particle_count,var_particle_count,cov_particle_count" << std::endl;
                summary_header_written = true;
            }
        }

        unsigned long long uniform_particle_count = m_uniform_particle_count;
        unsigned long long average_particle_count = m_average_particle_count;
        unsigned long long variance_particle_count = m_variance_particle_count;
        unsigned long long coefficient_particle_count = m_coefficient_particle_count;
#ifndef CPU_VER
        MPI_Reduce(
            &m_uniform_particle_count,
            &uniform_particle_count,
            1,
            MPI_UNSIGNED_LONG_LONG,
            MPI_SUM,
            0,
            MPI_COMM_WORLD );
        MPI_Reduce(
            &m_average_particle_count,
            &average_particle_count,
            1,
            MPI_UNSIGNED_LONG_LONG,
            MPI_SUM,
            0,
            MPI_COMM_WORLD );
        MPI_Reduce(
            &m_variance_particle_count,
            &variance_particle_count,
            1,
            MPI_UNSIGNED_LONG_LONG,
            MPI_SUM,
            0,
            MPI_COMM_WORLD );
        MPI_Reduce(
            &m_coefficient_particle_count,
            &coefficient_particle_count,
            1,
            MPI_UNSIGNED_LONG_LONG,
            MPI_SUM,
            0,
            MPI_COMM_WORLD );
#endif

        const bool verbose = EnsembleTimerVerbose();
        for ( int i = 0; i < EnsembleTimerSectionCount; i++ )
        {
            const double local = m_seconds[i];
            const int local_active = local > 0.0 ? 1 : 0;
            const double local_min_candidate = local_active ? local : DBL_MAX;
            double sum = local;
            double max_value = local;
            double min_value = local_min_candidate;
            int active = local_active;

#ifndef CPU_VER
            MPI_Reduce( &local, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD );
            MPI_Reduce( &local, &max_value, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
            MPI_Reduce( &local_min_candidate, &min_value, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD );
            MPI_Reduce( &local_active, &active, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
#endif

            double thread_sum = 0.0;
            double thread_max = 0.0;
            double thread_min = 0.0;
            int thread_count = 0;
            for ( int t = 0; t < m_max_threads; t++ )
            {
                const double value = m_thread_seconds[i][t];
                if ( value <= 0.0 ) continue;
                thread_sum += value;
                thread_max = thread_count == 0 ? value : std::max( thread_max, value );
                thread_min = thread_count == 0 ? value : std::min( thread_min, value );
                thread_count++;
            }
            const double thread_avg = thread_count > 0 ? thread_sum / static_cast<double>( thread_count ) : 0.0;
            const int local_thread_active = thread_count > 0 ? 1 : 0;
            const double thread_min_candidate = local_thread_active ? thread_min : DBL_MAX;
            double thread_avg_sum = thread_avg;
            double thread_max_global = thread_max;
            double thread_min_global = thread_min_candidate;
            int thread_active = local_thread_active;

#ifndef CPU_VER
            MPI_Reduce( &thread_avg, &thread_avg_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD );
            MPI_Reduce( &thread_max, &thread_max_global, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
            MPI_Reduce( &thread_min_candidate, &thread_min_global, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD );
            MPI_Reduce( &local_thread_active, &thread_active, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
#endif

            if ( mpi_rank == 0 && ( active > 0 || thread_active > 0 ) )
            {
                const double mpi_avg = sum / static_cast<double>( mpi_size );
                const double thread_avg_global = thread_active > 0 ? thread_avg_sum / static_cast<double>( thread_active ) : 0.0;
                *summary_out << m_step << ",ensemble_generate_particles,"
                             << EnsembleTimerSections[i].parent << ","
                             << EnsembleTimerSections[i].section << ","
                             << EnsembleTimerSections[i].level << ","
                             << std::setprecision( 9 ) << mpi_avg << ","
                             << max_value << ","
                             << min_value << ","
                             << thread_avg_global << ","
                             << thread_max_global << ","
                             << thread_min_global << ","
                             << uniform_particle_count << ","
                             << average_particle_count << ","
                             << variance_particle_count << ","
                             << coefficient_particle_count << std::endl;
            }

            if ( verbose )
            {
                std::cout << m_step << ",ensemble_generate_particles_rank,"
                          << mpi_rank << ","
                          << EnsembleTimerSections[i].section << ","
                          << local << std::endl;
            }
        }
    }
};

class EnsembleTimerScope
{
private:
    EnsembleTimerCollector* m_collector;
    EnsembleTimerSection m_section;
    vismodule::Timer m_timer;

public:
    EnsembleTimerScope( EnsembleTimerCollector* collector, const EnsembleTimerSection section ):
        m_collector( collector ),
        m_section( section )
    {
        m_timer.start();
    }

    ~EnsembleTimerScope()
    {
        m_timer.stop();
        m_collector->add( m_section, m_timer.sec() );
    }
};
#endif

// ave/var/cov 各1組の粒子データ(計算関数が生成し、出力関数が受け取る)
struct EnsembleParticleArrays
{
    std::vector<float> coords;
    std::vector<Byte>  colors;
    std::vector<float> normals;
};

// アンサンブル計算処理用関数:
//   サンプリング〜統計量算出〜棄却サンプリングまでを実行し ave/var/cov の粒子列と統計レンジを返す。
//   particle_property は伝達関数の範囲・synthesizer の min/max が更新されて返る。
// (Step1: 宣言のみ。本体は step2 で移設。シグネチャ細部は実装時に調整可)
bool GenerateEnsembleParticles(
    const int num_ensemble,
    ParticleProperty& particle_property,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    EnsembleParticleArrays& average,
    EnsembleParticleArrays& variance,
    EnsembleParticleArrays& coefficient,
    EnsembleStatisticRange& average_range,
    EnsembleStatisticRange& variance_range,
    EnsembleStatisticRange& co_variation_range
#ifdef ENABLE_ENSEMBLE_TIMER
    , EnsembleTimerCollector* timer
#endif
);

} // namespace pbvr

#endif // PBVR_ENSEMBLE_PARTICLE_GENERATOR_H
