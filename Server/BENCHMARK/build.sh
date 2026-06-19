#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$SCRIPT_DIR/config.sh"
cd "$REPO_ROOT"

if [ -f /etc/profile.d/modules.sh ]; then
  . /etc/profile.d/modules.sh
  module purge
  [ -n "$MODULE_CUDA" ] && module load "$MODULE_CUDA"
  [ -n "$MODULE_GNU" ] && module load "$MODULE_GNU"
  [ -n "$MODULE_INTEL" ] && module load "$MODULE_INTEL"
  [ -n "$MODULE_MPI" ] && module load "$MODULE_MPI"
else
  echo "[build] /etc/profile.d/modules.sh not found; assuming compiler/MPI are already in PATH."
fi

echo "[build] repo root: $REPO_ROOT"
echo "[build] benchmark dir: $BENCHMARK_DIR"
echo "[build] PBVR_MACHINE=$PBVR_MACHINE_MPI_OMP"
make PBVR_MACHINE="$PBVR_MACHINE_MPI_OMP" -j "$MAKE_JOBS"

build_example()
{
  EXEC_PATH=$1
  case "$EXEC_PATH" in
    /*) ;;
    *) EXEC_PATH="$REPO_ROOT/$EXEC_PATH" ;;
  esac
  EXEC_DIR=$(dirname "$EXEC_PATH")
  if [ -f "$EXEC_DIR/Makefile" ]; then
    echo "[build] Building example in $EXEC_DIR"
    make -C "$EXEC_DIR" PBVR_MACHINE="$PBVR_MACHINE_MPI_OMP" -j "$MAKE_JOBS"
  else
    echo "[build] Example Makefile not found at $EXEC_DIR; skipping example build."
  fi
}

BUILT_EXAMPLES=""
build_example_once()
{
  case " $BUILT_EXAMPLES " in
    *" $1 "*) return ;;
  esac
  BUILT_EXAMPLES="$BUILT_EXAMPLES $1"
  build_example "$1"
}

build_example_once "$EXECUTABLE"
for strong_executable in $STRONG_EXECUTABLES; do
  build_example_once "$strong_executable"
done
build_example_once "$WEAK_EXECUTABLE"

if [ "$BUILD_SERIAL" = "1" ]; then
  echo "[build] Optional serial build: PBVR_MACHINE=$PBVR_MACHINE_SERIAL"
  make PBVR_MACHINE="$PBVR_MACHINE_SERIAL" -j "$MAKE_JOBS"
fi
