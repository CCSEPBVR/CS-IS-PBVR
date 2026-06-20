#!/bin/sh
# Compare correctness cases by physical invariants (see compare_outputs.py).
# Baseline = correctness_mpi_2x1; candidates are other MPI/OpenMP decompositions.
# Only labeled invariants in the run log are compared; thread-time noise and
# pure-timing artifacts are ignored. Writes a combined comparison_summary.csv.
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$SCRIPT_DIR/config.sh"
cd "$BENCHMARK_DIR"

REF_CASE=${CORRECTNESS_BASELINE:-correctness_mpi_2x1}
CAND_CASES=${CORRECTNESS_CANDIDATES:-"correctness_mpi_2x2 correctness_hybrid_4x4"}
OUT="$ANALYSIS_DIR/comparison_summary.csv"

REF_LOG="$OUTPUT_ROOT/$REF_CASE/$REF_CASE.log"
if [ ! -f "$REF_LOG" ]; then
  echo "[compare-correctness] baseline log not found: $REF_LOG" >&2
  exit 2
fi

mkdir -p "$ANALYSIS_DIR"
echo "reference,candidate,ok,detail" > "$OUT"
status=0
for cand in $CAND_CASES; do
  CAND_LOG="$OUTPUT_ROOT/$cand/$cand.log"
  tmp=$(mktemp)
  if python3 "$SCRIPT_DIR/compare_outputs.py" --reference "$REF_LOG" --candidate "$CAND_LOG" --output "$tmp" >/dev/null 2>&1; then
    ok=1
  else
    ok=0
    status=1
  fi
  detail=$(tail -n +2 "$tmp" | head -1 | cut -d, -f3-)
  [ -z "$detail" ] && detail='"no result"'
  echo "$REF_CASE,$cand,$ok,$detail" >> "$OUT"
  rm -f "$tmp"
done

echo "wrote $OUT"
cat "$OUT"
if [ "$status" = 0 ]; then echo "[compare-correctness] ALL OK"; else echo "[compare-correctness] NG"; fi
exit $status
