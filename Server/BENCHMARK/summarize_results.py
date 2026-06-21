#!/usr/bin/env python3
"""Summarize PBVR benchmark timings and flag bottleneck candidates."""

import argparse
import csv
from collections import defaultdict
from pathlib import Path

# Level-0 wall-clock section that bounds every other section.
TOTAL_SECTION = "ensemble_generate_particles_total"

# Time-decomposition categories for scaling_summary.csv.
#
# Sections are matched by name regardless of tree level. To avoid parent/child
# double counting the L1 wrapper ``mpi_shift_exchange`` is intentionally NOT
# summed; instead its L2 children are split: the transfer parts go to ``comm``
# and ``omp_shift_interpolation`` goes to ``comp``. The L0 total is likewise
# never summed here.
CATEGORY_SECTIONS = {
    "comm": (
        "mpi_shift_size_exchange",
        "mpi_shift_alloc_recv_buffer",
        "mpi_shift_payload_all",
    ),
    "comp": (
        "omp_uniform_sampling",
        "omp_shift_interpolation",
        "omp_rejection",
        "create_cells",
        "sampling_prepare",
        "stat_average_variance",
        "stat_histogram",
    ),
    "sync": (
        "async_io_wait",
        "final_barrier_state",
    ),
    "io": (
        "read_parameter_file",
        "set_parameter_path",
        "init_transfer_functions",
        "output_particles_ave",
        "output_particles_var",
        "output_particles_cov",
        "output_coord_minmax",
        "output_history",
        "write_tf_file",
        "cleanup_tfs",
        "cleanup_cells",
    ),
}

# Per-phase speedup baseline. efficiency is relativized to each phase's own
# baseline so the ideal value is 1.0 (not 0.5 as with a global 2-core baseline).
PHASE_BASELINE = {
    "correctness": "correctness_mpi_2x1",
    "strong_scaling": "strong_4x1",
    "weak_scaling": "weak_10",
    "mpi_omp_sweep": "sweep_40x1",
}
# Fixed-work phases: efficiency = speedup * cores_base / cores.
EFFICIENCY_PHASES = ("correctness", "strong_scaling")
# Weak scaling grows work with cores: weak_efficiency = T_base / Tp (no /cores).
WEAK_EFFICIENCY_PHASES = ("weak_scaling",)


def read_csv(path):
    p = Path(path)
    if not p.exists():
        return []
    with p.open(newline="", encoding="utf-8-sig") as f:
        return list(csv.DictReader(f))


def write_csv(path, rows, fields):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8-sig") as f:
        writer = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)


def f(row, key):
    try:
        return float(row.get(key, 0) or 0)
    except (ValueError, AttributeError):
        return 0.0


def choose_total(summary_by_case):
    totals = {}
    preferred = ["total", "ensemble_generate_particles_total", "ensemble_total"]
    for case, rows in summary_by_case.items():
        chosen = None
        for name in preferred:
            for row in rows:
                if row["section"] == name:
                    chosen = f(row, "max")
                    break
            if chosen is not None:
                break
        if chosen is None and rows:
            chosen = max(f(row, "max") for row in rows)
        totals[case] = chosen or 0.0
    return totals


def cores_of(meta):
    mpi = int(meta.get("mpiprocs", 1) or 1)
    omp = int(meta.get("ompthreads", 1) or 1)
    nodes = int(meta.get("nodes", 1) or 1)
    return mpi * omp * nodes


def aggregate_sections(rows):
    """Collapse repeated (section) rows for one case to a single max per section.

    timing_summary.csv holds one row per (section, timestep); the per-section
    max across timesteps is used as the representative value.
    """
    agg = defaultdict(float)
    for row in rows:
        sec = row["section"]
        agg[sec] = max(agg[sec], f(row, "max"))
    return agg


def total_imbalance(rows):
    """Worst-case max/avg (over ranks) of the total section across timesteps."""
    moa = 0.0
    for row in rows:
        if row["section"] == TOTAL_SECTION:
            moa = max(moa, f(row, "max_over_avg"))
    return moa


def decompose_time(rows, total):
    """Return per-category times plus classified/overlap/comm_frac/imbalance."""
    sec_max = aggregate_sections(rows)
    cat_time = {cat: sum(sec_max.get(name, 0.0) for name in names)
                for cat, names in CATEGORY_SECTIONS.items()}
    classified = sum(cat_time.values())
    return {
        "comp_time": cat_time["comp"],
        "comm_time": cat_time["comm"],
        "sync_time": cat_time["sync"],
        "io_time": cat_time["io"],
        "classified_time": classified,
        "overlap_time": max(0.0, classified - total),
        "sum_frac": classified / total if total > 0 else 0.0,
        "comm_frac": cat_time["comm"] / total if total > 0 else 0.0,
        "imbalance": total_imbalance(rows),
    }


def imbalance_label(max_over_avg):
    if max_over_avg >= 2.0:
        return "強い負荷不均衡または待ち"
    if max_over_avg >= 1.2:
        return "要確認"
    if max_over_avg >= 1.05:
        return "軽度の偏り"
    return "良好"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cases", default="cases.csv")
    parser.add_argument("--timing", default="benchmark_analysis/timing_summary.csv")
    parser.add_argument("--output", default="benchmark_analysis")
    args = parser.parse_args()

    cases = read_csv(args.cases)
    timings = read_csv(args.timing)
    out = Path(args.output)

    case_meta = {row["case"]: row for row in cases}
    by_case = defaultdict(list)
    for row in timings:
        by_case[row["case"]].append(row)

    totals = choose_total(by_case)
    if "correctness_mpi_2x1" in totals:
        baseline_case = "correctness_mpi_2x1"
    elif "strong_2x1" in totals:
        baseline_case = "strong_2x1"
    elif "correctness_serial" in totals:
        baseline_case = "correctness_serial"
    elif "strong_1x1" in totals:
        baseline_case = "strong_1x1"
    else:
        baseline_case = next(iter(totals), "")
    # Per-phase speedup baselines: phase -> (T_base, cores_base).
    phase_base = {}
    for ph, bcase in PHASE_BASELINE.items():
        if bcase in totals and totals[bcase] > 0:
            phase_base[ph] = (totals[bcase], cores_of(case_meta.get(bcase, {})))

    scaling_rows = []
    for case in sorted(totals):
        total = totals[case]
        meta = case_meta.get(case, {})
        phase = meta.get("phase", "")
        nodes = int(meta.get("nodes", 1) or 1)
        mpi = int(meta.get("mpiprocs", 1) or 1)
        omp = int(meta.get("ompthreads", 1) or 1)
        cores = mpi * omp * nodes
        base = phase_base.get(phase)
        cores_base = base[1] if base else ""
        speedup = base[0] / total if base and total > 0 else ""
        if phase in EFFICIENCY_PHASES and base and total > 0 and cores > 0:
            efficiency = speedup * base[1] / cores
        else:
            efficiency = ""  # weak は weak_efficiency、sweep は時間比較のみで N/A
        weak_efficiency = speedup if (phase in WEAK_EFFICIENCY_PHASES and base and total > 0) else ""
        row = {
            "case": case,
            "phase": phase,
            "nodes": nodes,
            "mpi": mpi,
            "omp": omp,
            "total_cores": cores,
            "cores_base": cores_base,
            "total_time": total,
            "speedup": speedup,
            "efficiency": efficiency,
            "weak_efficiency": weak_efficiency,
        }
        row.update(decompose_time(by_case.get(case, []), total))
        scaling_rows.append(row)

    serial_sections = {row["section"]: row for row in by_case.get(baseline_case, [])}
    section_rows = []
    candidate_rows = []
    for row in timings:
        case = row["case"]
        section = row["section"]
        opt_max = f(row, "max")
        total = totals.get(case, 0.0)
        serial_time = f(serial_sections.get(section, {}), "max")
        contribution = opt_max / total if total > 0 else 0.0
        speedup = serial_time / opt_max if opt_max > 0 else 0.0
        moa = f(row, "max_over_avg")
        section_rows.append({
            "case": case,
            "section": section,
            "serial_time": serial_time,
            "opt_min": f(row, "min"),
            "opt_avg": f(row, "avg"),
            "opt_max": opt_max,
            "speedup_vs_serial": speedup,
            "max_over_avg": moa,
            "contribution": contribution,
        })
        symptoms = []
        if contribution >= 0.15:
            symptoms.append("全体時間への寄与が大きい")
        if serial_time > 0 and speedup < 1.0:
            symptoms.append("基準ケースより遅い")
        if moa >= 1.2:
            symptoms.append(imbalance_label(moa))
        if any(key in section.lower() for key in ["mpi", "barrier", "shift", "gather", "output", "history", "io", "merge"]):
            symptoms.append("通信/同期/I/O候補")
        if symptoms:
            priority = 1 if contribution >= 0.25 or moa >= 2.0 else (2 if contribution >= 0.10 or moa >= 1.2 else 3)
            candidate_rows.append({
                "priority": priority,
                "section": section,
                "symptom": "; ".join(sorted(set(symptoms))),
                "evidence": "case=%s, opt_max=%.6g, contribution=%.3f, max/avg=%.3f" % (case, opt_max, contribution, moa),
                "hypothesis": "負荷不均衡、集団通信待ち、rank 0集中、または局所kernelの高コスト",
                "next_check": "rank/thread別raw timingを確認し、支配的なら計算/通信をさらに分割する",
            })

    candidate_rows.sort(key=lambda r: (int(r["priority"]), r["section"]))
    write_csv(out / "scaling_summary.csv", scaling_rows, [
        "case", "phase", "nodes", "mpi", "omp", "total_cores", "cores_base",
        "total_time", "speedup", "efficiency", "weak_efficiency",
        "comp_time", "comm_time", "sync_time", "io_time",
        "classified_time", "overlap_time", "sum_frac", "comm_frac", "imbalance",
    ])
    write_csv(out / "section_summary.csv", section_rows, ["case", "section", "serial_time", "opt_min", "opt_avg", "opt_max", "speedup_vs_serial", "max_over_avg", "contribution"])
    write_csv(out / "bottleneck_candidates.csv", candidate_rows, ["priority", "section", "symptom", "evidence", "hypothesis", "next_check"])
    print("wrote summaries under %s" % out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
