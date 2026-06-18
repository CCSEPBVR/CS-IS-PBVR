#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$SCRIPT_DIR/config.sh"
cd "$BENCHMARK_DIR"
find "$JOB_ROOT" -name '*.pbs' -print | sort | while IFS= read -r job; do
  echo "qsub $job"
  qsub "$job"
done
