#include "kvs_wrapper_common.h"
#include "kvs_wrapper.h"

#include <cstdio>
#include <cfloat>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <climits>

#include <vismodule/ValueArray>
#include <vismodule/PointObject>
#include <vismodule/KVSMLObjectPoint>
#include <vismodule/PointExporter>
#include <vismodule/RGBColor>
#include <vismodule/MersenneTwister>
#include <vismodule/CellBase>
#include <vismodule/TetrahedralCell>
#include <vismodule/HexahedralCell>
#include <vismodule/QuadraticTetrahedralCell>
#include <vismodule/QuadraticHexahedralCell>
#include <vismodule/PrismaticCell>
#include <vismodule/PyramidalCell>
#include <vismodule/CellByCellParticleGenerator>
#include <vismodule/TransferFunctionSynthesizer>
#include <vismodule/StructuredVolumeObject>
#include <vismodule/UnstructuredVolumeObject>
#ifdef ENABLE_ENSEMBLE_TIMER
#include <vismodule/Timer>
#endif

#ifndef CPU_VER
#include <mpi.h>
#endif

#include <vismodule/ParticleProperty>

#include <vismodule/ParameterFileReader>
#include <vismodule/ParameterFileWriter>

// Generate
#include <vismodule/GenerateParticle>
#include <vismodule/PointObjectGenerator>
#include <vismodule/GlyphSeedGenerator>
#include <vismodule/PlotOverLineGenerator>
#include <vismodule/PlotOverTimeGenerator>

#ifdef EXTEND_FILE_FORMAT
#include <vismodule/UnstructuredVolumeImporter>
#include <vtkSmartPointerBase.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>
#include <kvs/UnstructuredVolumeObject>
#include <kvs/extendedfileformat/VtkUnstructuredFileFormat>
#include <kvs/extendedfileformat/VtkXmlUnstructuredGrid>
#include <kvs/extendedfileformat/VtkImporter>
#endif

#include "ChainRuleNormal.h"
#include "EnsembleCellHistogram.h"

namespace Generator = vismodule::CellByCellParticleGenerator;

static bool is_initial_step = true;
static std::size_t start_time_step = 0;

static void CollectParticleMinMax(
    ParticleProperty& particle_property,
    const domain_parameters_unstruct& dom,
    Type** values,
    const int nvariables,
    float* coordinates,
    const int ncoords,
    unsigned int* connections,
    const int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    const int tf_number,
    float* tmp_max,
    float* tmp_min
)
{
    vismodule::PointObjectGenerator point_object_generator;
    const char tmp_sampling_method = particle_property.m_sampling_method;
    particle_property.m_sampling_method = 'x';

    vismodule::PointObject* point_object = point_object_generator.GenerateParticleUnstruct(
        particle_property, dom, values, nvariables, coordinates,
        ncoords, connections, ncells, celltype, ServerMode::IS
    );

    MakeParticleMinMax( particle_property.m_transfunc_synthesizer, tf_number, tmp_max, tmp_min );

    delete point_object;
    particle_property.m_sampling_method = tmp_sampling_method;
}

static void CollectParticleHistogram(
    ParticleProperty& particle_property,
    const domain_parameters_unstruct& dom,
    Type** values,
    const int nvariables,
    float* coordinates,
    const int ncoords,
    unsigned int* connections,
    const int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    const int tf_number,
    vismodule::UInt64* tmp_c_bins,
    vismodule::UInt64* tmp_o_bins
)
{
    vismodule::PointObjectGenerator point_object_generator;
    const char tmp_sampling_method = particle_property.m_sampling_method;
    particle_property.m_sampling_method = 'h';

    vismodule::PointObject* point_object = point_object_generator.GenerateParticleUnstruct(
        particle_property, dom, values, nvariables, coordinates,
        ncoords, connections, ncells, celltype, ServerMode::IS
    );

    MakeHistgram( point_object, tf_number, tmp_c_bins, tmp_o_bins );

    delete point_object;
    particle_property.m_sampling_method = tmp_sampling_method;
}

static void GenerateParticleObject(
    ParticleProperty& particle_property,
    const domain_parameters_unstruct& dom,
    Type** values,
    const int nvariables,
    float* coordinates,
    const int ncoords,
    unsigned int* connections,
    const int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    std::vector<float>& particle_coords,
    std::vector<Byte>& particle_colors,
    std::vector<float>& particle_normals
)
{
    vismodule::PointObjectGenerator point_object_generator;
    vismodule::PointObject* point_object = point_object_generator.GenerateParticleUnstruct(
        particle_property, dom, values, nvariables, coordinates,
        ncoords, connections, ncells, celltype, ServerMode::IS
    );

    MakeParticle( point_object, particle_coords, particle_colors, particle_normals );

    delete point_object;
}

namespace
{

using pbvr::EnsembleStatisticRange;

bool UseStableEnsembleCommunication()
{
    const char* value = std::getenv( "PBVR_ENSEMBLE_STABLE_COMM" );
    if ( value == NULL ) return false;
    return std::strcmp( value, "0" ) != 0 &&
           std::strcmp( value, "false" ) != 0 &&
           std::strcmp( value, "FALSE" ) != 0;
}

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
    EnsembleTimerUniformChainRuleGrad,
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
    EnsembleTimerShiftChainRuleGrad,
    EnsembleTimerShiftCalcScalarGrad,
    EnsembleTimerShiftQGradSetup,
    EnsembleTimerShiftTfScalarEval,
    EnsembleTimerShiftChainRuleDfdq,
    EnsembleTimerShiftNormalNormalize,
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

const EnsembleTimerSectionDef EnsembleTimerSections[EnsembleTimerSectionCount] =
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
    { "uniform_particle_sampling_loop", "uniform_chain_rule_grad", 3 },
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
    { "omp_shift_interpolation", "shift_chain_rule_grad", 3 },
    { "shift_calculate_scalars_array", "shift_calc_scalar_grad", 4 },
    { "shift_calculate_scalars_array", "shift_q_values_grad_q_setup", 4 },
    { "shift_calculate_scalars_array", "shift_tf_scalar_eval", 4 },
    { "shift_calculate_scalars_array", "shift_chain_rule_dfdq", 4 },
    { "shift_calculate_scalars_array", "shift_normal_normalize", 4 },
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

bool EnsembleTimerVerbose()
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

std::string EnsembleParticleFilePrefix(
    const std::string& particleFilePrefix,
    const std::string& statisticPrefix
)
{
    if ( particleFilePrefix.size() >= 2 &&
         particleFilePrefix.compare( particleFilePrefix.size() - 2, 2, "t_" ) == 0 )
    {
        return particleFilePrefix.substr( 0, particleFilePrefix.size() - 2 ) + statisticPrefix;
    }

    return particleFilePrefix + statisticPrefix;
}

void OutputEnsembleStatisticParticles(
    ParticleProperty& particle_property,
    const MultiVolumePropertyList& mvpl,
    const int time_step,
    const std::string& particleFilePrefix,
    const std::vector<float>& coords,
    const std::vector<Byte>& colors,
    const std::vector<float>& normals
)
{
    static bool first_step = true;
    static int count;
    static int num_nodes;

#ifndef CPU_VER
    static MPI_Comm new_comm;
#endif

    if ( first_step )
    {
#ifndef CPU_VER
        int numprocs, myrank;
        int resultlen;
        char procname[MPI_MAX_PROCESSOR_NAME];
        char* procname_bak;
        char* procname_g;
        char* procname_p;

        MPI_Comm_size( MPI_COMM_WORLD, &numprocs );
        MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
        MPI_Get_processor_name( procname, &resultlen );
        procname_g = new char[MPI_MAX_PROCESSOR_NAME * numprocs];
        MPI_Allgather( procname, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                       procname_g, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                       MPI_COMM_WORLD );

        int color;
        count = 1;
        for ( color = 0; color < numprocs; color++ )
        {
            procname_p = procname_g + MPI_MAX_PROCESSOR_NAME * color;
            if ( color > 0 )
            {
                procname_bak = procname_p - MPI_MAX_PROCESSOR_NAME;
                if ( strcmp( procname_p, procname_bak ) != 0 ) count++;
            }
            if ( strcmp( procname_p, procname ) == 0 ) break;
        }

        delete[] procname_g;

        MPI_Comm_split( MPI_COMM_WORLD, color, myrank, &new_comm );

        int split_numprocs;
        MPI_Comm_size( new_comm, &split_numprocs );
        num_nodes = numprocs / split_numprocs;
        if ( numprocs % split_numprocs > 0 ) num_nodes++;
#else
        count = 1;
        num_nodes = 1;
#endif
        first_step = false;
    }

    std::stringstream ss;
    ss << std::setfill( '0' ) << std::setw( 5 ) << time_step;
    ss << "_";
    ss << std::setfill( '0' ) << std::setw( 7 ) << count;
    ss << "_";
    ss << std::setfill( '0' ) << std::setw( 7 ) << num_nodes;
    ss << ".kvsml";
    const std::string particleFilePath = particleFilePrefix + ss.str();

    int particle_size = coords.size();
    int* recvcounts;
    int* displs;
    int new_rank;
    int new_number_of_process;

#ifndef CPU_VER
    MPI_Comm_rank( new_comm, &new_rank );
    MPI_Comm_size( new_comm, &new_number_of_process );
#else
    new_rank = 0;
    new_number_of_process = 1;
#endif

    displs = new int[new_number_of_process];
    recvcounts = new int[new_number_of_process];

#ifndef CPU_VER
    MPI_Allgather( &particle_size, 1, MPI_INT,
                   recvcounts, 1, MPI_INT,
                   new_comm );
#else
    recvcounts[0] = particle_size;
#endif

    displs[0] = 0;
    for ( int i = 1; i < new_number_of_process; i++ )
    {
        displs[i] = displs[i - 1] + recvcounts[i - 1];
    }

    const int gathered_size = displs[new_number_of_process - 1] + recvcounts[new_number_of_process - 1];
    vismodule::ValueArray<float> new_coords( gathered_size );
    vismodule::ValueArray<Byte> new_colors( gathered_size );
    vismodule::ValueArray<float> new_normals( gathered_size );

#ifndef CPU_VER
    vismodule::ValueArray<float> tmp_coords( coords );
    vismodule::ValueArray<Byte> tmp_colors( colors );
    vismodule::ValueArray<float> tmp_normals( normals );

    MPI_Gatherv( tmp_coords.pointer(), particle_size, MPI_FLOAT,
                 new_coords.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, new_comm );
    MPI_Gatherv( tmp_colors.pointer(), particle_size, MPI_BYTE,
                 new_colors.pointer(), recvcounts, displs, MPI_BYTE,
                 0, new_comm );
    MPI_Gatherv( tmp_normals.pointer(), particle_size, MPI_FLOAT,
                 new_normals.pointer(), recvcounts, displs, MPI_FLOAT,
                 0, new_comm );
#else
    for ( int i = 0; i < particle_size; i++ )
    {
        new_coords[i] = coords[i];
        new_colors[i] = colors[i];
        new_normals[i] = normals[i];
    }
#endif

    if ( new_rank == 0 )
    {
        vismodule::PointObject* point_object = new vismodule::PointObject(
            new_coords, new_colors, new_normals, particle_property.m_subpixel_level
        );
        point_object->setMinMaxObjectCoords( mvpl.m_total_min_object_coord, mvpl.m_total_max_object_coord );

        if ( async_io_enabled )
        {
            pbvr::ParticleWriteThread* particle_write_thread = &pwt;
            particle_write_thread->join( true );
            particle_write_thread->setPointObject( point_object );
            particle_write_thread->setFilename( particleFilePath.c_str() );
            particle_write_thread->work( true );
        }
        else
        {
            vismodule::KVSMLObjectPoint* kvsml_object =
                new vismodule::PointExporter<vismodule::KVSMLObjectPoint>( *point_object );
            kvsml_object->setWritingDataType( vismodule::KVSMLObjectPoint::ExternalBinary );
            kvsml_object->write( particleFilePath.c_str() );
            delete kvsml_object;
        }

        delete point_object;
    }

    delete[] displs;
    delete[] recvcounts;
}

void AppendGeneratedParticles(
    vismodule::PointObject* point_object,
    std::vector<float>& coords,
    std::vector<Byte>& colors,
    std::vector<float>& normals
)
{
    if ( point_object )
    {
        MakeParticle( point_object, coords, colors, normals );
        delete point_object;
    }
}

inline const size_t CalculateNumberOfParticlesV35(
    const float density,
    const float volume_of_cell,
    const float repetition,
    vismodule::MersenneTwister* mt
)
{
    const float n_particles = density * volume_of_cell * repetition;
    const float random = static_cast<float>( mt->rand() );

    size_t n = static_cast<size_t>( n_particles );
    if ( n_particles - n > random ) ++n;

    return n;
}

inline size_t ExpandedCapacity( const size_t current_capacity, const size_t required_capacity )
{
    if ( current_capacity >= required_capacity ) return current_capacity;
    size_t capacity = current_capacity > 0 ? current_capacity : 4096;
    while ( capacity < required_capacity ) capacity *= 2;
    return capacity;
}

void ReserveAdditionalUniformParticles(
    const size_t additional_particles,
    std::vector<vismodule::Real32>& coords,
    std::vector<vismodule::Real32>& scalars,
    std::vector<vismodule::Real32>& normals,
    std::vector<int>& cellids,
    std::vector<vismodule::Real32>& sq_scalars,
    std::vector<vismodule::Real32>& tmp_term )
{
    const size_t particle_capacity = ExpandedCapacity( scalars.capacity(), scalars.size() + additional_particles );
    scalars.reserve( particle_capacity );
    cellids.reserve( particle_capacity );
    sq_scalars.reserve( particle_capacity );

    const size_t vector_capacity = particle_capacity * 3;
    coords.reserve( vector_capacity );
    normals.reserve( vector_capacity );
    tmp_term.reserve( vector_capacity );
}

struct ChainRuleEvalContext
{
    ChainRuleEvalContext():
        valid( false )
    {
        std::fill( variable_values, variable_values + 128, 0.0f );
    }

    void initialize( TransferFunctionSynthesizer* tfs, const int nvariables )
    {
        valid = false;
        const std::vector<EquationToken> opa_vars = tfs->opacityVariable();
        if ( opa_vars.empty() ) return;

        expr = opa_vars[0]; // 現状の統計粒子生成は TF 数 1 個想定
        rpn.setExpToken( &( expr.exp_token[0] ) );
        rpn.setVariableName( &( expr.var_name[0] ) );
        rpn.setNumber( &( expr.val_array[0] ) );
        rpn.setVariableValue( variable_values );
        workspace.setVariableValueBuffer( variable_values );
        workspace.setExpression( expr, static_cast<std::size_t>( nvariables ) );
        valid = true;
    }

    bool valid;
    EquationToken expr;
    FuncParser::ReversePolishNotation rpn;
    pbvr::ChainRuleNormalWorkspace workspace;
    float variable_values[128];
};

void AppendRejectedStatisticParticle(
    const float scalar,
    const vismodule::Vector3f& coord,
    const vismodule::Vector3f& normal,
    vismodule::TransferFunction& tf,
    const float sampling_volume_inverse,
    const float max_opacity,
    const float max_density,
    vismodule::MersenneTwister* mt,
    std::vector<float>& coords,
    std::vector<Byte>& colors,
    std::vector<float>& normals
)
{
    const float opacity = tf.opacityMap().at( scalar );
    const float density = opacity < max_opacity ?
        -std::log( 1.0f - opacity ) * sampling_volume_inverse :
        max_density;

    if ( density <= max_density * static_cast<float>( mt->rand() ) ) return;

    const vismodule::RGBColor color = tf.colorMap().at( scalar );

    coords.push_back( coord.x() );
    coords.push_back( coord.y() );
    coords.push_back( coord.z() );
    colors.push_back( color.r() );
    colors.push_back( color.g() );
    colors.push_back( color.b() );
    normals.push_back( normal.x() );
    normals.push_back( normal.y() );
    normals.push_back( normal.z() );
}

bool EnsembleHistogramBin(
    const float value,
    const float min_value,
    const float max_value,
    size_t& bin
)
{
    if ( value < min_value || value > max_value ) return false;

    bin = 0;
    if ( vismodule::Math::Equal<float>( min_value, max_value ) ) return true;

    const float position = ( value - min_value ) / ( max_value - min_value ) * DEFAULT_NBINS;
    bin = static_cast<size_t>( position );
    if ( bin >= DEFAULT_NBINS ) bin = DEFAULT_NBINS - 1;
    return true;
}

EnsembleStatisticRange MakeEnsembleStatisticRange(
    const std::vector<float>& values,
    const int tf_number,
    const std::vector<NamedTransferFunction>& transfunc_array
)
{
    EnsembleStatisticRange range;
    range.min_values.assign( tf_number * 2, FLT_MAX );
    range.max_values.assign( tf_number * 2, -FLT_MAX );
    range.o_bins.assign( tf_number * DEFAULT_NBINS, 0 );
    range.c_bins.assign( tf_number * DEFAULT_NBINS, 0 );

    if ( values.empty() ) return range;

    float min_value = FLT_MAX;
    float max_value = -FLT_MAX;
    for ( const float value : values )
    {
        min_value = vismodule::Math::Min( min_value, value );
        max_value = vismodule::Math::Max( max_value, value );
    }

    for ( int i = 0; i < tf_number; i++ )
    {
        range.min_values[2 * i    ] = min_value;
        range.max_values[2 * i    ] = max_value;
        range.min_values[2 * i + 1] = min_value;
        range.max_values[2 * i + 1] = max_value;
    }

    const size_t max_histogram_samples = 100000;
    const size_t stride = std::max<size_t>( 1, ( values.size() + max_histogram_samples - 1 ) / max_histogram_samples );

    for ( size_t i = 0; i < values.size(); i += stride )
    {
        for ( int tf = 0; tf < tf_number; tf++ )
        {
            size_t o_bin = 0;
            size_t c_bin = 0;
            if ( EnsembleHistogramBin(
                values[i],
                transfunc_array[tf].opacityMap().minValue(),
                transfunc_array[tf].opacityMap().maxValue(),
                o_bin ) )
            {
                range.o_bins[o_bin + tf * DEFAULT_NBINS]++;
            }
            if ( EnsembleHistogramBin(
                values[i],
                transfunc_array[tf].colorMap().minValue(),
                transfunc_array[tf].colorMap().maxValue(),
                c_bin ) )
            {
                range.c_bins[c_bin + tf * DEFAULT_NBINS]++;
            }
        }
    }

    return range;
}

void ReduceEnsembleStatisticRange(
    EnsembleStatisticRange& range,
    const int tf_number,
    MPI_Comm comm = MPI_COMM_WORLD
)
{
#ifndef CPU_VER
    std::vector<float> min_recv( tf_number * 2, FLT_MAX );
    std::vector<float> max_recv( tf_number * 2, -FLT_MAX );
    std::vector<vismodule::UInt64> o_bins_recv( tf_number * DEFAULT_NBINS, 0 );
    std::vector<vismodule::UInt64> c_bins_recv( tf_number * DEFAULT_NBINS, 0 );

    MPI_Reduce( range.min_values.data(), min_recv.data(), tf_number * 2, MPI_FLOAT, MPI_MIN, 0, comm );
    MPI_Reduce( range.max_values.data(), max_recv.data(), tf_number * 2, MPI_FLOAT, MPI_MAX, 0, comm );
    MPI_Reduce( range.o_bins.data(), o_bins_recv.data(), tf_number * DEFAULT_NBINS, MPI_UNSIGNED_LONG, MPI_SUM, 0, comm );
    MPI_Reduce( range.c_bins.data(), c_bins_recv.data(), tf_number * DEFAULT_NBINS, MPI_UNSIGNED_LONG, MPI_SUM, 0, comm );

    range.min_values.swap( min_recv );
    range.max_values.swap( max_recv );
    range.o_bins.swap( o_bins_recv );
    range.c_bins.swap( c_bins_recv );
#endif
}

void EnsureEnsembleStatisticRangeSize(
    EnsembleStatisticRange& range,
    const int tf_number
)
{
    const size_t value_size = static_cast<size_t>( tf_number * 2 );
    const size_t histogram_size = static_cast<size_t>( tf_number * DEFAULT_NBINS );

    if ( range.min_values.size() < value_size ) range.min_values.resize( value_size, 0.0f );
    if ( range.max_values.size() < value_size ) range.max_values.resize( value_size, 0.0f );
    if ( range.o_bins.size() < histogram_size ) range.o_bins.resize( histogram_size, 0 );
    if ( range.c_bins.size() < histogram_size ) range.c_bins.resize( histogram_size, 0 );
}

void WritePrefixedStatisticHistory(
    std::ofstream& ofs,
    const std::string& prefix,
    const EnsembleStatisticRange& range,
    const int tf_number
)
{
    for ( int i = 0; i < tf_number; i++ )
    {
        ofs << prefix << "_MIN_O" << ( i + 1 ) << "=" << range.min_values[2 * i    ] << std::endl;
        ofs << prefix << "_MAX_O" << ( i + 1 ) << "=" << range.max_values[2 * i    ] << std::endl;
        ofs << prefix << "_MIN_C" << ( i + 1 ) << "=" << range.min_values[2 * i + 1] << std::endl;
        ofs << prefix << "_MAX_C" << ( i + 1 ) << "=" << range.max_values[2 * i + 1] << std::endl;
//        ofs << prefix << "_RESOLUTION_O" << ( i + 1 ) << "=" << DEFAULT_NBINS << std::endl;
        ofs << prefix << "_HISTOGRAM_O" << ( i + 1 ) << "=";
        for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
        {
            ofs << range.o_bins[j + i * DEFAULT_NBINS] << ",";
        }
        ofs << std::endl;
//        ofs << prefix << "_RESOLUTION_C" << ( i + 1 ) << "=" << DEFAULT_NBINS << std::endl;
        ofs << prefix << "_HISTOGRAM_C" << ( i + 1 ) << "=";
        for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
        {
            ofs << range.c_bins[j + i * DEFAULT_NBINS] << ",";
        }
        ofs << std::endl;
    }
}

void WriteStatisticHistory(
    std::ofstream& ofs,
    const EnsembleStatisticRange& range,
    const int tf_number
)
{
    for ( int i = 0; i < tf_number; i++ )
    {
        ofs << "MIN_O" << ( i + 1 ) << "=" << range.min_values[2 * i    ] << std::endl;
        ofs << "MAX_O" << ( i + 1 ) << "=" << range.max_values[2 * i    ] << std::endl;
        ofs << "MIN_C" << ( i + 1 ) << "=" << range.min_values[2 * i + 1] << std::endl;
        ofs << "MAX_C" << ( i + 1 ) << "=" << range.max_values[2 * i + 1] << std::endl;
        ofs << "RESOLUTION_O" << ( i + 1 ) << "=" << DEFAULT_NBINS << std::endl;
        ofs << "HISTOGRAM_O" << ( i + 1 ) << "=";
        for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
        {
            ofs << range.o_bins[j + i * DEFAULT_NBINS] << ",";
        }
        ofs << std::endl;
        ofs << "RESOLUTION_C" << ( i + 1 ) << "=" << DEFAULT_NBINS << std::endl;
        ofs << "HISTOGRAM_C" << ( i + 1 ) << "=";
        for ( size_t j = 0; j < DEFAULT_NBINS; j++ )
        {
            ofs << range.c_bins[j + i * DEFAULT_NBINS] << ",";
        }
        ofs << std::endl;
    }
}

void StoreStatisticRangeToTransferFunctions(
    const EnsembleStatisticRange& range,
    std::vector<NamedTransferFunction>& transfer_functions,
    const int tf_number
)
{
    if ( static_cast<int>( transfer_functions.size() ) < tf_number ) return;
    if ( range.min_values.size() < static_cast<size_t>( tf_number * 2 ) ||
         range.max_values.size() < static_cast<size_t>( tf_number * 2 ) ||
         range.o_bins.size() < static_cast<size_t>( tf_number * DEFAULT_NBINS ) ||
         range.c_bins.size() < static_cast<size_t>( tf_number * DEFAULT_NBINS ) ) return;

    for ( int i = 0; i < tf_number; i++ )
    {
        transfer_functions[i].m_server_opacity_variable_min = range.min_values[2 * i    ];
        transfer_functions[i].m_server_opacity_variable_max = range.max_values[2 * i    ];
        transfer_functions[i].m_server_color_variable_min   = range.min_values[2 * i + 1];
        transfer_functions[i].m_server_color_variable_max   = range.max_values[2 * i + 1];
        std::copy(
            range.o_bins.begin() + i * DEFAULT_NBINS,
            range.o_bins.begin() + ( i + 1 ) * DEFAULT_NBINS,
            transfer_functions[i].m_opacity_histogram );
        std::copy(
            range.c_bins.begin() + i * DEFAULT_NBINS,
            range.c_bins.begin() + ( i + 1 ) * DEFAULT_NBINS,
            transfer_functions[i].m_color_histogram );
    }
}

void OutputEnsembleStatisticHistory(
    ParticleProperty& particle_property,
    const int tf_number,
    const int nvariables,
    const std::string& historyFilePath,
    EnsembleStatisticRange average_range,
    EnsembleStatisticRange variance_range,
    EnsembleStatisticRange co_variation_range,
    MPI_Comm ensemble_comm = MPI_COMM_WORLD
)
{
    EnsureEnsembleStatisticRangeSize( average_range, tf_number );
    EnsureEnsembleStatisticRangeSize( variance_range, tf_number );
    EnsureEnsembleStatisticRangeSize( co_variation_range, tf_number );

    ReduceEnsembleStatisticRange( average_range, tf_number, ensemble_comm );
    ReduceEnsembleStatisticRange( variance_range, tf_number, ensemble_comm );
    ReduceEnsembleStatisticRange( co_variation_range, tf_number, ensemble_comm );

    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( ensemble_comm, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    if ( mpi_rank != 0 ) return;

    std::ofstream ofs( historyFilePath.c_str(), std::ios::out );
    ofs << "TF_NUMBER=" << tf_number << std::endl;
    ofs << "RESOLUTION=" << DEFAULT_NBINS << std::endl;
    WriteStatisticHistory( ofs, average_range, tf_number );
    WritePrefixedStatisticHistory( ofs, "AVE", average_range, tf_number );
    WritePrefixedStatisticHistory( ofs, "VAR", variance_range, tf_number );
    WritePrefixedStatisticHistory( ofs, "COV", co_variation_range, tf_number );
    ofs << "N_VARIABLES=" << nvariables << std::endl;
    ofs << "PARTICLE_LIMIT=" << particle_property.m_particle_limit << std::endl;
    ofs << "END_HISTORY_FILE=SUCCESS" << std::endl;
    ofs.close();

    if ( particle_property.m_mean_transfer_function_array.empty() )
    {
        particle_property.m_mean_transfer_function_array = particle_property.m_transfunc_array;
    }
    if ( particle_property.m_variance_transfer_function_array.empty() )
    {
        particle_property.m_variance_transfer_function_array = particle_property.m_transfunc_array;
    }
    if ( particle_property.m_coefficient_of_variation_transfer_function_array.empty() )
    {
        particle_property.m_coefficient_of_variation_transfer_function_array = particle_property.m_transfunc_array;
    }

    StoreStatisticRangeToTransferFunctions( average_range, particle_property.m_transfunc_array, tf_number );
    StoreStatisticRangeToTransferFunctions( average_range, particle_property.m_mean_transfer_function_array, tf_number );
    StoreStatisticRangeToTransferFunctions( variance_range, particle_property.m_variance_transfer_function_array, tf_number );
    StoreStatisticRangeToTransferFunctions(
        co_variation_range,
        particle_property.m_coefficient_of_variation_transfer_function_array,
        tf_number );

    ParameterFileWriter ppw;
    ppw.writeTF2OldJson( particle_property );
}

} // namespace

void OutputCoordMinMaxFile(
    const domain_parameters_unstruct& dom,
    const std::string& coordMinMaxFilePath
)
{
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    static bool minmaxFlag = false;
    if (minmaxFlag == false && mpi_rank == 0) {
        FILE* fp = fopen( coordMinMaxFilePath.c_str(), "w" );
        if( fp )
        {
            fprintf( fp, "%f %f %f %f %f %f\n",
                     dom.x_global_min,
                     dom.y_global_min,
                     dom.z_global_min,
                     dom.x_global_max,
                     dom.y_global_max,
                     dom.z_global_max );
            fclose( fp );
        }
        minmaxFlag = true;
    }
}

// 変数配列用のソルバー関数
bool generate_particles(
    int time_step,
    domain_parameters_unstruct dom,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const  vismodule::VolumeObjectBase::CellType& celltype
)
{
    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif
    if ( is_initial_step == true )
    {
        is_initial_step = false;
        start_time_step = time_step;
    }

    std::string historyFilePath;
    std::string stateFilePath;
    std::string coordMinMaxFilePath;
    std::string particleFilePrefix;
    std::string glyphFilePrefix;
    std::string plotOverLineFilePrefix;
    std::string plotOverTimeFilePrefix;
    std::string tfJsonPath;
    std::string tfJsonPath_old;
    std::string tfJsonPath_step;
    std::string glyphParameterPath;
    std::string glyphParameterPath_old;
    std::string plotOverLineParameterPath;
    std::string plotOverLineParameterPath_old; 
    std::string plotOverTimeParameterPath;
    std::string plotOverTimeParameterPath_old;

    SetParameterFilePath(
        time_step,
        historyFilePath,
        stateFilePath,
        coordMinMaxFilePath,
        particleFilePrefix,
        glyphFilePrefix,
        plotOverLineFilePrefix,
        plotOverTimeFilePrefix,
        tfJsonPath,
        tfJsonPath_old,
        tfJsonPath_step,
        glyphParameterPath,
        glyphParameterPath_old,
        plotOverLineParameterPath,
        plotOverLineParameterPath_old,
        plotOverTimeParameterPath,
        plotOverTimeParameterPath_old
    );

    char  arg_dummy0[] = "dummy";
    char* arg_dummy[]  = { arg_dummy0, NULL };

    ParticleProperty particle_property;
    GlyphProperty glyph_property;
    PlotOverLineProperty pol_property;
    PlotOverTimeProperty pot_property;
    MultiVolumePropertyList mvpl;
    particle_property.m_transfunc_synthesizer = new TransferFunctionSynthesizer();
    particle_property.m_camera                = new vismodule::Camera();

    bool object_generation_enabled = false;
    SetParticleParameter(
        dom, tfJsonPath, tfJsonPath_old, particle_property, mvpl,
        nvariables, object_generation_enabled
    );
    if ( object_generation_enabled )
    {
        SetGlyphParameter( glyphParameterPath, glyphParameterPath_old, glyph_property );
        SetPlotOverLineParameter( plotOverLineParameterPath, plotOverLineParameterPath_old, pol_property );
        SetPlotOverTimeParameter( plotOverTimeParameterPath, plotOverTimeParameterPath_old, pot_property );
    }

    const int tf_number  = particle_property.m_transfunc_array.size();
    const int resolution = object_generation_enabled ? pol_property.m_sampling_size : 0;

    // particle parameters
    std::vector<float> particle_coords;
    std::vector<Byte>  particle_colors;
    std::vector<float> particle_normals;
    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    float* tmp_max;
    float* tmp_min;

    tmp_c_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_o_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_max = new float[tf_number * 2]; // color, opacity
    tmp_min = new float[tf_number * 2]; // color, opacity

    for ( std::size_t i = 0; i < (DEFAULT_NBINS * tf_number); i++ )
    {
        tmp_c_bins[i] = 0;
        tmp_o_bins[i] = 0;
    }

    for ( int i = 0; i < (tf_number * 2); i++ )
    {
        tmp_max[i] = FLT_MIN;
        tmp_min[i] = FLT_MAX;
    }

    // glyph parameters
    std::vector<float>         glyph_coords;
    std::vector<float>         glyph_vectors;
    std::vector<float>         glyph_sizes;
    std::vector<unsigned char> glyph_colors;

    // plot over line parameters
    std::vector<float> values_on_line( resolution, 0 );
    std::vector<int>   mask( resolution, 0 );
    std::vector<float> x_axis( resolution, 0 );

    // plot over time paraemters
    std::vector<float> value_on_time( nvariables, 0 );
    bool pot_mask = false;

    ServerMode server_mode = ServerMode::IS;

    CollectParticleMinMax(
        particle_property, dom, values, nvariables, coordinates,
        ncoords, connections, ncells, celltype, tf_number, tmp_max, tmp_min
    );

#ifndef CPU_VER
    if ( mpi_size > 1 )
    {
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, ( tf_number * 2 ), MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, ( tf_number * 2 ), MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    }
#endif

    for( std::size_t i = 0; i < tf_number; i++ )
    {
        const float color_variable_min   = tmp_min[2 * i + 1];
        const float color_variable_max   = tmp_max[2 * i + 1];
        const float opacity_variable_min = tmp_min[2 * i    ];
        const float opacity_variable_max = tmp_max[2 * i    ];

        particle_property.m_transfunc_array[i].m_server_color_variable_min   = color_variable_min;
        particle_property.m_transfunc_array[i].m_server_color_variable_max   = color_variable_max;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_min = opacity_variable_min;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_max = opacity_variable_max;

        if( particle_property.m_transfunc_array[i].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[i].setColorRange( color_variable_min, color_variable_max );
        }

        if( particle_property.m_transfunc_array[i].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[i].setOpacityRange( opacity_variable_min, opacity_variable_max );
        }
    }

    CollectParticleHistogram(
        particle_property, dom, values, nvariables, coordinates,
        ncoords, connections, ncells, celltype, tf_number, tmp_c_bins, tmp_o_bins
    );

    if ( object_generation_enabled )
    {
        GenerateParticleObject(
            particle_property, dom, values, nvariables, coordinates,
            ncoords, connections, ncells, celltype,
            particle_coords, particle_colors, particle_normals
        );

        if ( glyph_property.m_glyph_flag )
        {
            vismodule::KVSMLObjectGlyph* glyph_object = new vismodule::KVSMLObjectGlyph;
            vismodule::GlyphSeedGenerator glyph_creator;
            int number_of_divide = mpi_size;
            glyph_creator.GenerateGlyphUnstruct(
                glyph_property, number_of_divide, values, nvariables, coordinates,
                ncoords, connections, ncells, celltype, server_mode, glyph_object
            );

            MakeGlyph( glyph_object, glyph_coords, glyph_vectors, glyph_sizes, glyph_colors ); // InSitu only

            delete glyph_object;
        }

        // if ( pol_property.m_plot_flag )
        if ( true ) // 常に生成し続ける、将来的に変わる可能性あり
        {
            vismodule::KVSMLObjectPlotOverLine* pol_object = new vismodule::KVSMLObjectPlotOverLine;
            PlotOverLineGenerator pol_generator;
            pol_generator.GeneratePOLUnstruct(
                pol_property, values, nvariables, coordinates,
                ncoords, connections, ncells, celltype, pol_object
            );

            for( std::size_t i = 0; i < resolution; i++ )
            {
                x_axis[i] = pol_object->x_axis()[i];
                if ( pol_object->mask()[i] )
                {
                    mask[i]           = 1;
                    values_on_line[i] = pol_object->values_on_line()[i];
                }
            }

            delete pol_object;
        }

        // if ( pot_property.m_plot_flag )
        if ( true ) // 常に生成し続ける、将来的に変わる可能性あり
        {
            PlotOverTimeGenerator pot_generator;
            pot_mask = pot_generator.GeneratePOTUnstruct(
                pot_property, values, nvariables, coordinates,
                ncoords, connections, ncells, celltype, value_on_time
            );
        }
    }

    OutputCoordMinMaxFile( dom, coordMinMaxFilePath );

    if ( object_generation_enabled )
    {
        OutputParticleFiles(
            particle_property, mvpl, time_step, particleFilePrefix,
            particle_coords, particle_colors, particle_normals
        );
    }

    OutputParticleHistory(
        particle_property, tf_number, nvariables, historyFilePath,
        tmp_c_bins, tmp_o_bins, tmp_max, tmp_min
    );

    if ( object_generation_enabled && glyph_property.m_glyph_flag )
    {
        OutputGlyphs(
            time_step, glyphFilePrefix, glyph_coords,
            glyph_vectors, glyph_sizes, glyph_colors
        );
    }

    // if ( pol_property.m_plot_flag )
    if ( object_generation_enabled ) // 常に出力し続ける、将来的に変わる可能性あり
    {
        OutputLine( time_step, plotOverLineFilePrefix, values_on_line, mask, x_axis );
    }

    // if ( pot_property.m_plot_flag )
    if ( object_generation_enabled ) // 常に出力し続ける、将来的に変わる可能性あり
    {
        OutputPOT( time_step, plotOverTimeFilePrefix, pot_mask, value_on_time );
    }

    // OutputParticlesで更新したdefault_old.jsonをtimestep別JSONに保存する
    if ( object_generation_enabled && mpi_rank == 0 )
    {
        std::ifstream src( tfJsonPath_old.c_str(), std::ios::binary );
        std::ofstream dst( tfJsonPath_step.c_str(), std::ios::binary );
        dst << src.rdbuf();
    }

    // 粒子ファイル書き込みスレッドが終了するまで待機
    // async_io_enabled, pwtはkvs_wrapper_common.cppに宣言
    if ( async_io_enabled )
    {
        std::cout << "Particle write thread is active."             << std::endl;
        std::cout << "Waiting for particle write thread to finish." << std::endl;
        pbvr::ParticleWriteThread* particle_write_thread = &pwt;
        particle_write_thread->join( true );
        std::cout << "Particle write thread is finished." << std::endl;
    }
    else
    {
        std::cout << "Particle write thread is not active." << std::endl;
    }

    std::cout << "Waiting for all processes to finish." << std::endl;

#ifndef CPU_VER
    MPI_Barrier( MPI_COMM_WORLD ); // すべてのプロセスでファイルの書き込む処理が終了するまで待機
#endif

    std::cout << "All processes have finished." << std::endl;

    if ( mpi_rank == 0 )
    {
        std::ofstream ofs( stateFilePath.c_str(), std::ios::out );
        if( !ofs.is_open() ) std::cout << "Cannot open state.txt" << std::endl;

        ofs << "START_STEP  = " << start_time_step << std::endl;
        ofs << "LATEST_STEP = " << time_step       << std::endl;

        ofs.close();
    }
   
    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
    delete[] tmp_max;
    delete[] tmp_min;
    delete particle_property.m_transfunc_synthesizer;
    delete particle_property.m_camera;

    return true;
}

struct ChainRuleTimingBreakdown
{
    double calc_scalar_grad;
    double q_grad_setup;
    double tf_scalar_eval;
    double chain_rule_dfdq;
    double normal_normalize;

    ChainRuleTimingBreakdown():
        calc_scalar_grad( 0.0 ),
        q_grad_setup( 0.0 ),
        tf_scalar_eval( 0.0 ),
        chain_rule_dfdq( 0.0 ),
        normal_normalize( 0.0 )
    {
    }
};

// SIMD (across-particle) block chain rule: replaces the per-particle tf_scalar_eval
// + chain_rule_dfdq loop. Evaluates F and its finite-difference gradient for the
// whole particle block via ReversePolishNotation::evalArraySIMD. Bit-identical to
// the scalar path (same math / order); just vectorized across particles and with
// far fewer eval() calls (per-block instead of per-particle).
inline void chainRuleBlock(
    ChainRuleEvalContext& ctx,
    const int n,
    const int nvariables,
    float (*scalar_array)[SIMD_BLK_SIZE],
    float (*grad_qx)[SIMD_BLK_SIZE],
    float (*grad_qy)[SIMD_BLK_SIZE],
    float (*grad_qz)[SIMD_BLK_SIZE],
    const vismodule::Vector3f* coord,
    float* scalar_out,
    float* grad_array_x,
    float* grad_array_y,
    float* grad_array_z,
    ChainRuleTimingBreakdown* timing )
{
    const float FD = 1.0e-5f;   // == ChainRuleNormal FiniteDifferenceScale
    alignas(64) float xa[SIMD_BLK_SIZE], ya[SIMD_BLK_SIZE], za[SIMD_BLK_SIZE];
    for ( int p = 0; p < n; ++p ) { xa[p] = coord[p].x(); ya[p] = coord[p].y(); za[p] = coord[p].z(); }

    float* varr[NUMVAR];
    for ( int i = 0; i < NUMVAR; ++i ) varr[i] = 0;
    varr[X] = xa; varr[Y] = ya; varr[Z] = za;
    for ( int v = 0; v < nvariables; ++v )
    {
        varr[Q1 + 4 * v]     = scalar_array[v];
        varr[Q1 + 4 * v + 1] = grad_qx[v];
        varr[Q1 + 4 * v + 2] = grad_qy[v];
        varr[Q1 + 4 * v + 3] = grad_qz[v];
    }
    ctx.rpn.setVariableValueArray( varr );

#ifdef ENABLE_ENSEMBLE_TIMER
    vismodule::Timer t_eval; t_eval.start();
#endif
    ctx.rpn.evalArraySIMD( scalar_out, n );          // F (unperturbed)
#ifdef ENABLE_ENSEMBLE_TIMER
    t_eval.stop(); if ( timing ) timing->tf_scalar_eval += t_eval.sec();
#endif

    for ( int p = 0; p < n; ++p ) { grad_array_x[p] = 0.0f; grad_array_y[p] = 0.0f; grad_array_z[p] = 0.0f; }
#ifdef PBVR_SKIP_CHAINRULE
    // Optional (OFF by default): skip the finite-difference chain rule that produces
    // the statistic-particle normal, and assign a fixed placeholder gradient. The
    // per-variable gradient (grad_ary / grad_q) is still computed in the caller, so
    // TF expressions that reference gradient variables (dq*) remain correct, as does
    // the particle scalar F (evaluated above). Requires PBVR_SIMD_CHAINRULE.
    // NOTE: with this on, the mean/variance/cv particle normals and the
    // tmp_term = scalar*normal statistic are no longer meaningful.
    // Measured saving (chain rule only): compute ~6% (correctness 2x1),
    // ~9% compute / ~7% total (strong_4x1).
    for ( int p = 0; p < n; ++p ) { grad_array_x[p] = 1.0f; grad_array_y[p] = 0.0f; grad_array_z[p] = 0.0f; }
    return;
#endif

#ifdef ENABLE_ENSEMBLE_TIMER
    vismodule::Timer t_cr; t_cr.start();
#endif
    alignas(64) float Fp[SIMD_BLK_SIZE], Fm[SIMD_BLK_SIZE], qs[SIMD_BLK_SIZE], eps[SIMD_BLK_SIZE];
    const std::vector<std::size_t>& act = ctx.workspace.activeVariables();
    for ( std::size_t k = 0; k < act.size(); ++k )
    {
        const int i = static_cast<int>( act[k] );
        float* qi = scalar_array[i];                 // varr[Q1+4i] aliases this
        for ( int p = 0; p < n; ++p ) { qs[p] = qi[p]; eps[p] = FD * std::max( 1.0f, std::fabs( qi[p] ) ); }
        for ( int p = 0; p < n; ++p ) qi[p] = qs[p] + eps[p];
        ctx.rpn.evalArraySIMD( Fp, n );
        for ( int p = 0; p < n; ++p ) qi[p] = qs[p] - eps[p];
        ctx.rpn.evalArraySIMD( Fm, n );
        for ( int p = 0; p < n; ++p ) qi[p] = qs[p];
        for ( int p = 0; p < n; ++p )
        {
            const float dF = ( Fp[p] - Fm[p] ) / ( 2.0f * eps[p] );
            if ( std::isfinite( dF ) )
            {
                grad_array_x[p] += grad_qx[i][p] * dF;
                grad_array_y[p] += grad_qy[i][p] * dF;
                grad_array_z[p] += grad_qz[i][p] * dF;
            }
        }
    }
    for ( int p = 0; p < n; ++p )
    {
        if ( !( std::isfinite( grad_array_x[p] ) && std::isfinite( grad_array_y[p] ) && std::isfinite( grad_array_z[p] ) ) )
        { grad_array_x[p] = 0.0f; grad_array_y[p] = 0.0f; grad_array_z[p] = 0.0f; }
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    t_cr.stop(); if ( timing ) timing->chain_rule_dfdq += t_cr.sec();
#endif
}

void calculate_scalar_and_chain_rule_grad(
    const int nparticles_count,
    const int nvariables,
    ChainRuleEvalContext& chain_context,
    const std::vector< vismodule::CellBase<Type>* >& interp,
    const vismodule::Vector3f* local_coord_array,
    const vismodule::Vector3f* global_coord_array,
    const vismodule::UInt32* cell_index,
    float* scalar_result,
    float* grad_array_x,
    float* grad_array_y,
    float* grad_array_z,
    ChainRuleTimingBreakdown* timing )
{
    float scalar_array[nvariables][SIMD_BLK_SIZE];
    float grad_qx[nvariables][SIMD_BLK_SIZE];
    float grad_qy[nvariables][SIMD_BLK_SIZE];
    float grad_qz[nvariables][SIMD_BLK_SIZE];

#ifdef ENABLE_ENSEMBLE_TIMER
    vismodule::Timer calc_scalar_grad_timer;
    calc_scalar_grad_timer.start();
#endif
    static const bool s_jac_reuse_disabled = ( getenv( "PBVR_NO_JACREUSE" ) != 0 );
    if ( nvariables > 1 && interp[0]->supportsJacobianReuse() && !s_jac_reuse_disabled )
    {
        // Candidate A: the inverse Jacobian is geometry-only (vertices + shape-fn
        // derivatives) and identical across variables sharing coordinates/connections.
        // Compute it once on interp[0], reuse it for every variable's gradient.
        double cof[9][SIMD_BLK_SIZE];
        double det_inverse[SIMD_BLK_SIZE];
        double scale_factor[SIMD_BLK_SIZE];
        double determinant[SIMD_BLK_SIZE];
        interp[0]->setLocalPointArray( nparticles_count, local_coord_array );
        interp[0]->computeScaledInvJacobianArray(
            nparticles_count, cof, det_inverse, scale_factor, determinant );
        for ( int j = 0; j < nvariables; ++j )
        {
            if ( j != 0 ) interp[j]->setLocalPointArray( nparticles_count, local_coord_array );
            interp[j]->scalar_ary( scalar_array[j], nparticles_count );
            interp[j]->gradFromScaledInvJacobianArray(
                nparticles_count, cof, det_inverse, scale_factor, determinant,
                grad_qx[j], grad_qy[j], grad_qz[j] );
        }
    }
    else
    {
        for ( int j = 0; j < nvariables; ++j )
        {
            interp[j]->setLocalPointArray( nparticles_count, local_coord_array );
            interp[j]->CalcScalarGrad(
                nparticles_count,
                scalar_array[j],
                grad_qx[j],
                grad_qy[j],
                grad_qz[j] );
        }
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    calc_scalar_grad_timer.stop();
    if ( timing ) timing->calc_scalar_grad += calc_scalar_grad_timer.sec();
#endif

    if ( !chain_context.valid )
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer normal_timer;
        normal_timer.start();
#endif
        for ( int p = 0; p < nparticles_count; ++p )
        {
            scalar_result[p] = 0.0f;
            grad_array_x[p] = 0.0f;
            grad_array_y[p] = 0.0f;
            grad_array_z[p] = 0.0f;
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        normal_timer.stop();
        if ( timing ) timing->normal_normalize += normal_timer.sec();
#endif
        return;
    }

#ifdef PBVR_SIMD_CHAINRULE
    chainRuleBlock( chain_context, nparticles_count, nvariables,
                    scalar_array, grad_qx, grad_qy, grad_qz,
                    global_coord_array,
                    scalar_result, grad_array_x, grad_array_y, grad_array_z,
                    timing );
    return;
#endif

    float q_values[128];
    vismodule::Vector3f grad_q[128];

    for ( int p = 0; p < nparticles_count; ++p )
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer q_grad_setup_timer;
        q_grad_setup_timer.start();
#endif
        chain_context.variable_values[X] = global_coord_array[p].x();
        chain_context.variable_values[Y] = global_coord_array[p].y();
        chain_context.variable_values[Z] = global_coord_array[p].z();

        for ( int v = 0; v < nvariables; ++v )
        {
            const std::size_t q = static_cast<std::size_t>( 4 * ( v + 1 ) );
            q_values[v] = scalar_array[v][p];
            chain_context.variable_values[q] = scalar_array[v][p];
            chain_context.variable_values[q + 1] = grad_qx[v][p];
            chain_context.variable_values[q + 2] = grad_qy[v][p];
            chain_context.variable_values[q + 3] = grad_qz[v][p];
            grad_q[v] = vismodule::Vector3f(
                grad_qx[v][p],
                grad_qy[v][p],
                grad_qz[v][p] );
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        q_grad_setup_timer.stop();
        if ( timing ) timing->q_grad_setup += q_grad_setup_timer.sec();
#endif

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer tf_scalar_eval_timer;
        tf_scalar_eval_timer.start();
#endif
        scalar_result[p] = chain_context.rpn.eval();
#ifdef ENABLE_ENSEMBLE_TIMER
        tf_scalar_eval_timer.stop();
        if ( timing ) timing->tf_scalar_eval += tf_scalar_eval_timer.sec();
#endif

        vismodule::Vector3f grad_F;
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer chain_rule_timer;
        chain_rule_timer.start();
#endif
        const bool ok = chain_context.workspace.computeGradient(
            q_values,
            grad_q,
            nvariables,
            &grad_F );
#ifdef ENABLE_ENSEMBLE_TIMER
        chain_rule_timer.stop();
        if ( timing ) timing->chain_rule_dfdq += chain_rule_timer.sec();
#endif

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer normal_timer;
        normal_timer.start();
#endif
        if ( !ok )
        {
            grad_array_x[p] = 0.0f;
            grad_array_y[p] = 0.0f;
            grad_array_z[p] = 0.0f;
#ifdef ENABLE_ENSEMBLE_TIMER
            normal_timer.stop();
            if ( timing ) timing->normal_normalize += normal_timer.sec();
#endif
            continue;
        }

        grad_array_x[p] = grad_F.x();
        grad_array_y[p] = grad_F.y();
        grad_array_z[p] = grad_F.z();
#ifdef ENABLE_ENSEMBLE_TIMER
        normal_timer.stop();
        if ( timing ) timing->normal_normalize += normal_timer.sec();
#endif
    }
}

void calculation_glad(const int nparticles_count, const int nvariables,
        TransferFunctionSynthesizer* th_tfs,
        std::vector<vismodule::TransferFunction>& th_tf,
        const std::vector< vismodule::CellBase<Type>* > interp,
        const vismodule::Vector3f* local_coord_array,
        const vismodule::UInt32* cell_index,
        float* grad_array_x,
        float* grad_array_y,
        float* grad_array_z)
{

    vismodule::Vector3f l_plus_coord[ SIMD_BLK_SIZE ];
    vismodule::Vector3f l_minus_coord[ SIMD_BLK_SIZE ];
    vismodule::Vector3f g_plus_coord[ SIMD_BLK_SIZE ];
    vismodule::Vector3f g_minus_coord[ SIMD_BLK_SIZE ];
    float S_plus_opacity[ SIMD_BLK_SIZE ];
    float S_minus_opacity[ SIMD_BLK_SIZE ];


                    for( int j = 0; j < nvariables; j++ )
                    {
                        interp[j]->bindCellArray( nparticles_count, cell_index );
                    }


                    // dsdx ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0.1,0,0);
                        l_minus_coord[j] = local_coord_array[j] + vismodule::Vector3f(-0.1,0,0);

                    }

                    interp[0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_plus_coord,
                                                                  g_plus_coord );

                    interp[0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_minus_coord,
                                                                  g_minus_coord );


                    th_tfs->CalculateOpacityArray( interp,
                                                         nparticles_count,
                                                         l_plus_coord,
                                                         g_plus_coord,
                                                         th_tf,
                                                         S_plus_opacity );
                    th_tfs->CalculateOpacityArray( interp,
                                                         nparticles_count,
                                                         l_minus_coord,
                                                         g_minus_coord,
                                                         th_tf,
                                                         S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        grad_array_x[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
                // ------------------------------------------------

                    // dsdy ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,0.1,0);
                        l_minus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,-0.1,0);
                    }

                    interp[0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_plus_coord,
                                                                  g_plus_coord );

                    interp[0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_minus_coord,
                                                                  g_minus_coord );

                    th_tfs->CalculateOpacityArray( interp,
                                                         nparticles_count,
                                                         l_plus_coord,
                                                         g_plus_coord,
                                                         th_tf,
                                                         S_plus_opacity );
                    th_tfs->CalculateOpacityArray( interp,
                                                         nparticles_count,
                                                         l_minus_coord,
                                                         g_minus_coord,
                                                         th_tf,
                                                         S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        grad_array_y[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
                // ------------------------------------------------
                    // dsdz ----------------------------------------
                    // dsdz ----------------------------------------
                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        l_plus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,0,0.1);
                        l_minus_coord[j] = local_coord_array[j] + vismodule::Vector3f(0,0,-0.1);
                    }

                    interp[0]->setLocalPointArray( nparticles_count, l_plus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                            l_plus_coord,
                                                            g_plus_coord );

                    interp[0]->setLocalPointArray( nparticles_count, l_minus_coord );
                    interp[0]->transformLocalToGlobalArray( nparticles_count,
                                                                  l_minus_coord,
                                                                  g_minus_coord );

                    th_tfs->CalculateOpacityArray( interp,
                                                   nparticles_count,
                                                   l_plus_coord,
                                                   g_plus_coord,
                                                   th_tf,
                                                   S_plus_opacity );
                    th_tfs->CalculateOpacityArray( interp,
                                                         nparticles_count,
                                                         l_minus_coord,
                                                         g_minus_coord,
                                                         th_tf,
                                                         S_minus_opacity );

                    for( int j = 0; j < nparticles_count; j++ )
                    {
                        grad_array_z[j] = ( S_plus_opacity[j] - S_minus_opacity[j] )*5.0;
                    }
}


// Bind all variables for a cell block. The vertex gather in bindCellArray is identical
// across variables; when the Jacobian-reuse path is active (Hex, nvariables>1) the
// variables k>0 never read their own m_vertices_array (geometry goes through cell[*][0]).
// So bind full scalars+vertices for variable 0 and scalars only for the rest, skipping
// the redundant vertex gather. PBVR_FULL_BIND forces the old full bind for every variable.
static inline void bind_variables_scalars_opt(
    std::vector< vismodule::CellBase<Type>* >& cells,
    const int nvariables, const int n, const vismodule::UInt32* cell_index )
{
    static const bool full_bind = ( getenv( "PBVR_FULL_BIND" ) != 0 );
    cells[0]->bindCellArray( n, cell_index );
    const bool scalars_only = ( !full_bind && cells[0]->supportsJacobianReuse() );
    for ( int k = 1; k < nvariables; ++k )
    {
        if ( scalars_only ) cells[k]->bindScalarsArray( n, cell_index );
        else                cells[k]->bindCellArray( n, cell_index );
    }
}

bool ensemble_generate_particles(
    int time_step,
    const int num_ensemble,
    domain_parameters_unstruct dom,
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype
)
{
#if _OPENMP
    int max_threads = omp_get_max_threads();
#else
    int max_threads = 1;
#endif


    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
    EnsembleTimerCollector ensemble_timer( time_step, max_threads );
    vismodule::Timer ensemble_total_timer;
    ensemble_total_timer.start();
#endif

    if ( is_initial_step == true )
    {
        is_initial_step = false;
        start_time_step = time_step;
    }

    std::string historyFilePath;
    std::string stateFilePath;
    std::string coordMinMaxFilePath;
    std::string particleFilePrefix;
    std::string glyphFilePrefix;
    std::string plotOverLineFilePrefix;
    std::string plotOverTimeFilePrefix;
    std::string tfJsonPath;
    std::string tfJsonPath_old;
    std::string tfJsonPath_step;
    std::string glyphParameterPath;
    std::string glyphParameterPath_old;
    std::string plotOverLineParameterPath;
    std::string plotOverLineParameterPath_old;
    std::string plotOverTimeParameterPath;
    std::string plotOverTimeParameterPath_old;

    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerSetParameterPath );
#endif
        SetParameterFilePath(
            time_step,
            historyFilePath,
            stateFilePath,
            coordMinMaxFilePath,
            particleFilePrefix,
            glyphFilePrefix,
            plotOverLineFilePrefix,
            plotOverTimeFilePrefix,
            tfJsonPath,
            tfJsonPath_old,
            tfJsonPath_step,
            glyphParameterPath,
            glyphParameterPath_old,
            plotOverLineParameterPath,
            plotOverLineParameterPath_old,
            plotOverTimeParameterPath,
            plotOverTimeParameterPath_old
        );
    }

    ParticleProperty particle_property;
    MultiVolumePropertyList mvpl;
    particle_property.m_transfunc_synthesizer = new TransferFunctionSynthesizer();
    particle_property.m_camera = new vismodule::Camera();

    bool object_generation_enabled = false;
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerReadParameterFile );
#endif
        SetParticleParameter(
            dom, tfJsonPath, tfJsonPath_old, particle_property, mvpl,
            nvariables, object_generation_enabled
        );
    }
    int tf_number = particle_property.m_transfunc_array.size();

    const bool gen_flag = object_generation_enabled;
    if ( !gen_flag )
    {
//        particle_property.m_repeat_level = 1.f; // スタブデータ
        tf_number = nvariables;
        EnsembleStatisticRange average_range;
        EnsembleStatisticRange variance_range;
        EnsembleStatisticRange co_variation_range;
        pbvr::EnsembleCellHistogramLog histogram_log;

        bool ok = false;
        {
#ifdef ENABLE_ENSEMBLE_TIMER
            EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerStatHistogram );
#endif
            ok = pbvr::ComputeAndStoreEnsembleCellHistogram(
                    values,
                    nvariables,
                    coordinates,
                    ncoords,
                    connections,
                    ncells,
                    celltype,
                    MPI_COMM_WORLD, // ensemble_comm があればそれを渡す
                    DEFAULT_NBINS,
                    particle_property,
                    average_range,
                    variance_range,
                    co_variation_range,
                    &histogram_log );
        }

        if ( ok )
        {
            {
#ifdef ENABLE_ENSEMBLE_TIMER
                EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerOutputHistory );
#endif
                OutputEnsembleStatisticHistory(
                        particle_property,
                        tf_number,
                        nvariables,
                        historyFilePath,
                        average_range,
                        variance_range,
                        co_variation_range,
                        MPI_COMM_WORLD );  // 空間領域方向のcomm を指定
            }
        }

    }
    else
    {
    TransferFunctionSynthesizer** th_tfs = new TransferFunctionSynthesizer*[max_threads];
//    std::vector< std::vector<vismodule::TransferFunction> > th_tf;

    //std::vector<vismodule::TransferFunction> transfer_functions( tf_number );
    std::vector<std::vector<vismodule::TransferFunction>> transfer_functions( max_threads );
    std::vector<std::vector<vismodule::TransferFunction>>           mean_transfer_functions( max_threads );
    std::vector<std::vector<vismodule::TransferFunction>>       variance_transfer_functions( max_threads );
    std::vector<std::vector<vismodule::TransferFunction>> coef_variation_transfer_functions( max_threads );
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerInitTransferFunctions );
#endif
        for ( int n = 0; n < max_threads; n++ )
        {
            th_tfs[n] = new TransferFunctionSynthesizer( *particle_property.m_transfunc_synthesizer );
        }

        for ( int i = 0; i < max_threads; i++ )
        {
            transfer_functions[ i ].resize( tf_number );
                      mean_transfer_functions[ i ].resize( tf_number );
                  variance_transfer_functions[ i ].resize( tf_number );
            coef_variation_transfer_functions[ i ].resize( tf_number );
            for ( int j = 0; j < tf_number; j++ )
            {
                transfer_functions[i][j] = particle_property.m_transfunc_array[j];
                          mean_transfer_functions[i][j] = particle_property.m_mean_transfer_function_array[j];
                      variance_transfer_functions[i][j] = particle_property.m_variance_transfer_function_array[j];
                coef_variation_transfer_functions[i][j] = particle_property.m_coefficient_of_variation_transfer_function_array[j];
            }
        }
    }

//    std::cout << "particle_property.mean_max = " << particle_property.m_mean_transfer_function_array[0].colorMap().maxValue() << std::endl;
//    std::cout << "particle_property.var_max = " << particle_property.m_variance_transfer_function_array[0].colorMap().maxValue() << std::endl;
//    std::cout << "particle_property.cov_max = " << particle_property.m_coefficient_of_variation_transfer_function_array[0].colorMap().maxValue() << std::endl;


    const std::string averageFilePrefix = EnsembleParticleFilePrefix( particleFilePrefix, "ave_" );
    const std::string varianceFilePrefix = EnsembleParticleFilePrefix( particleFilePrefix, "var_" );
    const std::string coefficientFilePrefix = EnsembleParticleFilePrefix( particleFilePrefix, "cov_" );

    std::vector<float> average_coords;
    std::vector<Byte> average_colors;
    std::vector<float> average_normals;
    std::vector<float> variance_coords;
    std::vector<Byte> variance_colors;
    std::vector<float> variance_normals;
    std::vector<float> coefficient_coords;
    std::vector<Byte> coefficient_colors;
    std::vector<float> coefficient_normals;

    std::vector<vismodule::UInt64> tmp_c_bins( DEFAULT_NBINS * tf_number, 0 );
    std::vector<vismodule::UInt64> tmp_o_bins( DEFAULT_NBINS * tf_number, 0 );
    std::vector<float> tmp_max( tf_number * 2, FLT_MIN );
    std::vector<float> tmp_min( tf_number * 2, FLT_MAX );

#ifndef CPU_VER
    if ( mpi_size <= 1 )
    {
        std::cout << "ensemble_generate_particles requires MPI ensemble ranks." << std::endl;
        delete particle_property.m_transfunc_synthesizer;
        delete particle_property.m_camera;
        return false;
    }

    std::vector<std::vector<vismodule::CellBase<Type>*> > cell( max_threads );
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerCreateCells );
#endif
        for ( int thread = 0; thread < max_threads; thread++ )
        {
            cell[thread].resize( nvariables, nullptr );
            for ( int variable = 0; variable < nvariables; variable++ )
            {
                switch ( celltype )
                {
                case vismodule::VolumeObjectBase::Tetrahedra:
                    cell[thread][variable] = new vismodule::TetrahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                case vismodule::VolumeObjectBase::Hexahedra:
                    cell[thread][variable] = new vismodule::HexahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                case vismodule::VolumeObjectBase::QuadraticTetrahedra:
                    cell[thread][variable] = new vismodule::QuadraticTetrahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                case vismodule::VolumeObjectBase::QuadraticHexahedra:
                    cell[thread][variable] = new vismodule::QuadraticHexahedralCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                case vismodule::VolumeObjectBase::Prism:
                    cell[thread][variable] = new vismodule::PrismaticCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                case vismodule::VolumeObjectBase::Pyramid:
                    cell[thread][variable] = new vismodule::PyramidalCell<Type>( values[variable], coordinates, ncoords, connections, ncells );
                    break;
                default:
                    std::cout << "Unsupported cell type." << std::endl;
                    for ( int i = 0; i <= thread; i++ )
                    {
                        for ( int j = 0; j < nvariables; j++ ) delete cell[i][j];
                    }
                    delete particle_property.m_transfunc_synthesizer;
                    delete particle_property.m_camera;
                    return false;
                }
            }
        }
    }

    float sampling_volume_inverse = 0.0f;
    float max_opacity = 0.0f;
    float max_density = 0.0f;
    float repetitions = particle_property.m_repeat_level;  //
    const float particle_density = 1.0f;
    const int MPIprocess_per_ensemble = mpi_size/num_ensemble;
    const int ens_number = num_ensemble;
    {  // 区間計測用の{}
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerSamplingPrepare );
#endif
        sampling_volume_inverse = particle_property.m_transfunc_synthesizer->getSamplingVolumeInverse();
        max_opacity = particle_property.m_transfunc_synthesizer->getMaxOpacity();
        max_density = particle_property.m_transfunc_synthesizer->getMaxDensity();
        if ( mpi_size % MPIprocess_per_ensemble != 0 )
        {
            std::cerr << "error !! need  ens_number % MPIprocess_per_ensemble = 0!!  " << std::endl;
            return false;
        } 
        repetitions /= static_cast<float>( ens_number );
    }
    std::vector<vismodule::Real32> vertex_coords;
    std::vector<vismodule::Real32> vertex_scalars;
    std::vector<vismodule::Real32> vertex_normals;
    std::vector<int> vertex_cellids;
    std::vector<vismodule::Real32> sq_scalars;
    std::vector<vismodule::Real32> tmp_term;



#ifdef ENABLE_ENSEMBLE_TIMER
    std::vector<double> uniform_thread_times( max_threads, 0.0 );
    std::vector<double> uniform_setup_times( max_threads, 0.0 );
    std::vector<double> uniform_cell_index_times( max_threads, 0.0 );
    std::vector<double> uniform_bind_times( max_threads, 0.0 );
    std::vector<double> uniform_volume_times( max_threads, 0.0 );
    std::vector<double> uniform_particle_count_times( max_threads, 0.0 );
    std::vector<double> uniform_sampling_loop_times( max_threads, 0.0 );
    std::vector<double> uniform_local_coord_times( max_threads, 0.0 );
    std::vector<double> uniform_flush_prepare_times( max_threads, 0.0 );
    std::vector<double> uniform_scalar_times( max_threads, 0.0 );
    std::vector<double> uniform_chain_times( max_threads, 0.0 );
    std::vector<double> uniform_calc_scalar_grad_times( max_threads, 0.0 );
    std::vector<double> uniform_q_grad_setup_times( max_threads, 0.0 );
    std::vector<double> uniform_tf_scalar_eval_times( max_threads, 0.0 );
    std::vector<double> uniform_chain_rule_dfdq_times( max_threads, 0.0 );
    std::vector<double> uniform_normal_normalize_times( max_threads, 0.0 );
    std::vector<double> uniform_store_times( max_threads, 0.0 );
    std::vector<double> uniform_merge_times( max_threads, 0.0 );
    vismodule::Timer uniform_timer;
    uniform_timer.start();
#endif
#pragma omp parallel
    {
#if _OPENMP
        const int nthreads = omp_get_num_threads();
        const int thid = omp_get_thread_num();
#else
        const int nthreads = 1;
        const int thid = 0;
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer thread_timer;
        thread_timer.start();
        vismodule::Timer setup_timer;
        setup_timer.start();
        double th_uniform_setup_time = 0.0;
        double th_uniform_cell_index_time = 0.0;
        double th_uniform_bind_time = 0.0;
        double th_uniform_volume_time = 0.0;
        double th_uniform_particle_count_time = 0.0;
        double th_uniform_sampling_loop_time = 0.0;
        double th_uniform_local_coord_time = 0.0;
        double th_uniform_flush_prepare_time = 0.0;
        double th_uniform_scalar_time = 0.0;
        double th_uniform_chain_time = 0.0;
        double th_uniform_calc_scalar_grad_time = 0.0;
        double th_uniform_q_grad_setup_time = 0.0;
        double th_uniform_tf_scalar_eval_time = 0.0;
        double th_uniform_chain_rule_dfdq_time = 0.0;
        double th_uniform_normal_normalize_time = 0.0;
        double th_uniform_store_time = 0.0;
        double th_uniform_merge_time = 0.0;
#endif
        vismodule::UInt32 cell_index[SIMD_BLK_SIZE];
        vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
        vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];
        float volume_array[SIMD_BLK_SIZE];
        int nparticles_array[SIMD_BLK_SIZE];
        float scalar_array[SIMD_BLK_SIZE];
        float grad_scalar[SIMD_BLK_SIZE];
        float grad_array_x[SIMD_BLK_SIZE];
        float grad_array_y[SIMD_BLK_SIZE];
        float grad_array_z[SIMD_BLK_SIZE];
        std::vector<float> o_scalars_array[SIMD_BLK_SIZE];
        for ( int i = 0; i < SIMD_BLK_SIZE; i++ ) o_scalars_array[i].resize( tf_number );
        ChainRuleEvalContext chain_context;
        chain_context.initialize( th_tfs[thid], nvariables );

        std::vector<vismodule::Real32> th_vertex_coords;
        std::vector<vismodule::Real32> th_vertex_scalars;
        std::vector<vismodule::Real32> th_vertex_normals;
        std::vector<int> th_vertex_cellids;
        std::vector<vismodule::Real32> th_sq_scalars;
        std::vector<vismodule::Real32> th_tmp_term;
        vismodule::MersenneTwister mt( thid + mpi_rank * nthreads );
#ifdef ENABLE_ENSEMBLE_TIMER
        setup_timer.stop();
        th_uniform_setup_time += setup_timer.sec();
#endif

#pragma omp for schedule( dynamic ) nowait
        for ( size_t index = 0; index < static_cast<size_t>( ncells ); index += SIMD_BLK_SIZE )
        {
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer cell_index_timer;
            cell_index_timer.start();
#endif
            const int remain = ( ncells - index > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : ncells - index;
            for ( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell_index[cell_BLK] = static_cast<vismodule::UInt32>( index + cell_BLK );
            }
#ifdef ENABLE_ENSEMBLE_TIMER
            cell_index_timer.stop();
            th_uniform_cell_index_time += cell_index_timer.sec();
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer bind_timer;
            bind_timer.start();
#endif
            bind_variables_scalars_opt( cell[thid], nvariables, remain, cell_index );
#ifdef ENABLE_ENSEMBLE_TIMER
            bind_timer.stop();
            th_uniform_bind_time += bind_timer.sec();
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer volume_timer;
            volume_timer.start();
#endif
#if defined( ENABLE_HEX_TET_VOLUME ) && !defined( PBVR_SCALAR_VOLUME )
            // Candidate 1: the cells were already loaded by bindCellArray() above, so
            // reuse m_vertices_array and compute every volume with one vectorizable call
            // instead of a per-cell virtual bindCell() (which blocked vectorization,
            // #15333) plus a redundant gather. Bit-identical (same 6-tet decomposition
            // and vertex order). Build -DPBVR_SCALAR_VOLUME to restore the old loop.
            static_cast<vismodule::HexahedralCell<Type>*>( cell[thid][0] )
                ->volumeArrayByTetraDecomposition( remain, volume_array );
#else
            for ( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                cell[thid][0]->bindCell( cell_index[cell_BLK] );
#if defined( ENABLE_HEX_TET_VOLUME )
                    volume_array[cell_BLK] =
                        static_cast<vismodule::HexahedralCell<Type>*>( cell[thid][0] )->volumeByTetraDecomposition();
#else
                volume_array[cell_BLK] = cell[thid][0]->volume();
#endif
            }
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
            volume_timer.stop();
            th_uniform_volume_time += volume_timer.sec();
#endif

#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer particle_count_timer;
            particle_count_timer.start();
#endif
            for ( int cell_BLK = 0; cell_BLK < remain; cell_BLK++ )
            {
                nparticles_array[cell_BLK] = static_cast<int>(
                    CalculateNumberOfParticlesV35( max_density, volume_array[cell_BLK], repetitions, &mt )
//                    5
                );
            }
#ifdef ENABLE_ENSEMBLE_TIMER
            particle_count_timer.stop();
            th_uniform_particle_count_time += particle_count_timer.sec();
#endif

            int p_id = 0;
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer sampling_loop_timer;
            sampling_loop_timer.start();
#endif
            for ( int cell_BLK = 0; cell_BLK < remain + 1; cell_BLK++ )
            {
                const int nparticles_in_cell = cell_BLK < remain ? nparticles_array[cell_BLK] : 1;
                for ( int i = 0; i < nparticles_in_cell; i += SIMD_BLK_SIZE )
                {
                    const int remain_BLK = ( nparticles_in_cell - i > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : nparticles_in_cell - i;
                    if ( cell_BLK < remain )
                    {
#ifdef ENABLE_ENSEMBLE_TIMER
                        vismodule::Timer local_coord_timer;
                        local_coord_timer.start();
#endif
                        for ( int j = 0; j < remain_BLK; j++ )
                        {
                            cell_index[p_id] = static_cast<vismodule::UInt32>( index + cell_BLK );
                            local_coord_array[p_id] = cell[thid][0]->randomSampling_MT( &mt );
//                            local_coord_array[p_id] = vismodule::Vector3f (0,0,0);
                            p_id++;
                            if ( p_id == SIMD_BLK_SIZE )
                            {
#ifdef ENABLE_ENSEMBLE_TIMER
                                local_coord_timer.stop();
                                th_uniform_local_coord_time += local_coord_timer.sec();
                                vismodule::Timer flush_prepare_timer;
                                flush_prepare_timer.start();
#endif
                                bind_variables_scalars_opt( cell[thid], nvariables, p_id, cell_index );
                                cell[thid][0]->setLocalPointArray( p_id, local_coord_array );
                                cell[thid][0]->transformLocalToGlobalArray( p_id, local_coord_array, global_coord_array );
#ifdef ENABLE_ENSEMBLE_TIMER
                                flush_prepare_timer.stop();
                                th_uniform_flush_prepare_time += flush_prepare_timer.sec();
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
                                vismodule::Timer scalar_timer;
                                scalar_timer.start();
                                ChainRuleTimingBreakdown chain_rule_timing;
#endif
	                                calculate_scalar_and_chain_rule_grad(
	                                    p_id,
	                                    nvariables,
	                                    chain_context,
	                                    cell[thid],
	                                    local_coord_array,
	                                    global_coord_array,
	                                    cell_index,
	                                    scalar_array,
	                                    grad_array_x,
	                                    grad_array_y,
	                                    grad_array_z,
#ifdef ENABLE_ENSEMBLE_TIMER
	                                    &chain_rule_timing
#else
	                                    0
#endif
	                                     );
#ifdef ENABLE_ENSEMBLE_TIMER
                                scalar_timer.stop();
                                th_uniform_scalar_time += scalar_timer.sec();
                                th_uniform_calc_scalar_grad_time += chain_rule_timing.calc_scalar_grad;
                                th_uniform_q_grad_setup_time += chain_rule_timing.q_grad_setup;
                                th_uniform_tf_scalar_eval_time += chain_rule_timing.tf_scalar_eval;
                                th_uniform_chain_rule_dfdq_time += chain_rule_timing.chain_rule_dfdq;
                                th_uniform_chain_time += chain_rule_timing.chain_rule_dfdq;
                                th_uniform_normal_normalize_time += chain_rule_timing.normal_normalize;
#endif
//                                calculation_glad(p_id, nvariables, th_tfs[thid], transfer_functions[thid], cell[thid], local_coord_array, cell_index, grad_array_x, grad_array_y, grad_array_z);

#ifdef ENABLE_ENSEMBLE_TIMER
                                vismodule::Timer store_timer;
                                store_timer.start();
#endif
                                ReserveAdditionalUniformParticles(
                                    static_cast<size_t>( p_id ),
                                    th_vertex_coords,
	                                    th_vertex_scalars,
	                                    th_vertex_normals,
	                                    th_vertex_cellids,
	                                    th_sq_scalars,
	                                    th_tmp_term );
	                                const size_t scalar_offset = th_vertex_scalars.size();
	                                const size_t vector_offset = th_vertex_coords.size();
	                                th_vertex_scalars.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                                th_vertex_cellids.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                                th_sq_scalars.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                                th_vertex_coords.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                                th_vertex_normals.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                                th_tmp_term.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                                for ( int k = 0; k < p_id; k++ )
	                                {
	                                    const size_t s = scalar_offset + static_cast<size_t>( k );
	                                    const size_t v = vector_offset + 3 * static_cast<size_t>( k );
	                                    th_vertex_scalars[s] = scalar_array[k];
	                                    th_vertex_coords[v] = local_coord_array[k].x();
	                                    th_vertex_coords[v + 1] = local_coord_array[k].y();
	                                    th_vertex_coords[v + 2] = local_coord_array[k].z();
	                                    th_vertex_cellids[s] = cell_index[k];
	                                    th_vertex_normals[v] = -grad_array_x[k];
	                                    th_vertex_normals[v + 1] = -grad_array_y[k];
	                                    th_vertex_normals[v + 2] = -grad_array_z[k];
	                                    th_sq_scalars[s] = scalar_array[k] * scalar_array[k];
	                                    th_tmp_term[v] = scalar_array[k] * grad_array_x[k];
	                                    th_tmp_term[v + 1] = scalar_array[k] * grad_array_y[k];
	                                    th_tmp_term[v + 2] = scalar_array[k] * grad_array_z[k];
	                                }
#ifdef ENABLE_ENSEMBLE_TIMER
                                store_timer.stop();
                                th_uniform_store_time += store_timer.sec();
#endif
                                p_id = 0;
#ifdef ENABLE_ENSEMBLE_TIMER
                                local_coord_timer.start();
#endif
                            }
                        }
#ifdef ENABLE_ENSEMBLE_TIMER
                        local_coord_timer.stop();
                        th_uniform_local_coord_time += local_coord_timer.sec();
#endif
                    }
                    else if ( p_id > 0 )
                    {
#ifdef ENABLE_ENSEMBLE_TIMER
                        vismodule::Timer flush_prepare_timer;
                        flush_prepare_timer.start();
#endif
                        bind_variables_scalars_opt( cell[thid], nvariables, p_id, cell_index );
                        cell[thid][0]->setLocalPointArray( p_id, local_coord_array );
                        cell[thid][0]->transformLocalToGlobalArray( p_id, local_coord_array, global_coord_array );
#ifdef ENABLE_ENSEMBLE_TIMER
                        flush_prepare_timer.stop();
                        th_uniform_flush_prepare_time += flush_prepare_timer.sec();
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
                        vismodule::Timer scalar_timer;
                        scalar_timer.start();
                        ChainRuleTimingBreakdown chain_rule_timing;
#endif
	                        calculate_scalar_and_chain_rule_grad(
	                            p_id,
	                            nvariables,
	                            chain_context,
	                            cell[thid],
	                            local_coord_array,
	                            global_coord_array,
	                            cell_index,
	                            scalar_array,
	                            grad_array_x,
	                            grad_array_y,
	                            grad_array_z,
#ifdef ENABLE_ENSEMBLE_TIMER
	                            &chain_rule_timing
#else
	                            0
#endif
	                             );
#ifdef ENABLE_ENSEMBLE_TIMER
                        scalar_timer.stop();
                        th_uniform_scalar_time += scalar_timer.sec();
                        th_uniform_calc_scalar_grad_time += chain_rule_timing.calc_scalar_grad;
                        th_uniform_q_grad_setup_time += chain_rule_timing.q_grad_setup;
                        th_uniform_tf_scalar_eval_time += chain_rule_timing.tf_scalar_eval;
                        th_uniform_chain_rule_dfdq_time += chain_rule_timing.chain_rule_dfdq;
                        th_uniform_chain_time += chain_rule_timing.chain_rule_dfdq;
                        th_uniform_normal_normalize_time += chain_rule_timing.normal_normalize;
#endif
//                        calculation_glad(p_id, nvariables, th_tfs[thid], transfer_functions[thid], cell[thid], local_coord_array, cell_index, grad_array_x, grad_array_y, grad_array_z);


#ifdef ENABLE_ENSEMBLE_TIMER
                        vismodule::Timer store_timer;
                        store_timer.start();
#endif
                        ReserveAdditionalUniformParticles(
                            static_cast<size_t>( p_id ),
                            th_vertex_coords,
	                            th_vertex_scalars,
	                            th_vertex_normals,
	                            th_vertex_cellids,
	                            th_sq_scalars,
	                            th_tmp_term );
	                        const size_t scalar_offset = th_vertex_scalars.size();
	                        const size_t vector_offset = th_vertex_coords.size();
	                        th_vertex_scalars.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                        th_vertex_cellids.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                        th_sq_scalars.resize( scalar_offset + static_cast<size_t>( p_id ) );
	                        th_vertex_coords.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                        th_vertex_normals.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                        th_tmp_term.resize( vector_offset + 3 * static_cast<size_t>( p_id ) );
	                        for ( int k = 0; k < p_id; k++ )
	                        {
	                            const size_t s = scalar_offset + static_cast<size_t>( k );
	                            const size_t v = vector_offset + 3 * static_cast<size_t>( k );
	                            th_vertex_scalars[s] = scalar_array[k];
	                            th_vertex_coords[v] = local_coord_array[k].x();
	                            th_vertex_coords[v + 1] = local_coord_array[k].y();
	                            th_vertex_coords[v + 2] = local_coord_array[k].z();
	                            th_vertex_cellids[s] = cell_index[k];
	                            th_vertex_normals[v] = -grad_array_x[k];
	                            th_vertex_normals[v + 1] = -grad_array_y[k];
	                            th_vertex_normals[v + 2] = -grad_array_z[k];
	                            th_sq_scalars[s] = scalar_array[k] * scalar_array[k];
	                            th_tmp_term[v] = scalar_array[k] * grad_array_x[k];
	                            th_tmp_term[v + 1] = scalar_array[k] * grad_array_y[k];
	                            th_tmp_term[v + 2] = scalar_array[k] * grad_array_z[k];
	                        }
#ifdef ENABLE_ENSEMBLE_TIMER
                        store_timer.stop();
                        th_uniform_store_time += store_timer.sec();
#endif
                        p_id = 0;
                    }
                }
            }
#ifdef ENABLE_ENSEMBLE_TIMER
            sampling_loop_timer.stop();
            th_uniform_sampling_loop_time += sampling_loop_timer.sec();
#endif
        }

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer merge_timer;
        merge_timer.start();
#endif
#pragma omp critical
        {
            vertex_coords.reserve( vertex_coords.size() + th_vertex_coords.size() );
            vertex_scalars.reserve( vertex_scalars.size() + th_vertex_scalars.size() );
            vertex_normals.reserve( vertex_normals.size() + th_vertex_normals.size() );
            vertex_cellids.reserve( vertex_cellids.size() + th_vertex_cellids.size() );
            sq_scalars.reserve( sq_scalars.size() + th_sq_scalars.size() );
            tmp_term.reserve( tmp_term.size() + th_tmp_term.size() );
            vertex_coords.insert( vertex_coords.end(), th_vertex_coords.begin(), th_vertex_coords.end() );
            vertex_scalars.insert( vertex_scalars.end(), th_vertex_scalars.begin(), th_vertex_scalars.end() );
            vertex_normals.insert( vertex_normals.end(), th_vertex_normals.begin(), th_vertex_normals.end() );
            vertex_cellids.insert( vertex_cellids.end(), th_vertex_cellids.begin(), th_vertex_cellids.end() );
            sq_scalars.insert( sq_scalars.end(), th_sq_scalars.begin(), th_sq_scalars.end() );
            tmp_term.insert( tmp_term.end(), th_tmp_term.begin(), th_tmp_term.end() );
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        merge_timer.stop();
        th_uniform_merge_time += merge_timer.sec();
        thread_timer.stop();
        uniform_thread_times[thid] += thread_timer.sec();
        uniform_setup_times[thid] += th_uniform_setup_time;
        uniform_cell_index_times[thid] += th_uniform_cell_index_time;
        uniform_bind_times[thid] += th_uniform_bind_time;
        uniform_volume_times[thid] += th_uniform_volume_time;
        uniform_particle_count_times[thid] += th_uniform_particle_count_time;
        uniform_sampling_loop_times[thid] += th_uniform_sampling_loop_time;
        uniform_local_coord_times[thid] += th_uniform_local_coord_time;
        uniform_flush_prepare_times[thid] += th_uniform_flush_prepare_time;
        uniform_scalar_times[thid] += th_uniform_scalar_time;
        uniform_chain_times[thid] += th_uniform_chain_time;
        uniform_calc_scalar_grad_times[thid] += th_uniform_calc_scalar_grad_time;
        uniform_q_grad_setup_times[thid] += th_uniform_q_grad_setup_time;
        uniform_tf_scalar_eval_times[thid] += th_uniform_tf_scalar_eval_time;
        uniform_chain_rule_dfdq_times[thid] += th_uniform_chain_rule_dfdq_time;
        uniform_normal_normalize_times[thid] += th_uniform_normal_normalize_time;
        uniform_store_times[thid] += th_uniform_store_time;
        uniform_merge_times[thid] += th_uniform_merge_time;
#endif
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    uniform_timer.stop();
    ensemble_timer.add( EnsembleTimerOmpUniformSampling, uniform_timer.sec() );
    for ( int t = 0; t < max_threads; t++ )
    {
        ensemble_timer.addThread( EnsembleTimerOmpUniformSampling, t, uniform_thread_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformThreadSetup, t, uniform_setup_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformCellIndexSetup, t, uniform_cell_index_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformBindCellArray, t, uniform_bind_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformVolumeCalculation, t, uniform_volume_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformParticleCountCalculation, t, uniform_particle_count_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformParticleSamplingLoop, t, uniform_sampling_loop_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformLocalCoordGeneration, t, uniform_local_coord_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformFlushPrepare, t, uniform_flush_prepare_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformCalculateScalars, t, uniform_scalar_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformChainRuleGrad, t, uniform_chain_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformCalcScalarGrad, t, uniform_calc_scalar_grad_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformQGradSetup, t, uniform_q_grad_setup_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformTfScalarEval, t, uniform_tf_scalar_eval_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformChainRuleDfdq, t, uniform_chain_rule_dfdq_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformNormalNormalize, t, uniform_normal_normalize_times[t] );
        ensemble_timer.addThread( EnsembleTimerUniformStoreParticleData, t, uniform_store_times[t] );
        ensemble_timer.addThread( EnsembleTimerThreadParticleMerge, t, uniform_merge_times[t] );
    }
    ensemble_timer.setUniformParticleCount(
        static_cast<unsigned long long>( vertex_coords.size() / 3 ) );
#endif

    std::vector<std::vector<float> > v_scalars( 2 );
    std::vector<std::vector<float> > v_coords( 2 );
    std::vector<std::vector<float> > v_normals( 2 );
    std::vector<std::vector<int> > v_cellids( 2 );
    std::vector<std::vector<float> > v_sq( 2 );
    std::vector<std::vector<float> > v_tmp( 2 );
    v_scalars[0].swap( vertex_scalars );
    v_coords[0].swap( vertex_coords );
    v_normals[0].swap( vertex_normals );
    v_cellids[0].swap( vertex_cellids );
    v_sq[0].swap( sq_scalars );
    v_tmp[0].swap( tmp_term );

    int cur = 0;
    int nxt = 1;
    const bool use_stable_ensemble_comm = UseStableEnsembleCommunication();
#ifndef CPU_VER
    if ( mpi_rank == 0 && use_stable_ensemble_comm )
    {
        std::cout << "PBVR_ENSEMBLE_STABLE_COMM=1: use MPI_Sendrecv for ensemble exchange." << std::endl;
    }
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
    std::vector<double> shift_interp_thread_times( max_threads, 0.0 );
    std::vector<double> shift_scalar_thread_times( max_threads, 0.0 );
    std::vector<double> shift_chain_thread_times( max_threads, 0.0 );
    std::vector<double> shift_calc_scalar_grad_times( max_threads, 0.0 );
    std::vector<double> shift_q_grad_setup_times( max_threads, 0.0 );
    std::vector<double> shift_tf_scalar_eval_times( max_threads, 0.0 );
    std::vector<double> shift_chain_rule_dfdq_times( max_threads, 0.0 );
    std::vector<double> shift_normal_normalize_times( max_threads, 0.0 );
    vismodule::Timer mpi_shift_timer;
    mpi_shift_timer.start();
#endif
    for ( int shift = 1; shift < ens_number; shift++ )
    {
        const int send_to = ( mpi_rank + MPIprocess_per_ensemble ) % mpi_size;
        const int recv_from = ( mpi_rank - MPIprocess_per_ensemble + mpi_size ) % mpi_size;
        const size_t send_count = v_scalars[cur].size();
        if ( send_count > static_cast<size_t>( INT_MAX ) ||
             send_count > static_cast<size_t>( INT_MAX / 3 ) ||
             v_coords[cur].size() != 3 * send_count ||
             v_normals[cur].size() != 3 * send_count ||
             v_cellids[cur].size() != send_count ||
             v_sq[cur].size() != send_count ||
             v_tmp[cur].size() != 3 * send_count )
        {
            std::cerr << "Invalid ensemble exchange send buffer size at rank " << mpi_rank
                      << ": scalars=" << v_scalars[cur].size()
                      << ", coords=" << v_coords[cur].size()
                      << ", normals=" << v_normals[cur].size()
                      << ", cellids=" << v_cellids[cur].size()
                      << ", sq=" << v_sq[cur].size()
                      << ", tmp=" << v_tmp[cur].size()
                      << std::endl;
            return false;
        }
        const int send_size = static_cast<int>( send_count );
        int recv_size = 0;
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer mpi_size_timer;
        mpi_size_timer.start();
#endif
        if ( use_stable_ensemble_comm )
        {
            MPI_Sendrecv(
                &send_size, 1, MPI_INT, send_to, 0,
                &recv_size, 1, MPI_INT, recv_from, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );
        }
        else
        {
            MPI_Request reqs[2];
            MPI_Isend( &send_size, 1, MPI_INT, send_to, 0, MPI_COMM_WORLD, &reqs[0] );
            MPI_Irecv( &recv_size, 1, MPI_INT, recv_from, 0, MPI_COMM_WORLD, &reqs[1] );
            MPI_Waitall( 2, reqs, MPI_STATUSES_IGNORE );
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        mpi_size_timer.stop();
        ensemble_timer.add( EnsembleTimerMpiShiftSizeExchange, mpi_size_timer.sec() );
#endif
        if ( recv_size < 0 || recv_size > INT_MAX / 3 )
        {
            std::cerr << "Invalid ensemble exchange receive size at rank " << mpi_rank
                      << ": recv_size=" << recv_size << std::endl;
            return false;
        }

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer alloc_timer;
        alloc_timer.start();
#endif
        std::vector<float> recv_scalars( recv_size );
        std::vector<float> recv_coords( 3 * recv_size );
        std::vector<float> recv_normals( 3 * recv_size );
        std::vector<int> recv_cellids( recv_size );
        std::vector<float> recv_sq_scalars( recv_size );
        std::vector<float> recv_tmp_term( 3 * recv_size );
#ifdef ENABLE_ENSEMBLE_TIMER
        alloc_timer.stop();
        ensemble_timer.add( EnsembleTimerMpiShiftAllocRecvBuffer, alloc_timer.sec() );
#endif
        if ( use_stable_ensemble_comm )
        {
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer payload_timer;
            payload_timer.start();
#endif
            MPI_Sendrecv(
                v_cellids[cur].data(), send_size, MPI_INT, send_to, 12,
                recv_cellids.data(), recv_size, MPI_INT, recv_from, 12,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );
#ifdef ENABLE_ENSEMBLE_TIMER
            payload_timer.stop();
            ensemble_timer.add( EnsembleTimerMpiShiftPayloadCellids, payload_timer.sec() );
            payload_timer.start();
#endif
            MPI_Sendrecv(
                v_scalars[cur].data(), send_size, MPI_FLOAT, send_to, 10,
                recv_scalars.data(), recv_size, MPI_FLOAT, recv_from, 10,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );
#ifdef ENABLE_ENSEMBLE_TIMER
            payload_timer.stop();
            ensemble_timer.add( EnsembleTimerMpiShiftPayloadScalars, payload_timer.sec() );
            payload_timer.start();
#endif
            MPI_Sendrecv(
                v_coords[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 11,
                recv_coords.data(), 3 * recv_size, MPI_FLOAT, recv_from, 11,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );
#ifdef ENABLE_ENSEMBLE_TIMER
            payload_timer.stop();
            ensemble_timer.add( EnsembleTimerMpiShiftPayloadCoords, payload_timer.sec() );
            payload_timer.start();
#endif
            MPI_Sendrecv(
                v_normals[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 13,
                recv_normals.data(), 3 * recv_size, MPI_FLOAT, recv_from, 13,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );
#ifdef ENABLE_ENSEMBLE_TIMER
            payload_timer.stop();
            ensemble_timer.add( EnsembleTimerMpiShiftPayloadNormals, payload_timer.sec() );
            payload_timer.start();
#endif
            MPI_Sendrecv(
                v_sq[cur].data(), send_size, MPI_FLOAT, send_to, 14,
                recv_sq_scalars.data(), recv_size, MPI_FLOAT, recv_from, 14,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );
#ifdef ENABLE_ENSEMBLE_TIMER
            payload_timer.stop();
            ensemble_timer.add( EnsembleTimerMpiShiftPayloadSq, payload_timer.sec() );
            payload_timer.start();
#endif
            MPI_Sendrecv(
                v_tmp[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 15,
                recv_tmp_term.data(), 3 * recv_size, MPI_FLOAT, recv_from, 15,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE
            );
#ifdef ENABLE_ENSEMBLE_TIMER
            payload_timer.stop();
            ensemble_timer.add( EnsembleTimerMpiShiftPayloadTmp, payload_timer.sec() );
#endif
        }
        else
        {
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer payload_all_timer;
            payload_all_timer.start();
#endif
            MPI_Request req_recv[6];
            MPI_Request req_send[6];
            MPI_Irecv( recv_cellids.data(), recv_size, MPI_INT, recv_from, 12, MPI_COMM_WORLD, &req_recv[0] );
            MPI_Irecv( recv_scalars.data(), recv_size, MPI_FLOAT, recv_from, 10, MPI_COMM_WORLD, &req_recv[1] );
            MPI_Irecv( recv_coords.data(), 3 * recv_size, MPI_FLOAT, recv_from, 11, MPI_COMM_WORLD, &req_recv[2] );
            MPI_Irecv( recv_normals.data(), 3 * recv_size, MPI_FLOAT, recv_from, 13, MPI_COMM_WORLD, &req_recv[3] );
            MPI_Irecv( recv_sq_scalars.data(), recv_size, MPI_FLOAT, recv_from, 14, MPI_COMM_WORLD, &req_recv[4] );
            MPI_Irecv( recv_tmp_term.data(), 3 * recv_size, MPI_FLOAT, recv_from, 15, MPI_COMM_WORLD, &req_recv[5] );
            MPI_Isend( v_cellids[cur].data(), send_size, MPI_INT, send_to, 12, MPI_COMM_WORLD, &req_send[0] );
            MPI_Isend( v_scalars[cur].data(), send_size, MPI_FLOAT, send_to, 10, MPI_COMM_WORLD, &req_send[1] );
            MPI_Isend( v_coords[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 11, MPI_COMM_WORLD, &req_send[2] );
            MPI_Isend( v_normals[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 13, MPI_COMM_WORLD, &req_send[3] );
            MPI_Isend( v_sq[cur].data(), send_size, MPI_FLOAT, send_to, 14, MPI_COMM_WORLD, &req_send[4] );
            MPI_Isend( v_tmp[cur].data(), 3 * send_size, MPI_FLOAT, send_to, 15, MPI_COMM_WORLD, &req_send[5] );
            MPI_Waitall( 6, req_recv, MPI_STATUSES_IGNORE );
            MPI_Waitall( 6, req_send, MPI_STATUSES_IGNORE );
#ifdef ENABLE_ENSEMBLE_TIMER
            payload_all_timer.stop();
            ensemble_timer.add( EnsembleTimerMpiShiftPayloadAll, payload_all_timer.sec() );
#endif
        }

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer shift_interp_timer;
        shift_interp_timer.start();
#endif
#pragma omp parallel
        {
#if _OPENMP
            const int thid = omp_get_thread_num();
#else
            const int thid = 0;
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
            vismodule::Timer thread_timer;
            thread_timer.start();
            double th_shift_scalar_time = 0.0;
            double th_shift_chain_time = 0.0;
            double th_shift_calc_scalar_grad_time = 0.0;
            double th_shift_q_grad_setup_time = 0.0;
            double th_shift_tf_scalar_eval_time = 0.0;
            double th_shift_chain_rule_dfdq_time = 0.0;
            double th_shift_normal_normalize_time = 0.0;
#endif
            vismodule::UInt32 cell_index[SIMD_BLK_SIZE];
            vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
            vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];
            std::vector<float> o_scalars_array[SIMD_BLK_SIZE];
            float scalar_array[SIMD_BLK_SIZE];
            float grad_scalar[SIMD_BLK_SIZE];
            float grad_array_x[SIMD_BLK_SIZE];
            float grad_array_y[SIMD_BLK_SIZE];
            float grad_array_z[SIMD_BLK_SIZE];
            for ( int i = 0; i < SIMD_BLK_SIZE; i++ ) o_scalars_array[i].resize( tf_number );
            ChainRuleEvalContext chain_context;
            chain_context.initialize( th_tfs[thid], nvariables );

//#pragma omp for schedule( dynamic )
#pragma omp for 
            for ( int i = 0; i < recv_size; i += SIMD_BLK_SIZE )
            {
                const int remain_BLK = ( recv_size - i > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : recv_size - i;
                for ( int j = 0; j < remain_BLK; j++ )
                {
                    cell_index[j] = static_cast<vismodule::UInt32>( recv_cellids[i + j] );
                    local_coord_array[j] = vismodule::Vector3f(
                        recv_coords[3 * ( i + j )],
                        recv_coords[3 * ( i + j ) + 1],
                        recv_coords[3 * ( i + j ) + 2]
                    );
                }
                bind_variables_scalars_opt( cell[thid], nvariables, remain_BLK, cell_index );
                cell[thid][0]->setLocalPointArray( remain_BLK, local_coord_array );
                cell[thid][0]->transformLocalToGlobalArray( remain_BLK, local_coord_array, global_coord_array );
#ifdef ENABLE_ENSEMBLE_TIMER
                vismodule::Timer scalar_timer;
                scalar_timer.start();
                ChainRuleTimingBreakdown chain_rule_timing;
#endif
	                calculate_scalar_and_chain_rule_grad(
	                    remain_BLK,
	                    nvariables,
	                    chain_context,
	                    cell[thid],
	                    local_coord_array,
	                    global_coord_array,
	                    cell_index,
	                    scalar_array,
	                    grad_array_x,
	                    grad_array_y,
	                    grad_array_z,
#ifdef ENABLE_ENSEMBLE_TIMER
	                    &chain_rule_timing
#else
	                    0
#endif
	                     );
#ifdef ENABLE_ENSEMBLE_TIMER
                scalar_timer.stop();
                th_shift_scalar_time += scalar_timer.sec();
                th_shift_calc_scalar_grad_time += chain_rule_timing.calc_scalar_grad;
                th_shift_q_grad_setup_time += chain_rule_timing.q_grad_setup;
                th_shift_tf_scalar_eval_time += chain_rule_timing.tf_scalar_eval;
                th_shift_chain_rule_dfdq_time += chain_rule_timing.chain_rule_dfdq;
                th_shift_chain_time += chain_rule_timing.chain_rule_dfdq;
                th_shift_normal_normalize_time += chain_rule_timing.normal_normalize;
#endif
//                calculation_glad(remain_BLK, nvariables, th_tfs[thid], transfer_functions[thid], cell[thid], local_coord_array, cell_index, grad_array_x, grad_array_y, grad_array_z);


                for ( int j = 0; j < remain_BLK; j++ )
                {
                    const float scalar = scalar_array[j];
                    recv_scalars[i + j] += scalar;
                    recv_normals[3 * ( i + j )]     += -grad_array_x[j];
                    recv_normals[3 * ( i + j ) + 1] += -grad_array_y[j];
                    recv_normals[3 * ( i + j ) + 2] += -grad_array_z[j];
                    recv_sq_scalars[i + j] += scalar * scalar;
                    recv_tmp_term[3 * ( i + j )] += scalar * grad_array_x[j];
                    recv_tmp_term[3 * ( i + j ) + 1] += scalar * grad_array_y[j];
                    recv_tmp_term[3 * ( i + j ) + 2] += scalar * grad_array_z[j];
                }
            }
#ifdef ENABLE_ENSEMBLE_TIMER
            thread_timer.stop();
            shift_interp_thread_times[thid] += thread_timer.sec();
            shift_scalar_thread_times[thid] += th_shift_scalar_time;
            shift_chain_thread_times[thid] += th_shift_chain_time;
            shift_calc_scalar_grad_times[thid] += th_shift_calc_scalar_grad_time;
            shift_q_grad_setup_times[thid] += th_shift_q_grad_setup_time;
            shift_tf_scalar_eval_times[thid] += th_shift_tf_scalar_eval_time;
            shift_chain_rule_dfdq_times[thid] += th_shift_chain_rule_dfdq_time;
            shift_normal_normalize_times[thid] += th_shift_normal_normalize_time;
#endif
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        shift_interp_timer.stop();
        ensemble_timer.add( EnsembleTimerOmpShiftInterpolation, shift_interp_timer.sec() );
#endif

        v_scalars[nxt].swap( recv_scalars );
        v_coords[nxt].swap( recv_coords );
        v_normals[nxt].swap( recv_normals );
        v_cellids[nxt].swap( recv_cellids );
        v_sq[nxt].swap( recv_sq_scalars );
        v_tmp[nxt].swap( recv_tmp_term );
        std::swap( cur, nxt );
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    mpi_shift_timer.stop();
    ensemble_timer.add( EnsembleTimerMpiShiftExchange, mpi_shift_timer.sec() );
    for ( int t = 0; t < max_threads; t++ )
    {
        ensemble_timer.addThread( EnsembleTimerOmpShiftInterpolation, t, shift_interp_thread_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftCalculateScalars, t, shift_scalar_thread_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftChainRuleGrad, t, shift_chain_thread_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftCalcScalarGrad, t, shift_calc_scalar_grad_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftQGradSetup, t, shift_q_grad_setup_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftTfScalarEval, t, shift_tf_scalar_eval_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftChainRuleDfdq, t, shift_chain_rule_dfdq_times[t] );
        ensemble_timer.addThread( EnsembleTimerShiftNormalNormalize, t, shift_normal_normalize_times[t] );
    }
#endif

    vertex_scalars.swap( v_scalars[cur] );
    vertex_coords.swap( v_coords[cur] );
    vertex_normals.swap( v_normals[cur] );
    vertex_cellids.swap( v_cellids[cur] );
    sq_scalars.swap( v_sq[cur] );
    tmp_term.swap( v_tmp[cur] );

    std::vector<float> tmp_varience( vertex_scalars.size() );
    std::vector<float> tmp_varience_normals( 3 * vertex_scalars.size() );
    std::vector<float> co_varietion( vertex_scalars.size() );
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerStatAverageVariance );
#endif
    const float invert_num = 1.0f / static_cast<float>( ens_number );
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        vertex_scalars[i] *= invert_num;
        sq_scalars[i] *= invert_num;
    }
    for ( size_t i = 0; i < tmp_term.size(); i++ )
    {
        tmp_term[i] = -2.0f * invert_num * tmp_term[i];
        vertex_normals[i] *= -invert_num;
    }
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        tmp_varience[i] = sq_scalars[i] - vertex_scalars[i] * vertex_scalars[i];
        if ( tmp_varience[i] < 0.0f ) tmp_varience[i] = 0.0f;
        tmp_varience_normals[3 * i] = tmp_term[3 * i] - ( -2.0f  * vertex_scalars[i] * vertex_normals[3 * i] );
        tmp_varience_normals[3 * i + 1] = tmp_term[3 * i + 1] - ( -2.0f * vertex_scalars[i] * vertex_normals[3 * i + 1] );
        tmp_varience_normals[3 * i + 2] = tmp_term[3 * i + 2] - ( -2.0f * vertex_scalars[i] * vertex_normals[3 * i + 2] );
    }

    const float delta = 1.0e-30f;
    const float eps = 1.0e-5f;
    for ( size_t i = 0; i < vertex_scalars.size(); i++ )
    {
        co_varietion[i] = vertex_scalars[i] > eps ? std::sqrt( tmp_varience[i] ) / vertex_scalars[i] : delta;
    }
    }

    EnsembleStatisticRange average_range;
    EnsembleStatisticRange variance_range;
    EnsembleStatisticRange co_variation_range;
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerStatHistogram );
#endif
        average_range = MakeEnsembleStatisticRange(
            vertex_scalars, tf_number, particle_property.m_mean_transfer_function_array
        );
        variance_range = MakeEnsembleStatisticRange(
            tmp_varience, tf_number, particle_property.m_variance_transfer_function_array
        );
        co_variation_range = MakeEnsembleStatisticRange(
            co_varietion, tf_number, particle_property.m_coefficient_of_variation_transfer_function_array
        );

        particle_property.m_transfunc_synthesizer->m_o_min.resize( tf_number );
        particle_property.m_transfunc_synthesizer->m_o_max.resize( tf_number );
        particle_property.m_transfunc_synthesizer->m_c_min.resize( tf_number );
        particle_property.m_transfunc_synthesizer->m_c_max.resize( tf_number );
        for ( int i = 0; i < tf_number; i++ )
        {
            particle_property.m_transfunc_synthesizer->m_o_min[i] = average_range.min_values[2 * i    ];
            particle_property.m_transfunc_synthesizer->m_o_max[i] = average_range.max_values[2 * i    ];
            particle_property.m_transfunc_synthesizer->m_c_min[i] = average_range.min_values[2 * i + 1];
            particle_property.m_transfunc_synthesizer->m_c_max[i] = average_range.max_values[2 * i + 1];
        }
    }

#ifdef ENABLE_ENSEMBLE_TIMER
    std::vector<double> rejection_thread_times( max_threads, 0.0 );
    std::vector<double> rejection_merge_times( max_threads, 0.0 );
    vismodule::Timer rejection_timer;
    rejection_timer.start();
#endif
#pragma omp parallel
    {
#if _OPENMP
        const int thid = omp_get_thread_num();
#else
        const int thid = 0;
#endif
#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer thread_timer;
        thread_timer.start();
        double th_rejection_merge_time = 0.0;
#endif
        std::vector<float> th_average_coords;
        std::vector<Byte> th_average_colors;
        std::vector<float> th_average_normals;
        std::vector<float> th_variance_coords;
        std::vector<Byte> th_variance_colors;
        std::vector<float> th_variance_normals;
        std::vector<float> th_coefficient_coords;
        std::vector<Byte> th_coefficient_colors;
        std::vector<float> th_coefficient_normals;
        vismodule::MersenneTwister mt( 10 + mpi_rank + thid );
        vismodule::UInt32 cell_index[SIMD_BLK_SIZE];
        vismodule::Vector3f local_coord_array[SIMD_BLK_SIZE];
        vismodule::Vector3f global_coord_array[SIMD_BLK_SIZE];

#pragma omp for schedule( dynamic )
        for ( size_t i = 0; i < vertex_scalars.size(); i += SIMD_BLK_SIZE )
        {
            const int remain_BLK = ( vertex_scalars.size() - i > SIMD_BLK_SIZE ) ? SIMD_BLK_SIZE : vertex_scalars.size() - i;
            for ( int j = 0; j < remain_BLK; j++ )
            {
                cell_index[j] = static_cast<vismodule::UInt32>( vertex_cellids[i + j] );
                local_coord_array[j] = vismodule::Vector3f(
                    vertex_coords[3 * ( i + j )],
                    vertex_coords[3 * ( i + j ) + 1],
                    vertex_coords[3 * ( i + j ) + 2]
                );
            }
            bind_variables_scalars_opt( cell[thid], nvariables, remain_BLK, cell_index );
            cell[thid][0]->setLocalPointArray( remain_BLK, local_coord_array );
            cell[thid][0]->transformLocalToGlobalArray( remain_BLK, local_coord_array, global_coord_array );

            for ( int j = 0; j < remain_BLK; j++ )
            {
                const size_t idx = i + j;
                const vismodule::Vector3f average_normal(
                    vertex_normals[3 * idx],
                    vertex_normals[3 * idx + 1],
                    vertex_normals[3 * idx + 2]
                );
                const vismodule::Vector3f variance_normal(
                    tmp_varience_normals[3 * idx],
                    tmp_varience_normals[3 * idx + 1],
                    tmp_varience_normals[3 * idx + 2]
                );

//                std::cout << "tmp_varience_normals = " << variance_normal <<std::endl;
                AppendRejectedStatisticParticle(
                    vertex_scalars[idx], global_coord_array[j], average_normal, mean_transfer_functions[thid][0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_average_coords, th_average_colors, th_average_normals
                );
                AppendRejectedStatisticParticle(
                    tmp_varience[idx], global_coord_array[j], variance_normal, variance_transfer_functions[thid][0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_variance_coords, th_variance_colors, th_variance_normals
                );
                AppendRejectedStatisticParticle(
                    co_varietion[idx], global_coord_array[j], variance_normal, coef_variation_transfer_functions[thid][0],
                    sampling_volume_inverse, max_opacity, max_density, &mt,
                    th_coefficient_coords, th_coefficient_colors, th_coefficient_normals
                );
            }
        }

#ifdef ENABLE_ENSEMBLE_TIMER
        vismodule::Timer merge_timer;
        merge_timer.start();
#endif
#pragma omp critical
        {
            average_coords.insert( average_coords.end(), th_average_coords.begin(), th_average_coords.end() );
            average_colors.insert( average_colors.end(), th_average_colors.begin(), th_average_colors.end() );
            average_normals.insert( average_normals.end(), th_average_normals.begin(), th_average_normals.end() );
            variance_coords.insert( variance_coords.end(), th_variance_coords.begin(), th_variance_coords.end() );
            variance_colors.insert( variance_colors.end(), th_variance_colors.begin(), th_variance_colors.end() );
            variance_normals.insert( variance_normals.end(), th_variance_normals.begin(), th_variance_normals.end() );
            coefficient_coords.insert( coefficient_coords.end(), th_coefficient_coords.begin(), th_coefficient_coords.end() );
            coefficient_colors.insert( coefficient_colors.end(), th_coefficient_colors.begin(), th_coefficient_colors.end() );
            coefficient_normals.insert( coefficient_normals.end(), th_coefficient_normals.begin(), th_coefficient_normals.end() );
        }
#ifdef ENABLE_ENSEMBLE_TIMER
        merge_timer.stop();
        th_rejection_merge_time += merge_timer.sec();
        thread_timer.stop();
        rejection_thread_times[thid] += thread_timer.sec();
        rejection_merge_times[thid] += th_rejection_merge_time;
#endif
    }
#ifdef ENABLE_ENSEMBLE_TIMER
    rejection_timer.stop();
    ensemble_timer.add( EnsembleTimerOmpRejection, rejection_timer.sec() );
    for ( int t = 0; t < max_threads; t++ )
    {
        ensemble_timer.addThread( EnsembleTimerOmpRejection, t, rejection_thread_times[t] );
        ensemble_timer.addThread( EnsembleTimerRejectionThreadMerge, t, rejection_merge_times[t] );
    }
    ensemble_timer.setStatisticParticleCounts(
        static_cast<unsigned long long>( average_coords.size() / 3 ),
        static_cast<unsigned long long>( variance_coords.size() / 3 ),
        static_cast<unsigned long long>( coefficient_coords.size() / 3 ) );
#endif
#else
    std::cout << "ensemble_generate_particles requires MPI; CPU_VER path is disabled." << std::endl;
    delete particle_property.m_transfunc_synthesizer;
    delete particle_property.m_camera;
    return false;
#endif


    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerCleanupTfs );
#endif
    for(int i=0; i<max_threads; i++)
    {
        delete th_tfs[i];
    }
    delete[] th_tfs;
    }

#ifndef CPU_VER
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerOutputCoordMinmax );
#endif
        OutputCoordMinMaxFile( dom, coordMinMaxFilePath );
    }

    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerOutputParticlesAve );
#endif
        OutputEnsembleStatisticParticles(
            particle_property, mvpl, time_step, averageFilePrefix,
            average_coords, average_colors, average_normals
        );
    }

    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerOutputParticlesVar );
#endif
        OutputEnsembleStatisticParticles(
            particle_property, mvpl, time_step, varianceFilePrefix,
            variance_coords, variance_colors, variance_normals
        );
    }

    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerOutputParticlesCov );
#endif
        OutputEnsembleStatisticParticles(
            particle_property, mvpl, time_step, coefficientFilePrefix,
            coefficient_coords, coefficient_colors, coefficient_normals
        );
    }
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerOutputHistory );
#endif
        OutputEnsembleStatisticHistory(
            particle_property, tf_number, nvariables, historyFilePath,
            average_range, variance_range, co_variation_range
        );
    }

    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerCleanupCells );
#endif
    for ( int thread = 0; thread < max_threads; thread++ )
    {
        for ( int variable = 0; variable < nvariables; variable++ )
        {
            delete cell[thread][variable];
        }
    }
    }

    if ( mpi_rank == 0 )
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerWriteTfFile );
#endif
        std::ifstream src( tfJsonPath_old.c_str(), std::ios::binary );
        std::ofstream dst( tfJsonPath_step.c_str(), std::ios::binary );
        dst << src.rdbuf();
    }
#endif
    } // end initial flag
#ifndef CPU_VER
    if ( async_io_enabled )
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerAsyncIoWait );
#endif
        std::cout << "Particle write thread is active." << std::endl;
        std::cout << "Waiting for particle write thread to finish." << std::endl;
        pbvr::ParticleWriteThread* particle_write_thread = &pwt;
        particle_write_thread->join( true );
        std::cout << "Particle write thread is finished." << std::endl;
    }
    else
    {
        std::cout << "Particle write thread is not active." << std::endl;
    }

    std::cout << "Waiting for all processes to finish." << std::endl;
    {
#ifdef ENABLE_ENSEMBLE_TIMER
        EnsembleTimerScope timer_scope( &ensemble_timer, EnsembleTimerFinalBarrierState );
#endif
#ifndef CPU_VER
    MPI_Barrier( MPI_COMM_WORLD );
#endif
    std::cout << "All processes have finished." << std::endl;

    if ( mpi_rank == 0 )
    {
        std::ofstream ofs( stateFilePath.c_str(), std::ios::out );
        if ( !ofs.is_open() ) std::cout << "Cannot open state.txt" << std::endl;

        ofs << "START_STEP  = " << start_time_step << std::endl;
        ofs << "LATEST_STEP = " << time_step << std::endl;
        ofs.close();
    }
    }
#endif

#ifdef ENABLE_ENSEMBLE_TIMER
    ensemble_total_timer.stop();
    ensemble_timer.add( EnsembleTimerTotal, ensemble_total_timer.sec() );
    ensemble_timer.printCsv( mpi_rank, mpi_size );
#endif
        delete particle_property.m_transfunc_synthesizer;
        delete particle_property.m_camera;
    return true;
}

bool SetParticleParameter( 
    const domain_parameters_unstruct& dom,
    const std::string& tfJsonPath,
    const std::string& tfJsonPath_old,
    ParticleProperty& particle_property,
    MultiVolumePropertyList& mvpl,
    const int nvariables,
    bool& object_generation_enabled
)
{
    bool result = true;
    int mpi_rank;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
#else
    mpi_rank = 0;
#endif

    ParameterFileReader ppr;

    int size = 0;
    char* buf = nullptr;
    int object_generation_enabled_int = 0;
    static bool hasCachedParticleProperty = false;
    static std::vector<char> cachedParticlePropertyBuffer;

    if ( mpi_rank == 0 )
    {
        std::ifstream tfJson( tfJsonPath );
        std::ifstream tfJsonOld( tfJsonPath_old );

        bool loaded = false;
        if ( tfJson.good() )
        {
            loaded = ppr.readTransferFunctionFromJson( tfJsonPath.c_str(), particle_property );
            if ( loaded )
            {
                std::rename( tfJsonPath.c_str(), tfJsonPath_old.c_str() );
            }
        }
        if ( !loaded && !tfJson.good() && hasCachedParticleProperty )
        {
            size = static_cast<int>( cachedParticlePropertyBuffer.size() );
            if ( size > 0 )
            {
                buf = new char[size];
                for ( int i = 0; i < size; ++i )
                {
                    buf[i] = cachedParticlePropertyBuffer[i];
                }
                particle_property.unpack( buf );
                delete[] buf;
                buf = nullptr;
            }
            loaded = size > 0;
        }
        if ( !loaded && tfJsonOld.good() )
        {
            loaded = ppr.readTransferFunctionFromJson( tfJsonPath_old.c_str(), particle_property );
        }

        if ( loaded )
        {
            object_generation_enabled_int = 1;
        }
        else if ( size == 0 )
        {
            std::cout << "================================================================" << std::endl;
            std::cout << "[WARN] Failed to load transfer function json." << std::endl;
            std::cout << "[WARN] Files: " << FileNameOnly( tfJsonPath ) << " and "
                      << FileNameOnly( tfJsonPath_old ) << std::endl;
            std::cout << "[INFO] VIS_PARAM_DIR = " << EnvValueOrUnset( "VIS_PARAM_DIR" ) << std::endl;
            std::cout << "[INFO] PARTICLE_DIR  = " << EnvValueOrUnset( "PARTICLE_DIR" ) << std::endl;
            std::cout << "[INFO] Set default particle parameters and skip object generation." << std::endl;
            std::cout << "================================================================" << std::endl;

            SetDefaultParticleParameter( particle_property, mvpl, nvariables );
            object_generation_enabled_int = 0;
        }
        else
        {
            result = false;
        }

        size = particle_property.byteSize();

        if ( size > 0 )
        {
            buf = new char[size];
            particle_property.pack( buf );
            if ( object_generation_enabled_int != 0 )
            {
                cachedParticlePropertyBuffer.assign( buf, buf + size );
                hasCachedParticleProperty = true;
            }
        }

    }

#ifndef CPU_VER
    MPI_Bcast( &object_generation_enabled_int, 1, MPI_INT, 0, MPI_COMM_WORLD );
    MPI_Bcast( &size, 1, MPI_INT, 0, MPI_COMM_WORLD );
#endif
    object_generation_enabled = object_generation_enabled_int != 0;

    if ( size > 0 )
    {
        if ( mpi_rank > 0 ) buf = new char [size];
#ifndef CPU_VER
        MPI_Bcast( buf, size, MPI_CHARACTER, 0, MPI_COMM_WORLD );
#endif
        if( mpi_rank > 0 ) particle_property.unpack( buf );
        if ( mpi_rank > 0 && object_generation_enabled )
        {
            cachedParticlePropertyBuffer.assign( buf, buf + size );
            hasCachedParticleProperty = true;
        }
        particle_property.UpdateTransferFunctionSynthesizer();
        delete[] buf;
    }
    else
    {
        return false;
    }

    vismodule::Vector3f min_object_coords(
        dom.x_global_min,
        dom.y_global_min,
        dom.z_global_min
    );
    vismodule::Vector3f max_object_coords(
        dom.x_global_max,
        dom.y_global_max,
        dom.z_global_max
    );

    mvpl.m_total_min_object_coord  = min_object_coords;
    mvpl.m_total_max_object_coord  = max_object_coords;

    const float min = vismodule::Math::Min(
        dom.x_global_min,
        dom.y_global_min,
        dom.z_global_min
    );

    const float max = vismodule::Math::Max(
        dom.x_global_max,
        dom.y_global_max,
        dom.z_global_max
    );

    vismodule::StructuredVolumeObject object;
    object.setMinMaxObjectCoords( min_object_coords, max_object_coords );
    object.setMinMaxExternalCoords( min_object_coords, max_object_coords );

    const double total_volume = ( dom.x_global_max - dom.x_global_min )
                              * ( dom.y_global_max - dom.y_global_min )
                              * ( dom.z_global_max - dom.z_global_min );

    //const float max_opacity           = 0.98;
    const float max_opacity           = 0.9;  //アンサンブル用調整
    const int particle_limit          = particle_property.m_particle_limit;
    const float extra_opacity_factor  = particle_property.m_extra_opacity_factor;
    particle_property.m_sampling_step = ( max - min ) / 1E1 / extra_opacity_factor;
    const float sampling_step         = particle_property.m_sampling_step;
    const int subpixel_level          = CalculateSubpixelLevel(
        particle_limit,
        *particle_property.m_camera,
        sampling_step,
        total_volume,
        &object
    );

    float sampling_volume_inverse;
    float max_density;

    Generator::CalculateDensityParameters(
        particle_property.m_camera,
        &object,
        (float)subpixel_level,
        sampling_step,
        max_opacity,
        &sampling_volume_inverse,
        &max_density
    );

    particle_property.m_transfunc_synthesizer->setMaxOpacity( max_opacity );
    particle_property.m_transfunc_synthesizer->setMaxDensity( max_density );
    particle_property.m_transfunc_synthesizer->setSamplingVolumeInverse( sampling_volume_inverse );


    if( mpi_rank == 1 )
    {
        fprintf( stdout , "---------initialize Parameters--------------------------------------------\n" );
        fprintf( stdout , "particle_limit       = %20d\n"   , particle_limit                             );
        fprintf( stdout , "extra_opacity_factor = %20f\n"   , extra_opacity_factor                       );
        fprintf( stdout , "resolutin_height     = %20d\n"   , particle_property.m_camera->windowHeight() );
        fprintf( stdout , "resolutin_width      = %20d\n"   , particle_property.m_camera->windowWidth()  );
        fprintf( stdout , "total_volume         = %20.3e\n" , total_volume                               );
        fprintf( stdout , "  |-X                = %20f\n"   , object.maxObjectCoord().x()                );
        fprintf( stdout , "  |-Y                = %20f\n"   , object.maxObjectCoord().y()                );
        fprintf( stdout , "  |-Z                = %20f\n"   , object.maxObjectCoord().z()                );
        fprintf( stdout , "max_opacity          = %20.3e\n" , max_opacity                                );
        fprintf( stdout , "max_density          = %20.3e\n" , max_density                                );
        fprintf( stdout , "sampling_step        = %20.3e\n" , sampling_step                              );
        fprintf( stdout , "subpixel_level       = %20d\n"   , subpixel_level                             );
        fprintf( stdout , "--------------------------------------------------------------------------\n" );
    }

    return result;
}

#ifdef EXTEND_FILE_FORMAT
// vtk用のソルバー関数
bool generate_particles_vtk( int time_step, vtkUnstructuredGrid* ucd )
{
    int mpi_rank;
    int mpi_size;
#ifndef CPU_VER
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_rank );
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_size );
#else
    mpi_rank = 0;
    mpi_size = 1;
#endif
 
    domain_parameters_unstruct dom;
    SetDomain( ucd, &dom );

    if ( is_initial_step == true )
    {
        is_initial_step = false;
        start_time_step = time_step;
    }

    std::string historyFilePath;
    std::string stateFilePath;
    std::string coordMinMaxFilePath;
    std::string particleFilePrefix;
    std::string glyphFilePrefix;
    std::string plotOverLineFilePrefix;
    std::string plotOverTimeFilePrefix;
    std::string tfJsonPath;
    std::string tfJsonPath_old;
    std::string tfJsonPath_step;
    std::string glyphParameterPath;
    std::string glyphParameterPath_old;
    std::string plotOverLineParameterPath;
    std::string plotOverLineParameterPath_old; 
    std::string plotOverTimeParameterPath;
    std::string plotOverTimeParameterPath_old;

    SetParameterFilePath(
        time_step,
        historyFilePath,
        stateFilePath,
        coordMinMaxFilePath,
        particleFilePrefix,
        glyphFilePrefix,
        plotOverLineFilePrefix,
        plotOverTimeFilePrefix,
        tfJsonPath,
        tfJsonPath_old,
        tfJsonPath_step,
        glyphParameterPath,
        glyphParameterPath_old,
        plotOverLineParameterPath,
        plotOverLineParameterPath_old,
        plotOverTimeParameterPath,
        plotOverTimeParameterPath_old
    );

    char  arg_dummy0[] = "dummy";
    char* arg_dummy[]  = { arg_dummy0, NULL };

    ParticleProperty particle_property;
    GlyphProperty glyph_property;
    PlotOverLineProperty pol_property;
    PlotOverTimeProperty pot_property;
    MultiVolumePropertyList mvpl;
    particle_property.m_transfunc_synthesizer = new TransferFunctionSynthesizer();
    particle_property.m_camera                = new vismodule::Camera();

    bool object_generation_enabled = false;
    SetParticleParameter(
        dom, tfJsonPath, tfJsonPath_old, particle_property, mvpl,
        nvariables, object_generation_enabled
    );
    if ( object_generation_enabled )
    {
        SetGlyphParameter( glyphParameterPath, glyphParameterPath_old, glyph_property );
        SetPlotOverLineParameter( plotOverLineParameterPath, plotOverLineParameterPath_old, pol_property );
        SetPlotOverTimeParameter( plotOverTimeParameterPath, plotOverTimeParameterPath_old, pot_property );
    }

    const int tf_number  = particle_property.m_transfunc_array.size();
    const int resolution = object_generation_enabled ? pol_property.m_sampling_size : 0;

    // particle parameters
    std::vector<float> particle_coords;
    std::vector<Byte>  particle_colors;
    std::vector<float> particle_normals;
    vismodule::UInt64* tmp_c_bins;
    vismodule::UInt64* tmp_o_bins;
    float* tmp_max;
    float* tmp_min;

    tmp_c_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_o_bins = new vismodule::UInt64[DEFAULT_NBINS * tf_number];
    tmp_max = new float[tf_number * 2]; // color, opacity
    tmp_min = new float[tf_number * 2]; // color, opacity

    for ( std::size_t i = 0; i < (DEFAULT_NBINS * tf_number); i++ )
    {
        tmp_c_bins[i] = 0;
        tmp_o_bins[i] = 0;
    }

    for ( int i = 0; i < (tf_number * 2); i++ )
    {
        tmp_max[i] = FLT_MIN;
        tmp_min[i] = FLT_MAX;
    }

    // glyph parameters
    std::vector<float>         glyph_coords;
    std::vector<float>         glyph_vectors;
    std::vector<float>         glyph_sizes;
    std::vector<unsigned char> glyph_colors;

    // plot over line parameters
    std::vector<float> values_on_line( resolution, 0 );
    std::vector<int>   mask( resolution, 0 );
    std::vector<float> x_axis( resolution, 0 );

    // plot over time paraemters
    std::vector<float> value_on_time;
    bool pot_mask = false;

    int nvariables = 0;

    // 先にセルタイプを持っておく
    kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid input_vtu( ucd );
    std::vector<kvs::UnstructuredVolumeObject::CellType> kvs_cell_type_vector;

    for( auto vtu : input_vtu.eachCellType() )
    {
        kvs::ExtendedFileFormat::VtkImporter<kvs::ExtendedFileFormat::VtkXmlUnstructuredGrid> importer( &vtu );
        kvs::UnstructuredVolumeObject* volume = &importer;
        kvs_cell_type_vector.push_back( volume->cellType() );
    }

    auto convert_cell_type = [&](
        kvs::UnstructuredVolumeObject::CellType kvs_cell_type,
        std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
        int& nvariables,
        std::unique_ptr<float[]>& coordinates,
        int& ncoords,
        std::unique_ptr<unsigned int[]>& connections,
        int& ncells,
        vismodule::VolumeObjectBase::CellType& celltype
    )
    {
        vismodule::UnstructuredVolumeImporter importer;
        importer.import( input_vtu, static_cast<int>(kvs_cell_type) );
        vismodule::UnstructuredVolumeObject* volume = &importer;
        domain_parameters_unstruct tmp_dom; // not use
        nvariables = 0;

        store_volume_in_variables_array_unstruct(
            volume, tmp_dom, values, nvariables, coordinates,
            ncoords, connections, ncells, celltype
        );
    };

    auto collect_cell_type_min_max = [&](
        std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
        const int nvariables,
        std::unique_ptr<float[]>& coordinates,
        const int ncoords,
        std::unique_ptr<unsigned int[]>& connections,
        const int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype
    )
    {
        std::vector<Type*> raw_pointers_vector( nvariables );
        for ( std::size_t j = 0; j < nvariables; ++j )
        {
            raw_pointers_vector[j] = values.get()[j].get();
        }

        CollectParticleMinMax(
            particle_property, dom, raw_pointers_vector.data(), nvariables, coordinates.get(),
            ncoords, connections.get(), ncells, celltype, tf_number, tmp_max, tmp_min
        );
    };

    auto process_cell_type = [&](
        std::unique_ptr<std::unique_ptr<Type[]>[]>& values,
        const int nvariables,
        std::unique_ptr<float[]>& coordinates,
        const int ncoords,
        std::unique_ptr<unsigned int[]>& connections,
        const int ncells,
        const vismodule::VolumeObjectBase::CellType& celltype
    )
    {

        if ( value_on_time.size() != nvariables ) value_on_time.resize( nvariables ); // Plot Over Time

        std::vector<Type*> raw_pointers_vector( nvariables );
        for ( std::size_t j = 0; j < nvariables; ++j )
        {
            raw_pointers_vector[j] = values.get()[j].get();
        }

        ServerMode server_mode = ServerMode::IS;

        CollectParticleHistogram(
            particle_property, dom, raw_pointers_vector.data(), nvariables, coordinates.get(),
            ncoords, connections.get(), ncells, celltype, tf_number, tmp_c_bins, tmp_o_bins
        );

        if ( object_generation_enabled )
        {
            GenerateParticleObject(
                particle_property, dom, raw_pointers_vector.data(), nvariables, coordinates.get(),
                ncoords, connections.get(), ncells, celltype,
                particle_coords, particle_colors, particle_normals
            );

            if ( glyph_property.m_glyph_flag )
            {
                vismodule::KVSMLObjectGlyph* glyph_object = new vismodule::KVSMLObjectGlyph;
                vismodule::GlyphSeedGenerator glyph_creator;
                int number_of_divide = mpi_size;
                glyph_creator.GenerateGlyphUnstruct(
                    glyph_property, number_of_divide, raw_pointers_vector.data(), nvariables, coordinates.get(),
                    ncoords, connections.get(), ncells, celltype, server_mode, glyph_object
                );

                MakeGlyph( glyph_object, glyph_coords, glyph_vectors, glyph_sizes, glyph_colors ); // InSitu only

                delete glyph_object;
            }

            if ( pol_property.m_plot_flag )
            {
                vismodule::KVSMLObjectPlotOverLine* pol_object = new vismodule::KVSMLObjectPlotOverLine;
                PlotOverLineGenerator pol_generator;
                pol_generator.GeneratePOLUnstruct(
                    pol_property, raw_pointers_vector.data(), nvariables, coordinates.get(),
                    ncoords, connections.get(), ncells, celltype, pol_object
                );

                for( std::size_t j = 0; j < resolution; j++ )
                {
                    x_axis[j] = pol_object->x_axis()[j];
                    if ( pol_object->mask()[j] )
                    {
                        mask[j]           = 1;
                        values_on_line[j] = pol_object->values_on_line()[j];
                    }
                }

                delete pol_object;
            }

            // if ( pot_property.m_plot_flag )
            if ( true ) // 常に生成し続ける、将来的に変わる可能性あり
            {
                PlotOverTimeGenerator pot_generator;

                // すでにPlotOverTimeを取得している場合はスキップ
                if ( !pot_mask )
                {
                    pot_mask = pot_generator.GeneratePOTUnstruct(
                        pot_property, raw_pointers_vector.data(), nvariables, coordinates.get(),
                        ncoords, connections.get(), ncells, celltype, value_on_time
                    );
                }
            }
        }
    };

    for ( std::size_t i = 0; i < kvs_cell_type_vector.size(); i++ )
    {
        std::unique_ptr<std::unique_ptr<Type[]>[]> values;
        nvariables = 0;
        std::unique_ptr<float[]> coordinates;
        int ncoords = 0;
        std::unique_ptr<unsigned int[]> connections;
        int ncells = 0;
        vismodule::VolumeObjectBase::CellType celltype;

        convert_cell_type(
            kvs_cell_type_vector[i], values, nvariables, coordinates,
            ncoords, connections, ncells, celltype
        );

        collect_cell_type_min_max( values, nvariables, coordinates, ncoords, connections, ncells, celltype );
    } // for ( std::size_t i = 0; i < kvs_cell_type_vector.size(); i++ )

#ifndef CPU_VER
    if ( mpi_size > 1 )
    {
        MPI_Allreduce( MPI_IN_PLACE, tmp_max, ( tf_number * 2 ), MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
        MPI_Allreduce( MPI_IN_PLACE, tmp_min, ( tf_number * 2 ), MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    }
#endif

    for( std::size_t i = 0; i < tf_number; i++ )
    {
        const float color_variable_min   = tmp_min[2 * i + 1];
        const float color_variable_max   = tmp_max[2 * i + 1];
        const float opacity_variable_min = tmp_min[2 * i    ];
        const float opacity_variable_max = tmp_max[2 * i    ];

        particle_property.m_transfunc_array[i].m_server_color_variable_min   = color_variable_min;
        particle_property.m_transfunc_array[i].m_server_color_variable_max   = color_variable_max;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_min = opacity_variable_min;
        particle_property.m_transfunc_array[i].m_server_opacity_variable_max = opacity_variable_max;

        if( particle_property.m_transfunc_array[i].m_server_color_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[i].setColorRange( color_variable_min, color_variable_max );
        }

        if( particle_property.m_transfunc_array[i].m_server_opacity_range_mode == NamedTransferFunction::ServerRangeMode::ServerSide )
        {
            particle_property.m_transfunc_array[i].setOpacityRange( opacity_variable_min, opacity_variable_max );
        }
    }

    for ( std::size_t i = 0; i < kvs_cell_type_vector.size(); i++ )
    {
        std::unique_ptr<std::unique_ptr<Type[]>[]> values;
        nvariables = 0;
        std::unique_ptr<float[]> coordinates;
        int ncoords = 0;
        std::unique_ptr<unsigned int[]> connections;
        int ncells = 0;
        vismodule::VolumeObjectBase::CellType celltype;

        convert_cell_type(
            kvs_cell_type_vector[i], values, nvariables, coordinates,
            ncoords, connections, ncells, celltype
        );

        process_cell_type( values, nvariables, coordinates, ncoords, connections, ncells, celltype );
    }

    OutputCoordMinMaxFile( dom, coordMinMaxFilePath );

    if ( object_generation_enabled )
    {
        OutputParticleFiles(
            particle_property, mvpl, time_step, particleFilePrefix,
            particle_coords, particle_colors, particle_normals
        );
    }

    OutputParticleHistory(
        particle_property, tf_number, nvariables, historyFilePath,
        tmp_c_bins, tmp_o_bins, tmp_max, tmp_min
    );

    if ( object_generation_enabled && glyph_property.m_glyph_flag )
    {
        OutputGlyphs(
            time_step, glyphFilePrefix, glyph_coords,
            glyph_vectors, glyph_sizes, glyph_colors
        );
    }

    if ( object_generation_enabled && pol_property.m_plot_flag )
    {
        OutputLine( time_step, plotOverLineFilePrefix, values_on_line, mask, x_axis );
    }

    // if ( pot_property.m_plot_flag )
    if ( object_generation_enabled ) // 常にファイルを出力し続ける、将来的に変更する可能性あり
    {
        OutputPOT( time_step, plotOverTimeFilePrefix, pot_mask, value_on_time );
    }

    // OutputParticlesで更新したdefault_old.jsonをtimestep別JSONに保存する
    if ( object_generation_enabled && mpi_rank == 0 )
    {
        std::ifstream src( tfJsonPath_old.c_str(), std::ios::binary );
        std::ofstream dst( tfJsonPath_step.c_str(), std::ios::binary );
        dst << src.rdbuf();
    }

    // 粒子ファイル書き込みスレッドが終了するまで待機
    // async_io_enabled, pwtはkvs_wrapper_common.cppに宣言
    if ( async_io_enabled )
    {
        std::cout << "Particle write thread is active."             << std::endl;
        std::cout << "Waiting for particle write thread to finish." << std::endl;
        pbvr::ParticleWriteThread* particle_write_thread = &pwt;
        particle_write_thread->join( true );
        std::cout << "Particle write thread is finished." << std::endl;
    }
    else
    {
        std::cout << "Particle write thread is not active." << std::endl;
    }

    std::cout << "Waiting for all processes to finish." << std::endl;
    
#ifndef CPU_VER
    MPI_Barrier( MPI_COMM_WORLD ); // すべてのプロセスでファイルの書き込む処理が終了するまで待機
#endif
    
    std::cout << "All processes have finished." << std::endl;

    if ( mpi_rank == 0 )
    {
        std::ofstream ofs( stateFilePath.c_str(), std::ios::out );
        if( !ofs.is_open() ) std::cout << "Cannot open state.txt" << std::endl;

        ofs << "START_STEP  = " << start_time_step << std::endl;
        ofs << "LATEST_STEP = " << time_step       << std::endl;

        ofs.close();
    }

    delete[] tmp_c_bins;
    delete[] tmp_o_bins;
    delete[] tmp_max;
    delete[] tmp_min;
    delete particle_property.m_transfunc_synthesizer;
    delete particle_property.m_camera;

    return true;
}

void SetDomain( vtkUnstructuredGrid* ucd, domain_parameters_unstruct* dom )
{
    double bounds[6];
    ucd->GetPoints()->GetBounds( bounds );
    float recv_Xmin, recv_Xmax;
    float recv_Ymin, recv_Ymax;
    float recv_Zmin, recv_Zmax;

    float Xmin = bounds[0];
    float Xmax = bounds[1];
    float Ymin = bounds[2];
    float Ymax = bounds[3];
    float Zmin = bounds[4];
    float Zmax = bounds[5];

#ifndef CPU_VER
    MPI_Allreduce( &Xmin, &recv_Xmin, 1 , MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( &Ymin, &recv_Ymin, 1 , MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( &Zmin, &recv_Zmin, 1 , MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD );
    MPI_Allreduce( &Xmax, &recv_Xmax, 1 , MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
    MPI_Allreduce( &Ymax, &recv_Ymax, 1 , MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
    MPI_Allreduce( &Zmax, &recv_Zmax, 1 , MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD );
#else // CPU_VER
    recv_Xmin = Xmin;
    recv_Ymin = Ymin;
    recv_Zmin = Zmin;
    recv_Xmax = Xmin;
    recv_Ymax = Ymax;
    recv_Zmax = Zmax;
#endif // CPU_VER

    dom->x_global_min = recv_Xmin;
    dom->y_global_min = recv_Ymin;
    dom->z_global_min = recv_Zmin;
    dom->x_global_max = recv_Xmax;
    dom->y_global_max = recv_Ymax;
    dom->z_global_max = recv_Zmax;
}
#endif // EXTEND_FILE_FORMAT
