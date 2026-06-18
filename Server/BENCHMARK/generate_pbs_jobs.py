#!/usr/bin/env python3
"""Generate PBS Pro job scripts from cases.csv.

This script is intended to run from Server/BENCHMARK. Generated jobs use
PBS_O_WORKDIR as BENCHMARK_DIR and execute the PBVR binary from REPO_ROOT=.. .
"""
from __future__ import annotations

import argparse
import csv
import os
from pathlib import Path


def env(name: str, default: str) -> str:
    return os.environ.get(name, default)


def shell_quote_arg(value: str) -> str:
    return value.replace("'", "'\\''")


def render(row: dict[str, str]) -> str:
    queue = env("QUEUE", "sc16")
    project = env("PROJECT", "job")
    module_intel = env("MODULE_INTEL", "intel/cur")
    module_mpi = env("MODULE_MPI", "mpt/cur")
    mpi_runner = env("MPI_RUNNER", "mpirun")
    placement_cmd = env("PLACEMENT_CMD", "omplace -nt ${OMP_NUM_THREADS}")
    executable = row.get("executable") or env("EXECUTABLE", "Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/run")
    input_args = row.get("input_args") or env("INPUT_ARGS", "")
    output_dir = row["output_dir"]
    log_file = f"{output_dir}/{row['case']}.log"
    job_name = row["case"][:15]
    project_line = f"#PBS -P {project}\n" if project else ""
    return f"""#!/bin/sh
#PBS -q {queue}
#PBS -l select={row['nodes']}:ncpus={row['ncpus']}:mpiprocs={row['mpiprocs']}:ompthreads={row['ompthreads']}
#PBS -l walltime={row['walltime']}
#PBS -N {job_name}
#PBS -j oe
{project_line}
set -eu

BENCHMARK_DIR=$PBS_O_WORKDIR
REPO_ROOT=$(CDPATH= cd -- "$BENCHMARK_DIR/.." && pwd)
cd "$REPO_ROOT"

. /etc/profile.d/modules.sh
module load {module_intel}
module load {module_mpi}

export OMP_NUM_THREADS={row['ompthreads']}
export KMP_AFFINITY=disabled

EXECUTABLE='{shell_quote_arg(executable)}'
case "$EXECUTABLE" in
  /*) EXEC_PATH="$EXECUTABLE" ;;
  *) EXEC_PATH="$REPO_ROOT/$EXECUTABLE" ;;
esac

OUTPUT_DIR="$BENCHMARK_DIR/{shell_quote_arg(output_dir)}"
LOG_FILE="$BENCHMARK_DIR/{shell_quote_arg(log_file)}"
mkdir -p "$OUTPUT_DIR"

echo "CASE={row['case']}"
echo "PHASE={row['phase']}"
echo "PBS_NODEFILE=$PBS_NODEFILE"
echo "REPO_ROOT=$REPO_ROOT"
echo "BENCHMARK_DIR=$BENCHMARK_DIR"
echo "OMP_NUM_THREADS=$OMP_NUM_THREADS"
echo "COMMAND={mpi_runner} {placement_cmd} $EXEC_PATH {input_args}"

{mpi_runner} {placement_cmd} "$EXEC_PATH" {input_args} > "$LOG_FILE" 2>&1
"""


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cases", default=env("CASES_CSV", "cases.csv"))
    parser.add_argument("--job-root", default=env("JOB_ROOT", "pbs_jobs"))
    args = parser.parse_args()

    root = Path(args.job_root)
    root.mkdir(parents=True, exist_ok=True)
    generated: list[Path] = []

    with open(args.cases, newline="") as f:
        for row in csv.DictReader(f):
            phase_dir = root / row["phase"]
            phase_dir.mkdir(parents=True, exist_ok=True)
            path = phase_dir / f"{row['case']}.pbs"
            path.write_text(render(row), encoding="utf-8")
            path.chmod(0o755)
            generated.append(path)

    print(f"generated {len(generated)} PBS scripts under {root}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
