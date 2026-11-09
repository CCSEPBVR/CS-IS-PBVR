#!/usr/bin/env zsh
# Run with: zsh test_generate_particles.sh
# Linux example: zsh test_generate_particles.sh PBVR_MACHINE=Makefile_machine_gcc_mpi_omp
# Optional arguments are forwarded to make (e.g. PBVR_MACHINE=...).
# Prerequisite: PBVR libraries and KVS/Install have already been built.
set -euo pipefail

ensemble_dir="${0:A:h}"
repo_dir="$(cd "$ensemble_dir/../../../../.." && pwd)"
export KVS_DIR="${KVS_DIR:-$repo_dir/KVS/Install}"

sample="preflight"
step="check prerequisites"
trap 'printf "ERROR: %s: %s (line %s)\n" "$sample" "$step" "$LINENO" >&2' ZERR

command -v make >/dev/null
command -v mpirun >/dev/null
[[ -f "$KVS_DIR/kvs.conf" ]]

# Set the runtime library search path for the host OS before invoking MPI.
case "$(uname -s)" in
    Darwin)
        # macOS may clear inherited DYLD_* variables at shell startup.
        : "${VTK_LIB_PATH:?Set VTK_LIB_PATH to the VTK library directory}"
        export DYLD_LIBRARY_PATH="${VTK_LIB_PATH}${DYLD_LIBRARY_PATH:+:${DYLD_LIBRARY_PATH}}"
        ;;
    Linux)
        # Without an explicit VTK path, retain the system/module settings.
        if [[ -n "${VTK_LIB_PATH:-}" ]]; then
            export LD_LIBRARY_PATH="${VTK_LIB_PATH}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
        fi
        ;;
    *)
        printf 'ERROR: This script supports macOS and Linux.\n' >&2
        exit 1
        ;;
esac

samples=(
    ens_Hydrogen_struct
    ens_Hydrogen_unstruct
    ens_Hydrogen_unstruct_prism
    ens_Hydrogen_unstruct_pyramid
    ens_Hydrogen_unstruct_quadhexa
    ens_Hydrogen_unstruct_quadtetra
    ens_Hydrogen_unstruct_strong
    ens_Hydrogen_unstruct_tetra
)

# Freeze the reference before MPI can rename/update its default.json.
# A previous run may have moved default.json to default_old.json.
reference="$ensemble_dir/ens_Hydrogen_unstruct/default.json"
if [[ ! -f "$reference" ]]; then
    reference="$ensemble_dir/ens_Hydrogen_unstruct/default_old.json"
fi
settings_snapshot="$(mktemp "${TMPDIR:-/tmp}/pbvr-ensemble-parameters.XXXXXX")"
trap 'rm -f "$settings_snapshot"' EXIT
cp "$reference" "$settings_snapshot"

for sample in "${samples[@]}"; do
    printf '\n=== %s ===\n' "$sample"
    step="rebuild"
    # Load the parent configuration to export MPI flags to the sample make.
    # Only this sample is rebuilt; other ensemble examples are not included.
    make -C "$ensemble_dir" -f - TEST_SAMPLE="$sample" "$@" rebuild_particle_test <<'MAKEFILE'
include Makefile
.PHONY: rebuild_particle_test
rebuild_particle_test:
	$(MAKE) -C "$(TEST_SAMPLE)" clean
	$(MAKE) -C "$(TEST_SAMPLE)" all
MAKEFILE

    cd "$ensemble_dir/$sample"
    step="create particle_out"
    mkdir -p particle_out

    step="prepare parameters and reset"
    # RESET.sh moves default_old.json to default.json. Replenish it each run
    # so every sample starts with exactly the same reference parameters.
    cp "$settings_snapshot" default_old.json
    /bin/sh -e RESET.sh

    export VIS_PARAM_DIR="$PWD"
    export PARTICLE_DIR="$PWD/particle_out"

    step="mpirun -n 2 ./run"
    if [[ "$sample" == ens_Hydrogen_unstruct_strong ]]; then
        # The strong-scaling driver's default is four ensembles. Use two
        # ensembles for two MPI ranks, matching the other samples.
        PBVR_STRONG_ENS=2 mpirun -n 2 ./run
    else
        mpirun -n 2 ./run
    fi
    printf 'Completed: %s\n' "$sample"
done

printf '\nAll eight samples completed. Check particle_out in each directory.\n'
