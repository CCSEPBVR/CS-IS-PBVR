# BOTTLENECK CANDIDATES

This is the static pre-run candidate list. The generated `benchmark_analysis/bottleneck_candidates.csv` is the measurement-driven list.

| Priority | Section / area | Symptom | Evidence | Hypothesis | Next check |
| ---: | --- | --- | --- | --- | --- |
| 1 | `uniform_volume_calculation` | Often dominant in particle generation | Timers in `InSituLib/unstruct/kvs_wrapper.cpp`; cell volume called inside sampling | Expensive HexahedralCell volume/Jacobian or repeated geometry work | Compare with cached/tetra volume experiments and inspect SIMD reports |
| 1 | `uniform_particle_sampling_loop` | Heavy local compute | OpenMP dynamic loop around `InSituLib/unstruct/kvs_wrapper.cpp:1997` | Random coordinate generation, scalar interpolation, TF eval, particle push/store | Split by fine timers and check thread max/avg |
| 1 | `uniform_calc_scalar_grad` / `shift_calc_scalar_grad` | Chain-rule normal work | Fine timers at `InSituLib/unstruct/kvs_wrapper.cpp:1367` onward | q/grad setup, TF eval, numerical dF/dq, normal normalize | Use fine sections: q setup, TF scalar eval, dF/dq, normalize |
| 1 | `MPI_Gatherv` particle gather | Rank 0 concentration | `InSituLib/unstruct/kvs_wrapper.cpp:563-569` | Root memory bandwidth and network fan-in | Compare particle count imbalance and gather/output sections |
| 1 | `mpi_shift_exchange` / `MPI_Waitall` | Wait time / communication | `InSituLib/unstruct/kvs_wrapper.cpp:2439-2555` | Payload imbalance, no useful overlap before wait | Inspect payload component timers and rank max/avg |
| 2 | `thread_particle_merge` | OpenMP serialization | `InSituLib/unstruct/kvs_wrapper.cpp:2298` critical | Thread-local vector merge becomes serial | Measure merge contribution vs particle count |
| 2 | `async_io_wait` / `output_particles_*` | I/O wait | output timers in ensemble timer enum | Slow storage or rank 0 write concentration | Separate I/O-inclusive and I/O-exclusive totals |
| 2 | `EnsembleCellHistogram` moment `MPI_Allreduce` | Large collective payload | `InSituLib/unstruct/EnsembleCellHistogram.cpp:235-236` | `ncells * nvariables` float reductions | Measure collective time and memory footprint |
| 2 | `stat_histogram` | Histogram construction/reduction | `EnsembleCellHistogram.cpp:635-695` and timer enum | Thread-local histogram merge and MPI_Reduce | Check bins, variables, OpenMP merge cost |
| 3 | `MPI_Barrier` final/state sections | Slowest rank dominates | `InSituLib/unstruct/kvs_wrapper.cpp:1303`, `3009`, `3494` | Previous imbalance hidden by barrier | Correlate with previous max/avg sections |
