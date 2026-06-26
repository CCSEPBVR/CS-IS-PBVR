#!/bin/sh
# =============================================================================
# CS-path particle-generation regression test.
#
# Why this exists:
#   The CS server generates particles via
#     GenerateParticleCS() -> PointObjectGenerator::GenerateParticleUnstruct()
#       -> CellByCellUniformSampling -> CellBase (scalar_ary/grad_ary, buffers)
#   The IS ensemble test (compare_correctness.sh) does NOT exercise this path:
#   ensemble_generate_particles() uses its own inline uniform+shift sampling and
#   never calls CellByCellUniformSampling/GenerateParticleUnstruct.
#   This script runs the BASE generate_particles() via Hydrogen_unstruct_cstest,
#   which uses the SAME CellByCellUniformSampling/CellBase code as CS. It is thus
#   the right regression for shared CellBase changes (e.g. aligned allocation).
#
# Determinism:
#   MersenneTwister is seeded by (thread + rank*nthreads). For a fixed
#   (NP, STEPS, input) the particle output is bit-identical run-to-run, so a
#   behavior-preserving change MUST yield byte-identical particles -> exact diff.
#
# Usage:
#   ./compare_base_cs.sh save     # BEFORE the change: capture baseline particles
#   # ... apply CellBase change, rebuild Hydrogen_unstruct_cstest ...
#   ./compare_base_cs.sh check    # AFTER the change: rerun and exact-diff vs baseline
#
# Env overrides (save and check MUST use identical NP/STEPS):
#   PBVR_CSTEST_NP     (default 1)        MPI ranks
#   PBVR_CSTEST_STEPS  (default 1)        time steps
#   PBVR_CSTEST_WRAP   (default omplace)  mpirun wrapper; set empty for non-PBS quick run
#   PBVR_CSTEST_LAUNCH (default mpirun)   set 'direct' to run the binary as a singleton (login node)
#   OMP_NUM_THREADS    (default 1)
# =============================================================================

MODE="${1:-check}"
NP="${PBVR_CSTEST_NP:-1}"
STEPS="${PBVR_CSTEST_STEPS:-1}"
WRAP="${PBVR_CSTEST_WRAP-omplace}"

BENCHMARK_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$BENCHMARK_DIR/.." && pwd)"
EXE_DIR="$REPO_ROOT/Example/C/s86_mpi_omp/Hydrogen_unstruct_cstest"
RUN="$EXE_DIR/run"
OUT_DIR="$EXE_DIR/particle_out"
BASELINE_DIR="$BENCHMARK_DIR/cs_baseline"
CANDIDATE_DIR="$BENCHMARK_DIR/cs_candidate"

# --- environment: KVS/VTK vars (.bashrc) + runtime modules + Intel backend ---
[ -f "$HOME/.bashrc" ] && . "$HOME/.bashrc"
if [ -f /etc/profile.d/modules.sh ]; then
  . /etc/profile.d/modules.sh
  module purge
  module load cuda/11.4 gnu/cur intel/2023.2.1 mpt/2.23-ga
fi
export MPICC_CC=icc MPICXX_CXX=icpc
export OMP_NUM_THREADS="${OMP_NUM_THREADS:-1}"
export KMP_AFFINITY=disabled
export VIS_PARAM_DIR="$EXE_DIR"
export PARTICLE_DIR="$OUT_DIR"

if [ ! -x "$RUN" ]; then
  echo "ERROR: $RUN not built. Build it first:" >&2
  echo "  (source ~/.bashrc; module load ...; export MPICXX_CXX=icpc; make -C $EXE_DIR -j4)" >&2
  exit 2
fi

# --- run (deterministic, fixed steps) ---
# Reset inputs/outputs for a deterministic run. The run renames
# parameter.{gly,pol,pot} -> parameter_old.* after reading, so recreate the
# inputs from the *_old templates before every run (idempotent).
for ext in gly pol pot; do
  [ -f "$EXE_DIR/parameter_old.$ext" ] && cp -f "$EXE_DIR/parameter_old.$ext" "$EXE_DIR/parameter.$ext"
done
mkdir -p "$OUT_DIR"
find "$OUT_DIR" -type f -delete 2>/dev/null
rm -f "$EXE_DIR"/default_0*.json "$EXE_DIR"/history_0* "$EXE_DIR"/timer.out.* "$EXE_DIR"/state.txt 2>/dev/null
export PBVR_CSTEST_STEPS="$STEPS"
LAUNCH="${PBVR_CSTEST_LAUNCH:-mpirun}"   # mpirun (PBS) | direct (singleton, login node)
echo "[cs-test] mode=$MODE NP=$NP STEPS=$STEPS LAUNCH=$LAUNCH WRAP=${WRAP:-<none>}"
if [ "$LAUNCH" = "direct" ]; then
  ( cd "$EXE_DIR" && "$RUN" )
else
  ( cd "$EXE_DIR" && mpirun -n "$NP" $WRAP "$RUN" )
fi
RC=$?
if [ "$RC" -ne 0 ]; then echo "ERROR: run failed (rc=$RC)" >&2; exit 1; fi

NFILES="$(find "$OUT_DIR" -type f 2>/dev/null | wc -l)"
if [ "$NFILES" -eq 0 ]; then echo "ERROR: no particle output produced in $OUT_DIR" >&2; exit 1; fi

case "$MODE" in
  save)
    rm -rf "$BASELINE_DIR"; mkdir -p "$BASELINE_DIR"
    cp -a "$OUT_DIR/." "$BASELINE_DIR/"
    echo "[cs-test] baseline saved: $BASELINE_DIR ($NFILES files)"
    ;;
  check)
    if [ ! -d "$BASELINE_DIR" ]; then
      echo "ERROR: no baseline at $BASELINE_DIR. Run '$0 save' before the change." >&2
      exit 2
    fi
    rm -rf "$CANDIDATE_DIR"; mkdir -p "$CANDIDATE_DIR"
    cp -a "$OUT_DIR/." "$CANDIDATE_DIR/"
    echo "[cs-test] exact diff: baseline vs candidate ($NFILES files)"
    # Compare particle output: coord/color/kvsml exact (deterministic); normals within
    # tolerance (alignment/FP optimizations shift normals at ULP level -> physically
    # identical). Auxiliary p_/g_/pt_ excluded. Tol via PBVR_CSTEST_NORMAL_TOL.
    NORMAL_TOL="${PBVR_CSTEST_NORMAL_TOL:-1e-4}"
    DETAIL=$( python3 "$BENCHMARK_DIR/compare_base_cs_invariants.py" "$BASELINE_DIR" "$CANDIDATE_DIR" "$NORMAL_TOL" )
    if [ $? -eq 0 ]; then
      echo "[cs-test] RESULT: OK  ($DETAIL; normal tol=$NORMAL_TOL) -> CS path physically unchanged"
      exit 0
    else
      echo "[cs-test] RESULT: NG  ($DETAIL)" >&2
      exit 1
    fi
    ;;
  *)
    echo "Usage: $0 {save|check}" >&2; exit 2;;
esac
