#!/bin/sh
set -eu
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$SCRIPT_DIR/config.sh"
cd "$REPO_ROOT"

if [ -f /etc/profile.d/modules.sh ]; then
  . /etc/profile.d/modules.sh
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

build_example "$EXECUTABLE"
if [ "$WEAK_EXECUTABLE" != "$EXECUTABLE" ]; then
  build_example "$WEAK_EXECUTABLE"
fi

if [ "$BUILD_SERIAL" = "1" ]; then
  echo "[build] Optional serial build: PBVR_MACHINE=$PBVR_MACHINE_SERIAL"
  make PBVR_MACHINE="$PBVR_MACHINE_SERIAL" -j "$MAKE_JOBS"
fi
