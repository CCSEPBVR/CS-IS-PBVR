# CODE STRUCTURE REPORT

## Scope

This report summarizes the PBVR server-side repository for serial, MPI, OpenMP, visualization, communication, I/O, and timer analysis. Large-scale performance measurement is intentionally not performed on the local PC.

## Supercomputer Manual Notes

The attached SGI user guide takes precedence for PBS and runtime settings. The benchmark scripts therefore use:

- PBS Professional directives with `select`, `ncpus`, `mpiprocs`, `ompthreads`, `walltime`, and `qsub`.
- Environment Modules initialized by `. /etc/profile.d/modules.sh`.
- `module load intel/cur` for Intel oneAPI.
- `module load mpt/cur` for HPE MPT.
- `mpirun omplace` for MPI and MPI+OpenMP placement.
- `KMP_AFFINITY=disabled` when using Intel OpenMP with placement tools.
- `OMP_NUM_THREADS` from each case's `ompthreads` value.

## Repository Layout

| Item | Location | Notes |
| --- | --- | --- |
| Top-level build | `Makefile`, `pbvr.conf`, `arch/` | `pbvr.conf` currently selects `Makefile_machine_s86_mpi_omp`. |
| Serial machine file | `arch/Makefile_machine_s86` | Intel compiler, no MPI/OpenMP flags in the machine file. |
| OpenMP machine file | `arch/Makefile_machine_s86_omp` | Intel compiler with OpenMP flags. |
| MPI+OpenMP machine file | `arch/Makefile_machine_s86_mpi_omp` | `mpicxx`, `mpicc`, `mpif90`; `-qopenmp`, `-xCORE-AVX2`. |
| Main server app | `App/main.cpp`, `App/Server.cpp`, `App/ServerWorker.cpp` | MPI init/finalize and client/server task broadcast. |
| Shared in-situ code | `InSituLib/shared/` | Particle gather, particle property, async particle write helpers. |
| Structured volume path | `InSituLib/struct/kvs_wrapper.cpp` | Structured PBVR wrapper. |
| Unstructured volume path | `InSituLib/unstruct/kvs_wrapper.cpp` | Primary target for ensemble particle generation and timers. |
| AMR path | `InSituLib/AMR/kvs_wrapper.cpp` | AMR particle generation and older timing output. |
| Function parser | `FunctionParser/` | Expression/RPN/TF parsing and tests. |
| Visualization module | `VisModule/` | Cell classes, renderer, compositor, file formats, utility timer. |
| C examples | `Example/C/s86_mpi_omp/` | Hydrogen and ensemble examples for MPI/OpenMP testing. |
| Fortran examples | `Example/Fortran/` | Fortran integration examples. |

## Build Method

Primary build entry:

```sh
make PBVR_MACHINE=Makefile_machine_s86_mpi_omp -j ${MAKE_JOBS}
```

The benchmark wrapper uses `build.sh`, which loads `intel/cur` and `mpt/cur` on PBS systems, then builds the repository and the configured example directory.

## Executable

Default benchmark executable:

```text
Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/run
```

This can be overridden by `EXECUTABLE` in `config.sh` or the environment.

## Input Data

| Input kind | Location | Notes |
| --- | --- | --- |
| Example JSON | `Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/default.json` | Visualization/ensemble settings. |
| Ensemble statistic JSON | `Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/default_ensemble_statistics.json` | Ensemble statistic TF settings. |
| Generated Hydrogen data | `Example/C/s86_mpi_omp/*/Hydrogen.cpp` | Many examples synthesize data in code. |
| External data | User-provided at runtime | Configure via `INPUT_ARGS` and case CSV. |

## Output Data

| Output | Location | Notes |
| --- | --- | --- |
| Particle files | Example/runtime output directory | Existing PBVR particle output path depends on wrapper parameters. |
| Timer summary | `ensemble_timer_summary.csv` | Written by `InSituLib/unstruct/kvs_wrapper.cpp`. |
| Benchmark logs | `benchmark_results/<case>/<case>.log` | Generated PBS scripts redirect stdout/stderr here. |
| Parsed timing CSV | `benchmark_analysis/*.csv` | Produced by `parse_timing.py` and `summarize_results.py`. |

## Test Code

| Test | Location | Notes |
| --- | --- | --- |
| Function parser test | `FunctionParser/test.cpp` | Expression/parser unit-style test. |
| Hydrogen unstructured | `Example/C/s86_mpi_omp/Hydrogen_unstruct/` | MPI example; main loop may continue across timesteps. |
| Ensemble Hydrogen MPI4 | `Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/` | Includes sample timer CSVs for parser validation. |
| AMR Hydrogen | `Example/C/s86_mpi_omp/Hydrogen_AMR/` | AMR path check. |

Local smoke tests should avoid long-running examples by default. Use `RUN_EXECUTABLE_SMOKE=1 ./run_local_tests.sh` only after confirming the executable terminates for the chosen input.

## Major Visualization Files

| Area | Files |
| --- | --- |
| Unstructured particle generation | `InSituLib/unstruct/kvs_wrapper.cpp` |
| Structured particle generation | `InSituLib/struct/kvs_wrapper.cpp` |
| AMR particle generation | `InSituLib/AMR/kvs_wrapper.cpp` |
| Cell interpolation/gradient | `VisModule/Visualization/Mapper/*Cell.*` |
| Particle rendering/compositing | `VisModule/Visualization/Renderer/ParticleBufferCompositor.cpp`, `ParticleBufferAccumulator.cpp`, `ParticleVolumeRenderer.*` |
| Transfer functions | `FunctionParser/`, `VisModule/Visualization/Mapper/TransferFunction*` |

## Major MPI Communication Files

| File | Role |
| --- | --- |
| `App/main.cpp` | `MPI_Init`, rank dispatch, `MPI_Finalize`. |
| `App/ServerWorker.cpp` | Server-worker task broadcast. |
| `App/TaskSignalTransferProtocol.cpp` | Broadcast task/parameter payloads. |
| `InSituLib/shared/kvs_wrapper_common.cpp` | Particle gather and global min/max/histogram reduction. |
| `InSituLib/unstruct/kvs_wrapper.cpp` | Particle gather, ensemble shift exchange, timers, min/max reductions. |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | Ensemble statistic moment/minmax/histogram collectives. |
| `InSituLib/AMR/kvs_wrapper.cpp` | AMR particle gather and reductions. |

## Major OpenMP Locations

| File | Role |
| --- | --- |
| `InSituLib/unstruct/kvs_wrapper.cpp` | Uniform sampling, shift interpolation, rejection, particle merge. |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | Cell-center evaluation, statistic min/max, histogram loops. |
| `InSituLib/AMR/kvs_wrapper.cpp` | AMR sampling loops and merge sections. |
| `VisModule/DaemonAndSampler/Daemon/ParticleFile.cpp` | Parallel particle file processing. |

## Timer Output Format

Current ensemble timer output is CSV:

```text
step,scope,parent_section,section,level,mpi_avg_sec,mpi_max_sec,mpi_min_sec,thread_avg_sec,thread_max_sec,thread_min_sec,uniform_particle_count,ave_particle_count,var_particle_count,cov_particle_count
```

The benchmark parser treats `mpi_max_sec` as the value to compare against serial timings.
