#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$SCRIPT_DIR/config.sh"
cd "$BENCHMARK_DIR"

python3 -m py_compile \
  generate_benchmark_cases.py \
  generate_pbs_jobs.py \
  parse_timing.py \
  summarize_results.py \
  compare_outputs.py \
  make_report.py

python3 generate_benchmark_cases.py --output "$CASES_CSV"
python3 generate_pbs_jobs.py --cases "$CASES_CSV" --job-root "$JOB_ROOT"

SAMPLE_TIMER="$REPO_ROOT/Example/C/s86_mpi_omp/ens_Hydrogen_unstruct/ensemble_timer_summary.csv"
if [ ! -f "$SAMPLE_TIMER" ]; then
  SAMPLE_TIMER="$REPO_ROOT/Example/C/s86_mpi_omp/ens_Hydrogen_unstruct_mpi4/ensemble_timer_summary.csv"
fi
if [ -f "$SAMPLE_TIMER" ]; then
  python3 parse_timing.py --input "$SAMPLE_TIMER" --output "$ANALYSIS_DIR"
  python3 summarize_results.py --cases "$CASES_CSV" --timing "$ANALYSIS_DIR/timing_summary.csv" --output "$ANALYSIS_DIR"
  python3 make_report.py --analysis "$ANALYSIS_DIR" --output "$ANALYSIS_DIR/BENCHMARK_REPORT.md"
else
  echo "[local-tests] sample timer CSV not found; skipped timing parser smoke test: $SAMPLE_TIMER"
fi

EXEC_PATH=$EXECUTABLE
case "$EXEC_PATH" in
  /*) ;;
  *) EXEC_PATH="$REPO_ROOT/$EXEC_PATH" ;;
esac
if [ "$RUN_EXECUTABLE_SMOKE" = "1" ]; then
  if command -v timeout >/dev/null 2>&1 && command -v mpirun >/dev/null 2>&1 && [ -x "$EXEC_PATH" ]; then
    OMP_NUM_THREADS=1 timeout 15 mpirun -np 1 "$EXEC_PATH" $INPUT_ARGS || true
  else
    echo "[local-tests] executable smoke test unavailable; set RUN_EXECUTABLE_SMOKE=0 or run on PBS."
  fi
fi

echo "[local-tests] completed. No PBS job was submitted."
