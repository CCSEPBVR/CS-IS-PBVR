#include "EnsembleCellHistogram.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>

#ifdef _OPENMP
#include <omp.h>
#endif

#include <vismodule/CellBase>
#include <vismodule/HexahedralCell>
#include <vismodule/PyramidalCell>
#include <vismodule/PrismaticCell>
#include <vismodule/QuadraticHexahedralCell>
#include <vismodule/QuadraticTetrahedralCell>
#include <vismodule/TetrahedralCell>

namespace
{

double SecondsNow()
{
    return static_cast<double>( std::clock() ) / static_cast<double>( CLOCKS_PER_SEC );
}

int CommRank( MPI_Comm comm )
{
#ifndef CPU_VER
    int rank = 0;
    MPI_Comm_rank( comm, &rank );
    return rank;
#else
    (void)comm;
    return 0;
#endif
}

int CommSize( MPI_Comm comm )
{
#ifndef CPU_VER
    int size = 1;
    MPI_Comm_size( comm, &size );
    return size;
#else
    (void)comm;
    return 1;
#endif
}

std::unique_ptr< vismodule::CellBase<Type> > CreateCell(
    Type* values,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype )
{
    switch ( celltype )
    {
    case vismodule::VolumeObjectBase::Tetrahedra:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::TetrahedralCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    case vismodule::VolumeObjectBase::Hexahedra:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::HexahedralCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    case vismodule::VolumeObjectBase::QuadraticTetrahedra:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::QuadraticTetrahedralCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    case vismodule::VolumeObjectBase::QuadraticHexahedra:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::QuadraticHexahedralCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    case vismodule::VolumeObjectBase::Prism:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::PrismaticCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    case vismodule::VolumeObjectBase::Pyramid:
        return std::unique_ptr< vismodule::CellBase<Type> >(
            new vismodule::PyramidalCell<Type>( values, coordinates, ncoords, connections, ncells ) );
    default:
        return std::unique_ptr< vismodule::CellBase<Type> >();
    }
}

std::vector< std::unique_ptr< vismodule::CellBase<Type> > > CreateCellsForVariables(
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype )
{
    std::vector< std::unique_ptr< vismodule::CellBase<Type> > > cells;
    cells.reserve( static_cast<std::size_t>( nvariables ) );
    for ( int v = 0; v < nvariables; ++v )
    {
        cells.push_back( CreateCell( values[v], coordinates, ncoords, connections, ncells, celltype ) );
        if ( !cells.back() )
        {
            cells.clear();
            return cells;
        }
    }
    return cells;
}

bool ValidInput(
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells )
{
    return values != NULL && nvariables > 0 && coordinates != NULL && ncoords > 0 &&
           connections != NULL && ncells > 0;
}

int HistogramBin( const float value, const float min_value, const float max_value, const int nbins )
{
    if ( !std::isfinite( value ) ) return -1;
    if ( max_value == min_value ) return 0; // Constant fields are counted in bin 0.
    if ( value < min_value || value > max_value ) return -1;

    const float h = ( value - min_value ) / ( max_value - min_value ) * static_cast<float>( nbins );
    int bin = static_cast<int>( h );
    if ( bin == nbins ) bin = nbins - 1;
    if ( bin < 0 || bin >= nbins ) return -1;
    return bin;
}

} // namespace

namespace pbvr
{

EnsembleCellHistogramLog::EnsembleCellHistogramLog() :
    nvariables( 0 ),
    ncells( 0 ),
    nbins( 0 ),
    comm_size( 1 ),
    histogram_bytes( 0 ),
    local_minmax_seconds( 0.0 ),
    global_minmax_mpi_seconds( 0.0 ),
    local_histogram_seconds( 0.0 ),
    global_histogram_mpi_seconds( 0.0 )
{
}

bool ComputeLocalCellCenterMinMax(
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    std::vector<float>& local_min,
    std::vector<float>& local_max,
    EnsembleCellHistogramLog* log )
{
    if ( !ValidInput( values, nvariables, coordinates, ncoords, connections, ncells ) ) return false;

    const double start = SecondsNow();
    local_min.assign( static_cast<std::size_t>( nvariables ), FLT_MAX );
    local_max.assign( static_cast<std::size_t>( nvariables ), -FLT_MAX );

    if ( !CreateCell( values[0], coordinates, ncoords, connections, ncells, celltype ) ) return false;

#ifdef _OPENMP
#pragma omp parallel
#endif
    {
        std::vector<float> thread_min( static_cast<std::size_t>( nvariables ), FLT_MAX );
        std::vector<float> thread_max( static_cast<std::size_t>( nvariables ), -FLT_MAX );
        std::vector< std::unique_ptr< vismodule::CellBase<Type> > > cells =
            CreateCellsForVariables( values, nvariables, coordinates, ncoords, connections, ncells, celltype );

        if ( cells.size() == static_cast<std::size_t>( nvariables ) )
        {
#ifdef _OPENMP
#pragma omp for schedule(dynamic)
#endif
            for ( int cell_id = 0; cell_id < ncells; ++cell_id )
            {
                for ( int v = 0; v < nvariables; ++v )
                {
                    cells[v]->bindCell( static_cast<vismodule::UInt32>( cell_id ) );
                    const float value = static_cast<float>( cells[v]->localGravityPointValue() );
                    // NaN/Inf samples are ignored, not converted to zero.
                    if ( !std::isfinite( value ) ) continue;
                    if ( value < thread_min[v] ) thread_min[v] = value;
                    if ( value > thread_max[v] ) thread_max[v] = value;
                }
            }
        }

#ifdef _OPENMP
#pragma omp critical
#endif
        {
            for ( int v = 0; v < nvariables; ++v )
            {
                if ( thread_min[v] < local_min[v] ) local_min[v] = thread_min[v];
                if ( thread_max[v] > local_max[v] ) local_max[v] = thread_max[v];
            }
        }
    }

    for ( int v = 0; v < nvariables; ++v )
    {
        if ( local_min[v] == FLT_MAX ) local_min[v] = 0.0f;
        if ( local_max[v] == -FLT_MAX ) local_max[v] = 0.0f;
    }

    if ( log != NULL )
    {
        log->nvariables = nvariables;
        log->ncells = ncells;
        log->local_minmax_seconds += SecondsNow() - start;
    }

    return true;
}

bool ReduceGlobalMinMax(
    const std::vector<float>& local_min,
    const std::vector<float>& local_max,
    std::vector<float>& global_min,
    std::vector<float>& global_max,
    MPI_Comm comm,
    EnsembleCellHistogramLog* log )
{
    if ( local_min.empty() || local_min.size() != local_max.size() ) return false;

    const double start = SecondsNow();
    const int nvariables = static_cast<int>( local_min.size() );
    global_min.resize( local_min.size() );
    global_max.resize( local_max.size() );

#ifndef CPU_VER
    MPI_Allreduce( const_cast<float*>( local_min.data() ), global_min.data(), nvariables, MPI_FLOAT, MPI_MIN, comm );
    MPI_Allreduce( const_cast<float*>( local_max.data() ), global_max.data(), nvariables, MPI_FLOAT, MPI_MAX, comm );
#else
    (void)comm;
    global_min = local_min;
    global_max = local_max;
#endif

    if ( log != NULL )
    {
        log->nvariables = nvariables;
        log->comm_size = CommSize( comm );
        log->global_minmax_mpi_seconds += SecondsNow() - start;
    }

    return true;
}

bool ComputeLocalCellCenterHistogram(
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    const std::vector<float>& global_min,
    const std::vector<float>& global_max,
    int nbins,
    std::vector<long long>& local_histogram,
    EnsembleCellHistogramLog* log )
{
    if ( !ValidInput( values, nvariables, coordinates, ncoords, connections, ncells ) ) return false;
    if ( nbins <= 0 || global_min.size() != static_cast<std::size_t>( nvariables ) ||
         global_max.size() != static_cast<std::size_t>( nvariables ) ) return false;

    const double start = SecondsNow();
    const std::size_t histogram_size = static_cast<std::size_t>( nvariables ) * static_cast<std::size_t>( nbins );
    local_histogram.assign( histogram_size, 0 );

    if ( !CreateCell( values[0], coordinates, ncoords, connections, ncells, celltype ) ) return false;

#ifdef _OPENMP
#pragma omp parallel
#endif
    {
        std::vector<long long> thread_histogram( histogram_size, 0 );
        std::vector< std::unique_ptr< vismodule::CellBase<Type> > > cells =
            CreateCellsForVariables( values, nvariables, coordinates, ncoords, connections, ncells, celltype );

        if ( cells.size() == static_cast<std::size_t>( nvariables ) )
        {
#ifdef _OPENMP
#pragma omp for schedule(dynamic)
#endif
            for ( int cell_id = 0; cell_id < ncells; ++cell_id )
            {
                for ( int v = 0; v < nvariables; ++v )
                {
                    cells[v]->bindCell( static_cast<vismodule::UInt32>( cell_id ) );
                    const float value = static_cast<float>( cells[v]->localGravityPointValue() );
                    const int bin = HistogramBin( value, global_min[v], global_max[v], nbins );
                    if ( bin < 0 ) continue;
                    thread_histogram[static_cast<std::size_t>( v ) * nbins + bin]++;
                }
            }
        }

#ifdef _OPENMP
#pragma omp critical
#endif
        {
            for ( std::size_t i = 0; i < histogram_size; ++i ) local_histogram[i] += thread_histogram[i];
        }
    }

    if ( log != NULL )
    {
        log->nvariables = nvariables;
        log->ncells = ncells;
        log->nbins = nbins;
        log->histogram_bytes = histogram_size * sizeof( long long );
        log->local_histogram_seconds += SecondsNow() - start;
    }

    return true;
}

bool ReduceGlobalHistogram(
    const std::vector<long long>& local_histogram,
    std::vector<long long>& global_histogram,
    int nvariables,
    int nbins,
    MPI_Comm comm,
    EnsembleCellHistogramLog* log )
{
    const std::size_t histogram_size = static_cast<std::size_t>( nvariables ) * static_cast<std::size_t>( nbins );
    if ( local_histogram.size() != histogram_size ) return false;

    const double start = SecondsNow();
    global_histogram.assign( histogram_size, 0 );

#ifndef CPU_VER
    MPI_Reduce(
        const_cast<long long*>( local_histogram.data() ),
        global_histogram.data(),
        static_cast<int>( histogram_size ),
        MPI_LONG_LONG,
        MPI_SUM,
        0,
        comm );
#else
    (void)comm;
    global_histogram = local_histogram;
#endif

    if ( log != NULL )
    {
        log->nvariables = nvariables;
        log->nbins = nbins;
        log->comm_size = CommSize( comm );
        log->histogram_bytes = histogram_size * sizeof( long long );
        log->global_histogram_mpi_seconds += SecondsNow() - start;
    }

    return true;
}

bool WriteHistogramResult(
    const std::string& filename,
    const std::vector<float>& global_min,
    const std::vector<float>& global_max,
    const std::vector<long long>& global_histogram,
    int nvariables,
    int nbins,
    int mpi_rank )
{
    if ( mpi_rank != 0 ) return true;
    if ( nvariables <= 0 || nbins <= 0 ) return false;
    if ( global_min.size() != static_cast<std::size_t>( nvariables ) ||
         global_max.size() != static_cast<std::size_t>( nvariables ) ||
         global_histogram.size() != static_cast<std::size_t>( nvariables * nbins ) ) return false;

    std::ofstream ofs( filename.c_str(), std::ios::out );
    if ( !ofs.is_open() ) return false;

    ofs << "N_VARIABLES=" << nvariables << "\n";
    ofs << "NBINS=" << nbins << "\n";
    for ( int v = 0; v < nvariables; ++v )
    {
        ofs << "VAR=" << v << "\n";
        ofs << "MIN=" << global_min[v] << "\n";
        ofs << "MAX=" << global_max[v] << "\n";
        ofs << "HISTOGRAM=";
        for ( int b = 0; b < nbins; ++b )
        {
            if ( b > 0 ) ofs << ',';
            ofs << global_histogram[static_cast<std::size_t>( v ) * nbins + b];
        }
        ofs << "\n\n";
    }

    return true;
}

bool ComputeAndWriteEnsembleCellHistogram(
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    MPI_Comm ensemble_comm,
    int nbins,
    const std::string& output_filename,
    EnsembleCellHistogramLog* log )
{
    // Candidate integration point: call from the future kvs_wrapper.cpp statistics path
    // after initial TF/config loading and before particle generation. This function does
    // no particle sampling and communicates only min/max and histogram arrays.
    if ( nbins <= 0 ) nbins = 256;

    std::vector<float> local_min;
    std::vector<float> local_max;
    std::vector<float> global_min;
    std::vector<float> global_max;
    std::vector<long long> local_histogram;
    std::vector<long long> global_histogram;

    if ( !ComputeLocalCellCenterMinMax(
             values, nvariables, coordinates, ncoords, connections, ncells, celltype,
             local_min, local_max, log ) ) return false;
    if ( !ReduceGlobalMinMax( local_min, local_max, global_min, global_max, ensemble_comm, log ) ) return false;
    if ( !ComputeLocalCellCenterHistogram(
             values, nvariables, coordinates, ncoords, connections, ncells, celltype,
             global_min, global_max, nbins, local_histogram, log ) ) return false;
    if ( !ReduceGlobalHistogram( local_histogram, global_histogram, nvariables, nbins, ensemble_comm, log ) ) return false;

    return WriteHistogramResult(
        output_filename,
        global_min,
        global_max,
        global_histogram,
        nvariables,
        nbins,
        CommRank( ensemble_comm ) );
}

bool StoreGlobalCellHistogramToParticleProperty(
    const std::vector<float>& global_min,
    const std::vector<float>& global_max,
    const std::vector<long long>& global_histogram,
    int nvariables,
    int nbins,
    int mpi_rank,
    ParticleProperty& particle_property )
{
    if ( mpi_rank != 0 ) return true;
    if ( nbins != DEFAULT_NBINS ) return false;
    if ( nvariables <= 0 ) return false;
    if ( global_min.size() != static_cast<std::size_t>( nvariables ) ||
         global_max.size() != static_cast<std::size_t>( nvariables ) ||
         global_histogram.size() != static_cast<std::size_t>( nvariables * nbins ) ) return false;

    const int tf_number = static_cast<int>( particle_property.m_transfunc_array.size() );
    if ( tf_number <= 0 ) return false;
    if ( tf_number > nvariables ) return false;

    // Cell-center histogram is computed per physical variable. For this test path,
    // transfer-function i is mapped to variable i, and the same range/histogram is
    // stored for both color and opacity so OutputEnsembleStatisticHistory-compatible
    // code can treat it like ordinary TF history data.
    for ( int i = 0; i < tf_number; ++i )
    {
        NamedTransferFunction& tf = particle_property.m_transfunc_array[i];
        tf.m_server_opacity_variable_min = global_min[i];
        tf.m_server_opacity_variable_max = global_max[i];
        tf.m_server_color_variable_min = global_min[i];
        tf.m_server_color_variable_max = global_max[i];

        for ( int b = 0; b < DEFAULT_NBINS; ++b )
        {
            const long long count = global_histogram[static_cast<std::size_t>( i ) * nbins + b];
            tf.m_opacity_histogram[b] = count > 0 ? static_cast<vismodule::UInt64>( count ) : 0;
            tf.m_color_histogram[b] = count > 0 ? static_cast<vismodule::UInt64>( count ) : 0;
        }
    }

    return true;
}

bool ComputeAndStoreEnsembleCellHistogram(
    Type** values,
    int nvariables,
    float* coordinates,
    int ncoords,
    unsigned int* connections,
    int ncells,
    const vismodule::VolumeObjectBase::CellType& celltype,
    MPI_Comm ensemble_comm,
    int nbins,
    ParticleProperty& particle_property,
    EnsembleCellHistogramLog* log )
{
    // Candidate integration point: call this from kvs_wrapper.cpp/kvs_ewrapper.cpp
    // after ParticleProperty is initialized and before OutputEnsembleStatisticHistory().
    // The result is stored in particle_property.m_transfunc_array on rank 0.
    if ( nbins <= 0 ) nbins = DEFAULT_NBINS;
    if ( nbins != DEFAULT_NBINS ) return false;

    std::vector<float> local_min;
    std::vector<float> local_max;
    std::vector<float> global_min;
    std::vector<float> global_max;
    std::vector<long long> local_histogram;
    std::vector<long long> global_histogram;

    if ( !ComputeLocalCellCenterMinMax(
             values, nvariables, coordinates, ncoords, connections, ncells, celltype,
             local_min, local_max, log ) ) return false;
    if ( !ReduceGlobalMinMax( local_min, local_max, global_min, global_max, ensemble_comm, log ) ) return false;
    if ( !ComputeLocalCellCenterHistogram(
             values, nvariables, coordinates, ncoords, connections, ncells, celltype,
             global_min, global_max, nbins, local_histogram, log ) ) return false;
    if ( !ReduceGlobalHistogram( local_histogram, global_histogram, nvariables, nbins, ensemble_comm, log ) ) return false;

    return StoreGlobalCellHistogramToParticleProperty(
        global_min,
        global_max,
        global_histogram,
        nvariables,
        nbins,
        CommRank( ensemble_comm ),
        particle_property );
}

void PrintEnsembleCellHistogramLog( const EnsembleCellHistogramLog& log, std::ostream& os )
{
    os << "EnsembleCellHistogram: nvariables=" << log.nvariables
       << ", ncells=" << log.ncells
       << ", nbins=" << log.nbins
       << ", comm_size=" << log.comm_size
       << ", histogram_bytes=" << log.histogram_bytes
       << ", local_minmax_seconds=" << log.local_minmax_seconds
       << ", global_minmax_mpi_seconds=" << log.global_minmax_mpi_seconds
       << ", local_histogram_seconds=" << log.local_histogram_seconds
       << ", global_histogram_mpi_seconds=" << log.global_histogram_mpi_seconds
       << std::endl;
}

bool RunEnsembleCellHistogramSelfTest()
{
    float coordinates[12] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    unsigned int connections[4] = { 0, 1, 2, 3 };

    Type q1[4] = { 0.0f, 1.0f, 1.0f, 1.0f }; // q=x+y+z on the unit tetra vertices.
    Type q2[4] = { 2.0f, 2.0f, 2.0f, 2.0f }; // Constant field exercises max==min.
    Type* values[2] = { q1, q2 };

    std::vector<float> local_min;
    std::vector<float> local_max;
    if ( !ComputeLocalCellCenterMinMax(
             values,
             2,
             coordinates,
             4,
             connections,
             1,
             vismodule::VolumeObjectBase::Tetrahedra,
             local_min,
             local_max,
             NULL ) ) return false;

    if ( std::fabs( local_min[0] - 0.75f ) > 1.0e-5f ||
         std::fabs( local_max[0] - 0.75f ) > 1.0e-5f ||
         std::fabs( local_min[1] - 2.0f ) > 1.0e-5f ||
         std::fabs( local_max[1] - 2.0f ) > 1.0e-5f ) return false;

    std::vector<long long> hist;
    if ( !ComputeLocalCellCenterHistogram(
             values,
             2,
             coordinates,
             4,
             connections,
             1,
             vismodule::VolumeObjectBase::Tetrahedra,
             local_min,
             local_max,
             256,
             hist,
             NULL ) ) return false;

    if ( hist[0] != 1 || hist[256] != 1 ) return false;

    ParticleProperty particle_property;
    particle_property.m_transfunc_array.resize( 2 );
    if ( !StoreGlobalCellHistogramToParticleProperty(
             local_min,
             local_max,
             hist,
             2,
             256,
             0,
             particle_property ) ) return false;

    return particle_property.m_transfunc_array[0].m_opacity_histogram[0] == 1 &&
           particle_property.m_transfunc_array[0].m_color_histogram[0] == 1 &&
           particle_property.m_transfunc_array[1].m_opacity_histogram[0] == 1 &&
           particle_property.m_transfunc_array[1].m_color_histogram[0] == 1 &&
           std::fabs( particle_property.m_transfunc_array[0].m_server_opacity_variable_min - 0.75f ) <= 1.0e-5f &&
           std::fabs( particle_property.m_transfunc_array[1].m_server_color_variable_max - 2.0f ) <= 1.0e-5f;
}

} // namespace pbvr
