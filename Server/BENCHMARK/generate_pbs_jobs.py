#!/usr/bin/env python3
"""Generate PBS Pro job scripts from cases.csv.

This script is intended to run from Server/BENCHMARK. Generated jobs use
PBS_O_WORKDIR as BENCHMARK_DIR and execute the PBVR binary from REPO_ROOT=.. .
"""

import argparse
import csv
import os
from pathlib import Path


def env(name, default):
    return os.environ.get(name, default)


def shell_quote_arg(value):
    return value.replace("'", "'\\''")


def render(row):
    queue = env("QUEUE", "sc16")
    project = env("PROJECT", "job")
    module_cuda = env("MODULE_CUDA", "cuda/11.4")
    module_gnu = env("MODULE_GNU", "gnu/cur")
    module_intel = env("MODULE_INTEL", "intel/2023.2.1")
    module_mpi = env("MODULE_MPI", "mpt/2.23-ga")
    mpi_runner = env("MPI_RUNNER", "mpirun")
    placement_cmd = env("PLACEMENT_CMD", "omplace -nt ${OMP_NUM_THREADS}")
    executable = row.get("executable") or env("EXECUTABLE", "Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/run")
    input_args = row.get("input_args") or env("INPUT_ARGS", "")
    output_dir = row["output_dir"]
    log_file = "%s/%s.log" % (output_dir, row["case"])
    job_name = row["case"][:15]
    project_line = "#PBS -P %s\n" % project if project else ""
    module_lines = []
    for module in (module_cuda, module_gnu, module_intel, module_mpi):
        if module:
            module_lines.append("module load %s" % module)
    module_block = "\n".join(module_lines)
    return """#!/bin/sh
#PBS -q {queue}
#PBS -l select={nodes}:ncpus={ncpus}:mpiprocs={mpiprocs}:ompthreads={ompthreads}
#PBS -l walltime={walltime}
#PBS -N {job_name}
#PBS -j oe
{project_line}
set -eu

BENCHMARK_DIR=$PBS_O_WORKDIR
REPO_ROOT=$(CDPATH= cd -- "$BENCHMARK_DIR/.." && pwd)
cd "$REPO_ROOT"

. /etc/profile.d/modules.sh
{module_block}

export OMP_NUM_THREADS={ompthreads}
export KMP_AFFINITY=disabled

EXECUTABLE='{executable}'
case "$EXECUTABLE" in
  /*) EXEC_PATH="$EXECUTABLE" ;;
  *) EXEC_PATH="$REPO_ROOT/$EXECUTABLE" ;;
esac

OUTPUT_DIR="$BENCHMARK_DIR/{output_dir}"
LOG_FILE="$BENCHMARK_DIR/{log_file}"
mkdir -p "$OUTPUT_DIR"

echo "CASE={case}"
echo "PHASE={phase}"
echo "PBS_NODEFILE=$PBS_NODEFILE"
echo "REPO_ROOT=$REPO_ROOT"
echo "BENCHMARK_DIR=$BENCHMARK_DIR"
echo "OMP_NUM_THREADS=$OMP_NUM_THREADS"
echo "COMMAND={mpi_runner} {placement_cmd} $EXEC_PATH {input_args}"

{mpi_runner} {placement_cmd} "$EXEC_PATH" {input_args} > "$LOG_FILE" 2>&1
""".format(
        queue=queue,
        nodes=row["nodes"],
        ncpus=row["ncpus"],
        mpiprocs=row["mpiprocs"],
        ompthreads=row["ompthreads"],
        walltime=row["walltime"],
        job_name=job_name,
        project_line=project_line,
        module_block=module_block,
        executable=shell_quote_arg(executable),
        output_dir=shell_quote_arg(output_dir),
        log_file=shell_quote_arg(log_file),
        case=row["case"],
        phase=row["phase"],
        mpi_runner=mpi_runner,
        placement_cmd=placement_cmd,
        input_args=input_args,
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cases", default=env("CASES_CSV", "cases.csv"))
    parser.add_argument("--job-root", default=env("JOB_ROOT", "pbs_jobs"))
    args = parser.parse_args()

    root = Path(args.job_root)
    root.mkdir(parents=True, exist_ok=True)
    generated = []

    with open(args.cases, newline="") as f:
        for row in csv.DictReader(f):
            phase_dir = root / row["phase"]
            phase_dir.mkdir(parents=True, exist_ok=True)
            path = phase_dir / (row["case"] + ".pbs")
            path.write_text(render(row), encoding="utf-8")
            path.chmod(0o755)
            generated.append(path)

    print("generated %d PBS scripts under %s" % (len(generated), root))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
