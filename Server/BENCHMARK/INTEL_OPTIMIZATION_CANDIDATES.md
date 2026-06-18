# INTEL OPTIMIZATION CANDIDATES

## Compiler and Runtime Baseline

The SGI manual and repository machine file indicate Intel oneAPI and HPE MPT. The MPI+OpenMP machine file uses Intel-oriented flags such as:

```text
-qopenmp -xCORE-AVX2 -O3
```

For vectorization investigation on the supercomputer, add a temporary report build such as:

```sh
CXXFLAGS_EXTRA="-qopt-report=5 -qopt-report-phase=vec"
```

or the equivalent supported by the installed oneAPI version. Keep this separate from production timing builds.

## Static Optimization Candidates

| Candidate | Files / sections | Expected effect | Risk | Notes |
| --- | --- | --- | --- | --- |
| Verify SIMD vectorization in scalar/gradient loops | `InSituLib/unstruct/kvs_wrapper.cpp`, `calculate_scalar_and_chain_rule_grad()` | Medium/high if compiler can vectorize | Low for report-only check | Function parser calls and virtual cell methods may inhibit SIMD. |
| Reduce non-contiguous memory access | Cell interpolation and particle buffers | Medium | Medium | Random cells/local coords create scattered access; SoA helps. |
| Avoid repeated geometry work | `uniform_volume_calculation`, cell gradient paths | High | Medium | Cache if mesh is fixed and multiple samplings reuse cells. |
| Tune OpenMP schedule | Uniform loop and histogram loops | Medium | Low/medium | `dynamic` reduces imbalance but may add overhead. Compare static/guided on same input. |
| Reduce critical merge cost | Particle buffer merge sections | Medium | Medium | Prefix-sum style merge can avoid critical but needs careful memory planning. |
| Improve chain-rule derivative path | Fine timers under `*_chain_rule_*` | Medium/high | Medium | Analytic/AD derivative changes implementation complexity; validate numerics. |
| Use active variables after validation | Chain-rule q/grad setup | Medium/high when many variables unused | Medium | Can change behavior if active variable detection is incomplete. |
| Check `pow(x,2)` and math intrinsics | Function parser / TF eval | Low/medium | Medium | Only safe with exact semantic checks; avoid changing user TF behavior silently. |
| NUMA-aware placement | PBS `ompthreads`, `omplace`, first touch | Medium | Low | Use `omplace -nt $OMP_NUM_THREADS`; ensure allocation occurs on worker threads where useful. |
| Separate I/O from compute | output timers | Medium | Low | Report I/O-inclusive and exclusive totals. |
| Compositing/gather decentralization | particle `MPI_Gatherv` paths | High for large particle counts | High | Algorithmic change; not a first low-risk edit. |

## SIMD Effect Verification

Recommended checks on the supercomputer:

1. Build normal timing binary.
2. Build vectorization-report binary with Intel opt reports.
3. Run a small fixed case with identical input.
4. Compare `uniform_calc_scalar_grad`, `uniform_volume_calculation`, and `uniform_particle_sampling_loop`.
5. Inspect reports for "LOOP WAS VECTORIZED" vs dependency/call/aliasing blockers.
6. Avoid changing SIMD width as the first test; compare compiler reports and hardware counters first. SIMD-width forcing can confound placement, codegen, and numerical behavior.
