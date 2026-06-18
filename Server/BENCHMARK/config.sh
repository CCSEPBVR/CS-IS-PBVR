#!/bin/sh
# Benchmark configuration for PBVR on SGI/PBS systems.
# This file lives in Server/BENCHMARK. The PBVR source root is one level above.

BENCHMARK_DIR=${BENCHMARK_DIR:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}
REPO_ROOT=${REPO_ROOT:-$(CDPATH= cd -- "$BENCHMARK_DIR/.." && pwd)}

QUEUE=${QUEUE:-sc16}
PROJECT=${PROJECT:-job}
WALLTIME=${WALLTIME:-00:30:00}
NCPUS_PER_NODE=${NCPUS_PER_NODE:-40}

MODULE_CUDA=${MODULE_CUDA:-cuda/11.4}
MODULE_GNU=${MODULE_GNU:-gnu/cur}
MODULE_INTEL=${MODULE_INTEL:-intel/2023.2.1}
MODULE_MPI=${MODULE_MPI:-mpt/2.23-ga}
MPI_RUNNER=${MPI_RUNNER:-mpirun}
PLACEMENT_CMD=${PLACEMENT_CMD:-'omplace -nt ${OMP_NUM_THREADS}'}
OPENMP_PLACEMENT_CMD=${OPENMP_PLACEMENT_CMD:-dplace}

PBVR_MACHINE_MPI_OMP=${PBVR_MACHINE_MPI_OMP:-Makefile_machine_s86_mpi_omp}
PBVR_MACHINE_SERIAL=${PBVR_MACHINE_SERIAL:-Makefile_machine_s86}
MAKE_JOBS=${MAKE_JOBS:-4}

# EXECUTABLE is relative to REPO_ROOT unless an absolute path is supplied.
EXECUTABLE=${EXECUTABLE:-Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/run}
INPUT_ARGS=${INPUT_ARGS:-}

# Benchmark outputs are relative to BENCHMARK_DIR unless absolute paths are supplied.
OUTPUT_ROOT=${OUTPUT_ROOT:-benchmark_results}
JOB_ROOT=${JOB_ROOT:-pbs_jobs}
ANALYSIS_DIR=${ANALYSIS_DIR:-benchmark_analysis}
CASES_CSV=${CASES_CSV:-cases.csv}

# Set BUILD_SERIAL=1 only when a serial build target is available for the chosen example.
BUILD_SERIAL=${BUILD_SERIAL:-0}
# Local executable smoke tests are disabled by default because several examples run time-step loops.
RUN_EXECUTABLE_SMOKE=${RUN_EXECUTABLE_SMOKE:-0}
