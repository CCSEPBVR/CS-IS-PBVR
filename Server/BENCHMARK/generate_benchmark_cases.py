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


DEFAULT_EXECUTABLE = "Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/run"
DEFAULT_WEAK_EXECUTABLE = "Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_4eweak_scale/run"
# Strong scaling now uses a SINGLE executable that keeps the global problem
# size N fixed and only changes how many ranks split one ensemble at runtime
# (mpi_per_ens = mpi_size / num_ensembles). This makes the strong_* series a
# valid strong-scaling experiment (N fixed, p increases).
DEFAULT_STRONG_EXECUTABLE = "Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_strong/run"
DEFAULT_STRONG_EXECUTABLES = {
    4: DEFAULT_STRONG_EXECUTABLE,
    8: DEFAULT_STRONG_EXECUTABLE,
    16: DEFAULT_STRONG_EXECUTABLE,
    32: DEFAULT_STRONG_EXECUTABLE,
    64: DEFAULT_STRONG_EXECUTABLE,
}


def add_case(rows, name, phase, nodes, mpiprocs, ompthreads,
             input_name, output_root, walltime, executable, input_args,
             ncpus_per_node):
    if mpiprocs < 2:
        return
    if mpiprocs * ompthreads > ncpus_per_node:
        return
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


def split_total_mpi(total_mpi, ompthreads, ncpus_per_node):
    nodes = max(1, (total_mpi * ompthreads + ncpus_per_node - 1) // ncpus_per_node)
    while total_mpi % nodes != 0:
        nodes += 1
    mpiprocs = total_mpi // nodes
    ncpus = mpiprocs * ompthreads
    if ncpus > ncpus_per_node:
        return None
    return nodes, mpiprocs


def build_cases(executable_arg=None, input_args_arg=None, weak_executable_arg=None):
    ncpus_per_node = int(env("NCPUS_PER_NODE", "40"))
    weak_base_ens = max(2, int(env("PBVR_WEAK_BASE_ENS", "4")))
    output_root = env("OUTPUT_ROOT", "benchmark_results")
    executable = executable_arg or env("EXECUTABLE", DEFAULT_EXECUTABLE)
    weak_executable = weak_executable_arg or env("WEAK_EXECUTABLE", DEFAULT_WEAK_EXECUTABLE)
    input_args = input_args_arg if input_args_arg is not None else env("INPUT_ARGS", "")
    small_input = env("CORRECTNESS_INPUT", "small")
    strong_input = env("STRONG_INPUT", "fixed_global")
    weak_input = env("WEAK_INPUT", "weak_per_rank")
    walltime = env("WALLTIME", "00:30:00")

    rows = []

    def add(name, phase, nodes, mpiprocs, ompthreads, input_name, case_executable=None):
        add_case(rows, name, phase, nodes, mpiprocs, ompthreads,
                 input_name, output_root, walltime, case_executable or executable, input_args,
                 ncpus_per_node)

    def add_total_mpi(name, phase, total_mpi, ompthreads, input_name, case_executable=None):
        placement = split_total_mpi(total_mpi, ompthreads, ncpus_per_node)
        if placement is None:
            return
        nodes, mpiprocs = placement
        add(name, phase, nodes, mpiprocs, ompthreads, input_name, case_executable)

    add("correctness_mpi_2x1", "correctness", 1, 2, 1, small_input)
    add("correctness_mpi_2x2", "correctness", 1, 2, 2, small_input)
    add("correctness_hybrid_4x4", "correctness", 1, 4, 4, small_input)

    for mpi in (4, 8, 16, 32, 64):
        add_total_mpi(
            "strong_%sx1" % mpi,
            "strong_scaling",
            mpi,
            1,
            strong_input,
            DEFAULT_STRONG_EXECUTABLES.get(mpi, executable),
        )

    for mpi in (10, 20, 40, 80):
        add_total_mpi("weak_%s" % mpi, "weak_scaling", mpi, 1, weak_input, weak_executable)

    for mpi, omp in ((40, 1), (20, 2), (10, 4), (8, 5), (5, 8), (4, 10), (2, 20)):
        add("sweep_%sx%s" % (mpi, omp), "mpi_omp_sweep", 1, mpi, omp, strong_input)

    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", default=os.environ.get("CASES_CSV", "cases.csv"))
    parser.add_argument("--executable", default=None)
    parser.add_argument("--weak-executable", default=None)
    parser.add_argument("--input-args", default=None)
    args = parser.parse_args()

    rows = build_cases(args.executable, args.input_args, args.weak_executable)
    with open(args.output, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=FIELDNAMES)
        writer.writeheader()
        writer.writerows(rows)
    print("wrote %s (%d cases)" % (args.output, len(rows)))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
