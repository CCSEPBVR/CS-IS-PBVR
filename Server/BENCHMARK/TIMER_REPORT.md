# TIMER REPORT

## Timer Implementation

| Timer | File | Start line | End line | Target process | Rank-wise | Thread-wise | Usable for performance evaluation |
| --- | --- | ---: | ---: | --- | --- | --- | --- |
| `vismodule::Timer` | `VisModule/Utility/Timer.*` | N/A | N/A | General elapsed time using wall clock | Depends on caller | Depends on caller | Yes for coarse wall time; use MPI max for parallel comparison |
| `EnsembleTimerCollector` | `InSituLib/unstruct/kvs_wrapper.cpp` | 225 | 405 | Ensemble particle generation sections | Yes, MPI reduced avg/max/min | Yes, thread avg/max/min | Yes, primary existing timer |
| `EnsembleTimerScope` | `InSituLib/unstruct/kvs_wrapper.cpp` | 412 | 430 | RAII section timer | Accumulated per rank | No by itself | Yes |
| `ChainRuleTimingBreakdown` | `InSituLib/unstruct/kvs_wrapper.cpp` | around 1347 | around 1485 | Scalar + chain-rule gradient sub-sections | Folded into collector | Folded into collector | Yes, fine-grain local kernel diagnosis |
| `AMR time struct` | `InSituLib/AMR/kvs_wrapper.cpp` | around 502 | 508 | AMR initialize/sampling/write/reduce | MPI max reduced | No | Yes for AMR path |
| `particle_write_thread` timer | `InSituLib/shared/particle_write_thread.h` | 54 | 65 | Async particle file writing | No direct MPI reduction | Thread-local worker | Useful for I/O diagnosis |
| `thread_timer` helpers | `InSituLib/shared/thread_timer.h` | 113 | 140 | Generic chrono timed sections | Caller-dependent | Caller-dependent | Potentially useful, not primary output |

## Ensemble Timer CSV Format

`InSituLib/unstruct/kvs_wrapper.cpp` writes `ensemble_timer_summary.csv` with:

```text
step,scope,parent_section,section,level,mpi_avg_sec,mpi_max_sec,mpi_min_sec,thread_avg_sec,thread_max_sec,thread_min_sec,uniform_particle_count,ave_particle_count,var_particle_count,cov_particle_count
```

The scripts intentionally compare parallel runs using `mpi_max_sec`, because the slowest rank determines elapsed time.

## Existing Ensemble Sections

| Timer name / section | File | Start line | End line | Classification | Rank-wise | Thread-wise | Use |
| --- | --- | ---: | ---: | --- | --- | --- | --- |
| `total` | `InSituLib/unstruct/kvs_wrapper.cpp` | 88 | 146 | total | yes | no | Overall elapsed time |
| `set_parameter_path` | same | 88 | 146 | initialization | yes | no | Setup |
| `read_parameter_file` | same | 88 | 146 | initialization / I/O | yes | no | Parameter I/O |
| `init_transfer_functions` | same | 88 | 146 | initialization | yes | no | TF setup |
| `create_cells` | same | 88 | 146 | data_conversion | yes | no | Cell object creation |
| `sampling_prepare` | same | 88 | 146 | particle_generation | yes | no | Pre-sampling setup; verify if unexpectedly large |
| `omp_uniform_sampling` | same | 88 | 146 | OpenMP_parallel_region | yes | yes | Uniform particle generation region |
| `uniform_thread_setup` | same | 88 | 146 | initialization | yes | yes | Thread-local setup |
| `uniform_cell_index_setup` | same | 88 | 146 | data_conversion | yes | yes | Cell index preparation |
| `uniform_bind_cell_array` | same | 88 | 146 | interpolation | yes | yes | Cell binding |
| `uniform_volume_calculation` | same | 88 | 146 | local_compute | yes | yes | Cell volume calculation |
| `uniform_particle_count_calculation` | same | 88 | 146 | sampling | yes | yes | Particle count per cell |
| `uniform_particle_sampling_loop` | same | 88 | 146 | sampling | yes | yes | Random local coordinate and particle creation loop |
| `uniform_local_coord_generation` | same | 88 | 146 | sampling | yes | yes | Random/local coordinate generation |
| `uniform_flush_prepare` | same | 88 | 146 | data_conversion | yes | yes | Buffer flush prep |
| `uniform_calculate_scalars` | same | 88 | 146 | interpolation | yes | yes | Scalar interpolation |
| `uniform_chain_rule_grad` | same | 88 | 146 | local_compute | yes | yes | Chain-rule normal calculation |
| `uniform_calc_scalar_grad` | same | 88 | 146 | local_compute | yes | yes | Combined scalar/gradient helper total |
| `uniform_q_values_grad_q_setup` | same | 88 | 146 | interpolation | yes | yes | q and grad_q setup |
| `uniform_tf_scalar_eval` | same | 88 | 146 | visualization_kernel | yes | yes | TF scalar evaluation |
| `uniform_chain_rule_dfdq` | same | 88 | 146 | visualization_kernel | yes | yes | dF/dq calculation |
| `uniform_normal_normalize` | same | 88 | 146 | local_compute | yes | yes | Normal normalization |
| `uniform_store_particle_data` | same | 88 | 146 | data_conversion | yes | yes | Particle buffer append/store |
| `thread_particle_merge` | same | 88 | 146 | OpenMP_reduction / synchronization | yes | no | Critical merge candidate |
| `mpi_shift_exchange` | same | 88 | 146 | halo_exchange / MPI_collective | yes | no | Ensemble ring exchange |
| `mpi_shift_size_exchange` | same | 88 | 146 | MPI_collective | yes | no | Size exchange |
| `mpi_shift_alloc_recv_buffer` | same | 88 | 146 | data_conversion | yes | no | Receive buffer allocation |
| `mpi_shift_payload_all` | same | 88 | 146 | MPI_collective | yes | no | Payload send/receive |
| `mpi_shift_payload_*` | same | 88 | 146 | pack / unpack / MPI_collective | yes | no | Payload component diagnosis |
| `omp_shift_interpolation` | same | 88 | 146 | OpenMP_parallel_region | yes | yes | Shifted particle interpolation |
| `shift_calculate_scalars` | same | 88 | 146 | interpolation | yes | yes | Scalar interpolation for shifted particles |
| `shift_chain_rule_grad` | same | 88 | 146 | local_compute | yes | yes | Chain-rule normal for shifted particles |
| `shift_calc_scalar_grad` | same | 88 | 146 | local_compute | yes | yes | Combined shift helper total |
| `shift_q_values_grad_q_setup` | same | 88 | 146 | interpolation | yes | yes | Shift q and grad_q setup |
| `shift_tf_scalar_eval` | same | 88 | 146 | visualization_kernel | yes | yes | Shift TF eval |
| `shift_chain_rule_dfdq` | same | 88 | 146 | visualization_kernel | yes | yes | Shift dF/dq |
| `shift_normal_normalize` | same | 88 | 146 | local_compute | yes | yes | Shift normal normalize |
| `stat_average_variance` | same | 88 | 146 | local_compute | yes | no | Ensemble statistics |
| `stat_histogram` | same | 88 | 146 | local_compute / MPI_collective | yes | no | Histogram creation/reduction |
| `omp_rejection` | same | 88 | 146 | OpenMP_parallel_region | yes | yes | Rejection sampling/stat filtering |
| `rejection_thread_merge` | same | 88 | 146 | synchronization | yes | no | Critical merge |
| `cleanup_tfs` | same | 88 | 146 | finalization | yes | no | Cleanup |
| `output_coord_minmax` | same | 88 | 146 | I/O / MPI_collective | yes | no | Coordinate range output |
| `output_particles_*` | same | 88 | 146 | I/O | yes | no | Particle output |
| `output_history` | same | 88 | 146 | I/O | yes | no | History output |
| `async_io_wait` | same | 88 | 146 | I/O / synchronization | yes | no | Async write wait |
| `final_barrier_state` | same | 88 | 146 | synchronization | yes | no | Final barrier/state wait |

## Performance Evaluation Guidance

- Use `mpi_max_sec` for end-to-end and serial-vs-parallel comparisons.
- Use `max_over_avg` to detect rank imbalance or collective wait.
- Use `thread_max_sec / thread_avg_sec` manually for OpenMP imbalance where available.
- For I/O-inclusive and I/O-exclusive views, subtract or filter `output_*`, `async_io_wait`, and `final_barrier_state` sections.
- For compositing/gather diagnosis, inspect `MPI_Gatherv` sections indirectly via particle gather/output/compositing timers; add explicit timers only if current sections remain ambiguous.

## Raw Search CSV

For complete timer-related search results, see `timer_occurrences.csv`.
