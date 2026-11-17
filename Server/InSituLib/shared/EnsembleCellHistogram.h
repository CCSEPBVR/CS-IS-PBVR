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

namespace vismodule
{

struct EnsembleCellHistogramLog
{
    int nvariables;
    int ncells;
    int nbins;
    int comm_size;
    std::size_t histogram_bytes;
    double cell_center_eval_seconds;
    double ensemble_moments_mpi_seconds;
    double statistics_seconds;
    double local_minmax_seconds;
    double global_minmax_mpi_seconds;
    double local_histogram_seconds;
    double global_histogram_mpi_seconds;

    EnsembleCellHistogramLog();
};

struct EnsembleStatisticRange
{
    std::vector<float> min_values;
    std::vector<float> max_values;
    std::vector<vismodule::UInt64> o_bins;
    std::vector<vismodule::UInt64> c_bins;
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
    std::vector<int>& local_histogram,
    EnsembleCellHistogramLog* log = NULL );

bool ReduceGlobalHistogram(
    const std::vector<int>& local_histogram,
    std::vector<int>& global_histogram,
    int nvariables,
    int nbins,
    MPI_Comm comm,
    EnsembleCellHistogramLog* log = NULL );

bool WriteHistogramResult(
    const std::string& filename,
    const std::vector<float>& global_min,
    const std::vector<float>& global_max,
    const std::vector<int>& global_histogram,
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
    const std::vector<int>& global_histogram,
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
    EnsembleStatisticRange& average_range,
    EnsembleStatisticRange& variance_range,
    EnsembleStatisticRange& co_variation_range,
    EnsembleCellHistogramLog* log = NULL );

/*  構造格子版。connections が無いのでセル重心の値は三線形補間器で評価する。
 *  非構造版 ComputeAndStoreEnsembleCellHistogram との違いは第1段(セル重心値の
 *  評価)だけで、アンサンブル統計量・ヒストグラム・伝達関数への格納は同じ経路を通る。
 *
 *  nx/ny/nz は節点数(dom.resolution)。セル数は (nx-1)(ny-1)(nz-1) になる。
 *  domain_parameters_struct を引数に取らないのは、このヘッダに
 *  CellByCellParticleGenerator への依存を持ち込まないため。 */
bool ComputeAndStoreEnsembleCellHistogramStruct(
    Type** values,
    int nvariables,
    int nx,
    int ny,
    int nz,
    MPI_Comm ensemble_comm,
    int nbins,
    ParticleProperty& particle_property,
    EnsembleStatisticRange& average_range,
    EnsembleStatisticRange& variance_range,
    EnsembleStatisticRange& co_variation_range,
    EnsembleCellHistogramLog* log = NULL );

void PrintEnsembleCellHistogramLog(
    const EnsembleCellHistogramLog& log,
    std::ostream& os );

bool RunEnsembleCellHistogramSelfTest();

} // namespace vismodule

#endif // PBVR_ENSEMBLE_CELL_HISTOGRAM_H
