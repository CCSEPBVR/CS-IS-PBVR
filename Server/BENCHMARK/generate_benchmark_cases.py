#!/usr/bin/env python3
"""Generate PBS benchmark case matrix for PBVR MPI+OpenMP evaluation."""

import argparse
import csv
import os

FIELDNAMES = [
    "case", "phase", "nodes", "ncpus", "mpiprocs", "ompthreads",
    "input", "output_dir", "walltime", "executable", "input_args",
]


def env(name, default):
    return os.environ.get(name, default)


def add_case(rows, name, phase, nodes, mpiprocs, ompthreads,
             input_name, output_root, walltime, executable, input_args,
             ncpus_per_node):
    ncpus = max(1, min(ncpus_per_node, mpiprocs * ompthreads))
    rows.append({
        "case": name,
        "phase": phase,
        "nodes": nodes,
        "ncpus": ncpus,
        "mpiprocs": mpiprocs,
        "ompthreads": ompthreads,
        "input": input_name,
        "output_dir": "%s/%s" % (output_root, name),
        "walltime": walltime,
        "executable": executable,
        "input_args": input_args,
    })


def build_cases():
    ncpus_per_node = int(env("NCPUS_PER_NODE", "40"))
    output_root = env("OUTPUT_ROOT", "benchmark_results")
    executable = env("EXECUTABLE", "Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/run")
    input_args = env("INPUT_ARGS", "")
    small_input = env("CORRECTNESS_INPUT", "small")
    strong_input = env("STRONG_INPUT", "fixed_global")
    weak_input = env("WEAK_INPUT", "weak_per_rank")
    walltime = env("WALLTIME", "00:30:00")

    rows = []

    def add(name, phase, nodes, mpiprocs, ompthreads, input_name):
        add_case(rows, name, phase, nodes, mpiprocs, ompthreads,
                 input_name, output_root, walltime, executable, input_args,
                 ncpus_per_node)

    add("correctness_serial", "correctness", 1, 1, 1, small_input)
    add("correctness_mpi_1x1", "correctness", 1, 1, 1, small_input)
    add("correctness_omp_1x8", "correctness", 1, 1, 8, small_input)
    add("correctness_hybrid_4x4", "correctness", 1, 4, 4, small_input)

    for mpi in (1, 2, 4, 8):
        add("strong_%sx1" % mpi, "strong_scaling", 1, mpi, 1, strong_input)

    for nodes in (1, 2, 4):
        add("weak_%s" % nodes, "weak_scaling", nodes, ncpus_per_node, 1, weak_input)

    for mpi, omp in ((40, 1), (20, 2), (10, 4), (5, 8), (4, 10), (2, 20), (1, 40)):
        add("sweep_%sx%s" % (mpi, omp), "mpi_omp_sweep", 1, mpi, omp, strong_input)

    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", default=os.environ.get("CASES_CSV", "cases.csv"))
    args = parser.parse_args()

    rows = build_cases()
    with open(args.output, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=FIELDNAMES)
        writer.writeheader()
        writer.writerows(rows)
    print("wrote %s (%d cases)" % (args.output, len(rows)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
