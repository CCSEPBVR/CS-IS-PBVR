#!/usr/bin/env python3
"""Summarize PBVR benchmark timings and flag bottleneck candidates."""
from __future__ import annotations

import argparse
import csv
import math
from collections import defaultdict
from pathlib import Path


def read_csv(path: str) -> list[dict[str, str]]:
    p = Path(path)
    if not p.exists():
        return []
    with p.open(newline="") as f:
        return list(csv.DictReader(f))


def write_csv(path: Path, rows: list[dict], fields: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)


def f(row: dict, key: str) -> float:
    try:
        return float(row.get(key, 0) or 0)
    except ValueError:
        return 0.0


def choose_total(summary_by_case: dict[str, list[dict]]) -> dict[str, float]:
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


def imbalance_label(max_over_avg: float) -> str:
    if max_over_avg >= 2.0:
        return "strong imbalance or wait"
    if max_over_avg >= 1.2:
        return "needs review"
    if max_over_avg >= 1.05:
        return "mild imbalance"
    return "good"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cases", default="cases.csv")
    parser.add_argument("--timing", default="benchmark_analysis/timing_summary.csv")
    parser.add_argument("--output", default="benchmark_analysis")
    args = parser.parse_args()

    cases = read_csv(args.cases)
    timings = read_csv(args.timing)
    out = Path(args.output)

    case_meta = {row["case"]: row for row in cases}
    by_case: dict[str, list[dict]] = defaultdict(list)
    by_case_section: dict[tuple[str, str], dict] = {}
    for row in timings:
        by_case[row["case"]].append(row)
        by_case_section[(row["case"], row["section"])] = row

    totals = choose_total(by_case)
    baseline_case = "correctness_serial" if "correctness_serial" in totals else ("strong_1x1" if "strong_1x1" in totals else next(iter(totals), ""))
    baseline_total = totals.get(baseline_case, 0.0)

    scaling_rows = []
    for case, total in sorted(totals.items()):
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
            symptoms.append("large contribution")
        if serial_time > 0 and speedup < 1.0:
            symptoms.append("slower than serial section")
        if moa >= 1.2:
            symptoms.append(imbalance_label(moa))
        if any(key in section.lower() for key in ["mpi", "barrier", "shift", "gather", "output", "history", "io", "merge"]):
            symptoms.append("communication/synchronization/io candidate")
        if symptoms:
            priority = 1 if contribution >= 0.25 or moa >= 2.0 else (2 if contribution >= 0.10 or moa >= 1.2 else 3)
            candidate_rows.append({
                "priority": priority,
                "section": section,
                "symptom": "; ".join(sorted(set(symptoms))),
                "evidence": f"case={case}, opt_max={opt_max:.6g}, contribution={contribution:.3f}, max/avg={moa:.3f}",
                "hypothesis": "load imbalance, collective wait, rank-0 concentration, or expensive local kernel",
                "next_check": "inspect rank/thread raw timing and split compute/communication if the section remains dominant",
            })

    candidate_rows.sort(key=lambda r: (int(r["priority"]), r["section"]))
    write_csv(out / "scaling_summary.csv", scaling_rows, ["case", "nodes", "mpi", "omp", "total_cores", "total_time", "speedup", "efficiency"])
    write_csv(out / "section_summary.csv", section_rows, ["case", "section", "serial_time", "opt_min", "opt_avg", "opt_max", "speedup_vs_serial", "max_over_avg", "contribution"])
    write_csv(out / "bottleneck_candidates.csv", candidate_rows, ["priority", "section", "symptom", "evidence", "hypothesis", "next_check"])
    print(f"wrote summaries under {out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
