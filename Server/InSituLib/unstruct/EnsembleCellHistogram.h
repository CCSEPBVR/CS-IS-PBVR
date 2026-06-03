#ifndef PBVR_ENSEMBLE_CELL_HISTOGRAM_H
#define PBVR_ENSEMBLE_CELL_HISTOGRAM_H

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>
#include <vismodule/VolumeObjectBase>
#include <vismodule/ParticleProperty>

#ifdef DOUBLE_SCHEME
    typedef double Type;
#else
    typedef float Type;
#endif

#ifndef CPU_VER
#include <mpi.h>
#else
typedef int MPI_Comm;
#ifndef MPI_COMM_WORLD
#define MPI_COMM_WORLD 0
#endif
#endif

namespace pbvr
{

struct EnsembleCellHistogramLog
{
    int nvariables;
    int ncells;
    int nbins;
    int comm_size;
    std::size_t histogram_bytes;
    double local_minmax_seconds;
    double global_minmax_mpi_seconds;
    double local_histogram_seconds;
    double global_histogram_mpi_seconds;

    EnsembleCellHistogramLog();
};

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
    EnsembleCellHistogramLog* log = NULL );

bool ReduceGlobalMinMax(
    const std::vector<float>& local_min,
    const std::vector<float>& local_max,
    std::vector<float>& global_min,
    std::vector<float>& global_max,
    MPI_Comm comm,
    EnsembleCellHistogramLog* log = NULL );

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
    EnsembleCellHistogramLog* log = NULL );

bool ReduceGlobalHistogram(
    const std::vector<long long>& local_histogram,
    std::vector<long long>& global_histogram,
    int nvariables,
    int nbins,
    MPI_Comm comm,
    EnsembleCellHistogramLog* log = NULL );

bool WriteHistogramResult(
    const std::string& filename,
    const std::vector<float>& global_min,
    const std::vector<float>& global_max,
    const std::vector<long long>& global_histogram,
    int nvariables,
    int nbins,
    int mpi_rank );

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
    EnsembleCellHistogramLog* log = NULL );

bool StoreGlobalCellHistogramToParticleProperty(
    const std::vector<float>& global_min,
    const std::vector<float>& global_max,
    const std::vector<long long>& global_histogram,
    int nvariables,
    int nbins,
    int mpi_rank,
    ParticleProperty& particle_property );

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
    EnsembleCellHistogramLog* log = NULL );

void PrintEnsembleCellHistogramLog(
    const EnsembleCellHistogramLog& log,
    std::ostream& os );

bool RunEnsembleCellHistogramSelfTest();

} // namespace pbvr

#endif // PBVR_ENSEMBLE_CELL_HISTOGRAM_H
