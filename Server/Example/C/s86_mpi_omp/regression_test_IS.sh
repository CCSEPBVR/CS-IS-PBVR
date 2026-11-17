#!/bin/bash

BASE="/home/g1/k240001/CS-IS-PBVR/Server/Example/C/s86_mpi_omp"

dirs=(Hydrogen_struct Hydrogen_unstruct Hydrogen_unstruct_prism \
      Hydrogen_unstruct_pyramid Hydrogen_unstruct_quadhexa \
      Hydrogen_unstruct_quadtetra Hydrogen_unstruct_tetra)

for dir in "${dirs[@]}"; do
  echo "=== Processing: $dir ==="

  cd "$BASE/$dir" || { echo "[ERROR] Cannot access directory: $dir"; continue; }

  # Build
  make clean && make || { echo "[ERROR] Build failed: $dir"; continue; }

  # Ensure particle_out directory exists
  mkdir -p particle_out

  # Set environment variables
  export VIS_PARAM_DIR="$BASE/$dir"
  export PARTICLE_DIR="$BASE/$dir/particle_out"

  source RESET.sh

  # Execute
  mpirun -n 4 ./run || { echo "[ERROR] Run failed: $dir"; continue; }

  echo "=== Completed: $dir ==="
done
