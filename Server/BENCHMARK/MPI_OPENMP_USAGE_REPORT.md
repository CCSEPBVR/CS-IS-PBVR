# MPI / OpenMP USAGE REPORT

## MPI Calls

| File | Line | MPI call | Purpose | Communication target | Bottleneck candidate |
| --- | ---: | --- | --- | --- | --- |
| `App/main.cpp` | 19 | `MPI_Init` | Start MPI runtime | all ranks | No, initialization only |
| `App/main.cpp` | 25 | `MPI_Comm_rank` | Rank role selection | `MPI_COMM_WORLD` | No |
| `App/main.cpp` | 26 | `MPI_Comm_size` | Process count | `MPI_COMM_WORLD` | No |
| `App/main.cpp` | 43 | `MPI_Finalize` | End MPI runtime | all ranks | Possible final wait only |
| `App/ServerWorker.cpp` | 49 | `MPI_Bcast` | Broadcast task payload size | root 0 to workers | Yes if many small tasks |
| `App/ServerWorker.cpp` | 61 | `MPI_Bcast` | Broadcast task payload | root 0 to workers | Yes, rank 0 fan-out |
| `App/TaskSignalTransferProtocol.cpp` | 19 | `MPI_Bcast` | Broadcast control message size | root 0 to workers | Possible synchronization |
| `App/TaskSignalTransferProtocol.cpp` | 34 | `MPI_Bcast` | Broadcast control message body | root 0 to workers | Possible synchronization |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 519 | `MPI_Allgather` | Gather processor names | split communicator setup | Low, setup only |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 600 | `MPI_Allgather` | Gather particle counts | render/merge ranks | Yes, count imbalance visible |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 620 | `MPI_Gatherv` | Gather particle coordinates | rank 0 of split communicator | High, rank 0 concentration |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 624 | `MPI_Gatherv` | Gather particle colors | rank 0 of split communicator | High, rank 0 concentration |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 628 | `MPI_Gatherv` | Gather particle normals | rank 0 of split communicator | High, rank 0 concentration |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 676 | `MPI_Reduce` | Color histogram reduction | rank 0 | Medium, collective reduction |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 699 | `MPI_Reduce` | Global max values | rank 0 | Medium |
| `InSituLib/shared/kvs_wrapper_common.cpp` | 700 | `MPI_Reduce` | Global min values | rank 0 | Medium |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 301 | `MPI_Reduce` | Timer particle/count aggregate | rank 0 | Low/medium, diagnostic overhead |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 347 | `MPI_Reduce` | Timer avg/max/min aggregate | rank 0 | Low/medium, diagnostic overhead |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 481 | `MPI_Allgather` | Processor names for communicator split | world | Low |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 540 | `MPI_Allgather` | Particle count gather | split communicator | Yes, imbalance indicator |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 563 | `MPI_Gatherv` | Particle coordinate gather | rank 0 of split communicator | High, compositing/gather concentration |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 566 | `MPI_Gatherv` | Particle color gather | rank 0 of split communicator | High |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 569 | `MPI_Gatherv` | Particle normal gather | rank 0 of split communicator | High |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 826 | `MPI_Reduce` | Ensemble statistic min range | rank 0 or supplied comm root | Medium |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 828 | `MPI_Reduce` | Ensemble statistic histogram | rank 0 or supplied comm root | Medium/high with many bins |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 1303 | `MPI_Barrier` | Wait for file write completion | all world ranks | High if I/O imbalance exists |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2430 | `MPI_Sendrecv` | Stable ensemble ring shift size exchange | neighbor ranks | Medium/high if payload imbalance |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2439 | `MPI_Isend` | Nonblocking shift size send | neighbor rank | Medium; overlap limited by immediate wait |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2440 | `MPI_Irecv` | Nonblocking shift size receive | neighbor rank | Medium; overlap limited by immediate wait |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2441 | `MPI_Waitall` | Complete size exchange | neighbor rank | High wait-time candidate |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2542 | `MPI_Irecv` | Shift payload receive arrays | neighbor rank | High for particle-heavy cases |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2548 | `MPI_Isend` | Shift payload send arrays | neighbor rank | High for particle-heavy cases |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2554 | `MPI_Waitall` | Complete payload receives | neighbor rank | High wait-time candidate |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2555 | `MPI_Waitall` | Complete payload sends | neighbor rank | High wait-time candidate |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 3009 | `MPI_Barrier` | Synchronize state/final stage | all world ranks | High if previous stages imbalanced |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 3536 | `MPI_Allreduce` | Global coordinate min | all world ranks | Medium, collective per coordinate |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 235 | `MPI_Allreduce` | Cell-center `sum(x)` for statistics | ensemble communicator | High memory/collective candidate |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 236 | `MPI_Allreduce` | Cell-center `sum(x^2)` for statistics | ensemble communicator | High memory/collective candidate |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 592 | `MPI_Allreduce` | Statistic global min | ensemble communicator | Medium |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 593 | `MPI_Allreduce` | Statistic global max | ensemble communicator | Medium |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 695 | `MPI_Reduce` | Statistic histogram bins | root rank | Medium/high if bins/variables increase |
| `InSituLib/AMR/kvs_wrapper.cpp` | 1308 | `MPI_Allgather` | AMR particle counts | split communicator | Medium |
| `InSituLib/AMR/kvs_wrapper.cpp` | 1319 | `MPI_Gatherv` | AMR coordinates gather | root rank | High |
| `InSituLib/AMR/kvs_wrapper.cpp` | 1388 | `MPI_Reduce` | AMR opacity histogram | root rank | Medium |

## MPI Risk Summary

- Rank 0 concentration exists in particle `MPI_Gatherv` paths for coordinates/colors/normals.
- Ensemble shift exchange uses neighbor communication and immediate `MPI_Waitall`, so nonblocking communication may not overlap much with computation.
- `MPI_Barrier` around I/O/final states can expose slowest-rank behavior.
- Ensemble cell histogram uses full cell-by-variable arrays in `MPI_Allreduce`; this is correct for statistics but can be expensive at large `ncells * nvariables`.
- Compositing/gather and I/O should be analyzed with max time and max/avg imbalance, not average alone.

## OpenMP Constructs

| File | Line | OpenMP construct | Target loop/process | schedule | reduction | Bottleneck candidate |
| --- | ---: | --- | --- | --- | --- | --- |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 1938 | `#pragma omp parallel` | Uniform particle generation region | N/A | thread-local collector | Yes, large main kernel |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 1997 | `#pragma omp for schedule(dynamic) nowait` | Uniform cell sampling loop | dynamic | no direct reduction | Yes, dynamic overhead/load balance tradeoff |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2298 | `#pragma omp critical` | Merge thread-local particle buffers | N/A | manual merge | Yes, serialized merge |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2566 | `#pragma omp parallel` | Shift interpolation/statistic region | N/A | thread-local collector | Yes |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2598 | `#pragma omp for` | Shift particle interpolation loop | default static | no direct reduction | Yes if payload per rank is imbalanced |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2782 | `#pragma omp parallel` | Rejection/statistic output preparation | N/A | thread-local | Medium |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2808 | `#pragma omp for schedule(dynamic)` | Rejection loop | dynamic | no direct reduction | Medium |
| `InSituLib/unstruct/kvs_wrapper.cpp` | 2862 | `#pragma omp critical` | Rejection thread merge | N/A | manual merge | Medium/high for many particles |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 181 | `#pragma omp parallel` | Cell-center value evaluation | N/A | thread-local | High for large cells |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 190 | `#pragma omp for schedule(dynamic)` | Cell-center evaluation loop | dynamic | no | Medium/high |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 270 | `#pragma omp parallel` | Local statistic min/max | N/A | thread-local | Medium |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 277 | `#pragma omp for schedule(dynamic)` | Statistic min/max loop | dynamic | manual | Medium |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 308 | `#pragma omp critical` | Merge thread-local min/max | N/A | manual | Low/medium; variables only |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 353 | `#pragma omp parallel` | Local statistic histogram | N/A | thread-local histogram | Medium/high memory footprint |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 359 | `#pragma omp for schedule(dynamic)` | Histogram loop | dynamic | manual | Medium |
| `InSituLib/unstruct/EnsembleCellHistogram.cpp` | 377 | `#pragma omp critical` | Merge histograms | N/A | manual | Medium with many variables/bins |
| `InSituLib/AMR/kvs_wrapper.cpp` | 793 | `#pragma omp parallel` | AMR sampling region | N/A | manual | High in AMR path |
| `InSituLib/AMR/kvs_wrapper.cpp` | 855 | `#pragma omp for` | AMR cell loop | default | no | Medium |
| `InSituLib/AMR/kvs_wrapper.cpp` | 991 | `#pragma omp for schedule(dynamic,1)` | AMR particle loop | dynamic,1 | no | High scheduling overhead candidate |
| `InSituLib/AMR/kvs_wrapper.cpp` | 1149 | `#pragma omp critical` | AMR merge | N/A | manual | High if many buffers |
| `VisModule/DaemonAndSampler/Daemon/ParticleFile.cpp` | 108 | `#pragma omp parallel` | Particle file processing | N/A | no | I/O/parallel file candidate |
| `VisModule/DaemonAndSampler/Daemon/ParticleFile.cpp` | 115 | `#pragma omp for` | Particle file loop | default | no | Medium |

## OpenMP Risk Summary

- The uniform particle loop uses dynamic scheduling; good for volume-dependent load, but scheduling overhead should be checked with thread max/avg.
- Thread-local buffers avoid atomics in hot particle loops, but final `critical` merge can become visible for high particle counts.
- Histogram code uses thread-local histograms and critical merge, which is reasonable for correctness; merge cost grows with `nthreads * nvariables * nbins`.
- False sharing is less likely in thread-local vectors, but SoA/AoS layout and scattered cell access can limit SIMD and cache efficiency.
- NUMA placement should be controlled by PBS `ompthreads`, `omplace`, and first-touch behavior in production runs.

## Raw Search CSVs

For complete grep-style search results, see:

- `mpi_calls.csv`
- `openmp_usage.csv`

The Markdown tables above intentionally prioritize the calls most relevant to performance diagnosis.
