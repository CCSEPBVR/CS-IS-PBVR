#!/usr/bin/env python3
"""Summarize PBVR benchmark timings and flag bottleneck candidates."""

import argparse
import csv
from collections import defaultdict
from pathlib import Path


def read_csv(path):
    p = Path(path)
    if not p.exists():
        return []
    with p.open(newline="") as f:
        return list(csv.DictReader(f))


def write_csv(path, rows, fields):
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as f:
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
    baseline_total = totals.get(baseline_case, 0.0)

    scaling_rows = []
    for case in sorted(totals):
        total = totals[case]
        meta = case_meta.get(case, {})
        mpi = int(meta.get("mpiprocs", 1) or 1)
        omp = int(meta.get("ompthreads", 1) or 1)
        nodes = int(meta.get("nodes", 1) or 1)
        cores = mpi * omp * nodes
        speedup = baseline_total / total if total > 0 else 0.0
        scaling_rows.append({
            "case": case,
            "nodes": nodes,
            "mpi": mpi,
            "omp": omp,
            "total_cores": cores,
            "total_time": total,
            "speedup": speedup,
            "efficiency": speedup / cores if cores > 0 else 0.0,
        })

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
    write_csv(out / "scaling_summary.csv", scaling_rows, ["case", "nodes", "mpi", "omp", "total_cores", "total_time", "speedup", "efficiency"])
    write_csv(out / "section_summary.csv", section_rows, ["case", "section", "serial_time", "opt_min", "opt_avg", "opt_max", "speedup_vs_serial", "max_over_avg", "contribution"])
    write_csv(out / "bottleneck_candidates.csv", candidate_rows, ["priority", "section", "symptom", "evidence", "hypothesis", "next_check"])
    print("wrote summaries under %s" % out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
