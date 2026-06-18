#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$SCRIPT_DIR/config.sh"
cd "$BENCHMARK_DIR"
find "$JOB_ROOT/mpi_omp_sweep" -name '*.pbs' -print | sort | while IFS= read -r job; do qsub "$job"; done
