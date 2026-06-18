# BENCHMARK PLAN

## Goal

Compare serial and MPI+OpenMP PBVR runs while identifying performance regressions from local compute, MPI communication, synchronization, I/O, compositing/gather, and OpenMP load imbalance.

## Phase 1: Correctness

Cases:

- `correctness_serial`
- `correctness_mpi_1x1`
- `correctness_omp_1x8`
- `correctness_hybrid_4x4`

Checks:

- Output image/file equality when deterministic.
- Tolerant numeric comparison for floating-point text outputs.
- Particle count, polygon count, sample count.
- min/max values.
- Representative grid/cell/particle values.
- Histogram totals.

If correctness fails, stop before performance interpretation.

## Phase 2: Same Problem Size

- Fix the global problem size.
- Compare serial and optimized variants.
- For parallel section time, use `mpi_max_sec` rather than only average.
- Produce rank min/avg/max/std/max-over-avg where raw rank logs are available.

## Phase 3: Strong Scaling

- Keep global problem size fixed.
- Increase MPI processes and/or OpenMP threads.
- Compute speedup and parallel efficiency.
- Use total cores = `nodes * mpiprocs * ompthreads`.

## Phase 4: Weak Scaling

- Keep problem size per rank approximately fixed.
- Increase nodes/processes and global size.
- Watch global communication, compositing/gather, and I/O sections.

## Phase 5: MPI x OpenMP Sweep

For one 40-core node:

- `40 MPI x 1 thread`
- `20 MPI x 2 threads`
- `10 MPI x 4 threads`
- `5 MPI x 8 threads`
- `4 MPI x 10 threads`
- `2 MPI x 20 threads`
- `1 MPI x 40 threads`

`NCPUS_PER_NODE` is configurable in `config.sh`.

## Automatic Judgement Rules

### max/avg

| Value | Meaning |
| --- | --- |
| 1.00-1.05 | Good |
| 1.05-1.20 | Mild imbalance |
| >=1.20 | Needs review |
| >=2.00 | Strong load imbalance or wait time |

### Contribution

`contribution = opt_section_max / opt_total_max`

Large contribution sections are prioritized first.

### Section speedup

`section_speedup = serial_section_time / opt_section_max`

Values below 1.0 indicate a section that became slower in parallel.

### Parallel efficiency

`parallel_efficiency = speedup / total_cores`

## Generated Workflow

```sh
./build.sh
python3 generate_benchmark_cases.py
python3 generate_pbs_jobs.py
./submit_correctness.sh
# after correctness passes
./submit_strong_scaling.sh
./submit_weak_scaling.sh
./submit_mpi_omp_sweep.sh
python3 parse_timing.py --input benchmark_results
python3 summarize_results.py
python3 make_report.py
```
