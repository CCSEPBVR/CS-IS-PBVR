#!/usr/bin/env python3
"""Create a Markdown benchmark report from generated CSV summaries."""

import argparse
import csv
from pathlib import Path


def read_rows(path):
    if not path.exists():
        return []
    with path.open(newline="") as f:
        return list(csv.DictReader(f))


def md_table(rows, fields, limit=40):
    if not rows:
        return "No data.\n"
    lines = ["| " + " | ".join(fields) + " |", "| " + " | ".join(["---"] * len(fields)) + " |"]
    for row in rows[:limit]:
        lines.append("| " + " | ".join(str(row.get(f, "")) for f in fields) + " |")
    if len(rows) > limit:
        lines.append("\nShowing first %d of %d rows.\n" % (limit, len(rows)))
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--analysis", default="benchmark_analysis")
    parser.add_argument("--output", default="benchmark_analysis/BENCHMARK_REPORT.md")
    args = parser.parse_args()

    root = Path(args.analysis)
    timing = read_rows(root / "timing_summary.csv")
    scaling = read_rows(root / "scaling_summary.csv")
    sections = read_rows(root / "section_summary.csv")
    candidates = read_rows(root / "bottleneck_candidates.csv")

    text = []
    text.append("# PBVR Benchmark Report\n")
    text.append("This report is generated from existing PBVR timer CSV/log outputs. Parallel comparisons use the MPI max time, not only the average.\n")
    text.append("## Scaling Summary\n")
    text.append(md_table(scaling, ["case", "nodes", "mpi", "omp", "total_cores", "total_time", "speedup", "efficiency"]))
    text.append("## Bottleneck Candidates\n")
    text.append(md_table(candidates, ["priority", "section", "symptom", "evidence", "hypothesis", "next_check"]))
    text.append("## Section Summary\n")
    text.append(md_table(sections, ["case", "section", "serial_time", "opt_avg", "opt_max", "speedup_vs_serial", "max_over_avg", "contribution"]))
    text.append("## Timing Summary\n")
    text.append(md_table(timing, ["case", "section", "min", "avg", "max", "max_over_avg", "thread_avg_sec", "thread_max_sec"]))

    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text("\n".join(text), encoding="utf-8")
    print("wrote %s" % out)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
