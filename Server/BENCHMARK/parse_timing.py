#!/usr/bin/env python3
"""Parse PBVR timer logs and ensemble_timer_summary.csv files."""
from __future__ import annotations

import argparse
import csv
import math
import os
import re
import statistics
from collections import defaultdict
from pathlib import Path

TIMER_RE = re.compile(
    r"(?:rank\s*[=:]\s*(?P<rank>\d+).*?)?"
    r"(?:section|timer|name)\s*[=:]\s*(?P<section>[A-Za-z0-9_./:-]+).*?"
    r"(?:time|sec|elapsed)\s*[=:]\s*(?P<time>[0-9.eE+-]+)"
)


def case_from_path(path: Path) -> str:
    if path.name in {"ensemble_timer_summary.csv", "timing.csv"} and path.parent.name:
        return path.parent.name
    return path.stem


def iter_inputs(inputs: list[str]) -> list[Path]:
    paths: list[Path] = []
    for item in inputs:
        p = Path(item)
        if p.is_dir():
            paths.extend(p.rglob("ensemble_timer_summary.csv"))
            paths.extend(p.rglob("*.log"))
            paths.extend(p.rglob("*.out"))
        elif p.exists():
            paths.append(p)
    return sorted(set(paths))


def parse_ensemble_summary(path: Path):
    case = case_from_path(path)
    summary_rows = []
    raw_rows = []
    with path.open(newline="", errors="replace") as f:
        reader = csv.DictReader((line for line in f if line.strip()))
        for row in reader:
            if row.get("section") == "section" or not row.get("section"):
                continue
            section = row["section"]
            try:
                mn = float(row.get("mpi_min_sec") or 0.0)
                avg = float(row.get("mpi_avg_sec") or 0.0)
                mx = float(row.get("mpi_max_sec") or 0.0)
            except ValueError:
                continue
            max_over_avg = mx / avg if avg > 0 else 0.0
            summary_rows.append({
                "case": case,
                "section": section,
                "min": mn,
                "avg": avg,
                "max": mx,
                "std": 0.0,
                "max_over_avg": max_over_avg,
                "source": str(path),
                "parent_section": row.get("parent_section", ""),
                "level": row.get("level", ""),
                "thread_avg_sec": row.get("thread_avg_sec", ""),
                "thread_max_sec": row.get("thread_max_sec", ""),
                "thread_min_sec": row.get("thread_min_sec", ""),
            })
            # Rank -1 represents the already-reduced MPI max from ensemble_timer_summary.csv.
            raw_rows.append({"case": case, "rank": -1, "section": section, "time_sec": mx, "source": str(path)})
    return raw_rows, summary_rows


def parse_text_log(path: Path):
    case = case_from_path(path)
    rows = []
    with path.open(errors="replace") as f:
        for line in f:
            m = TIMER_RE.search(line)
            if not m:
                continue
            rank = int(m.group("rank") or -1)
            rows.append({
                "case": case,
                "rank": rank,
                "section": m.group("section"),
                "time_sec": float(m.group("time")),
                "source": str(path),
            })
    return rows


def summarize_raw(raw_rows):
    groups: dict[tuple[str, str], list[float]] = defaultdict(list)
    for row in raw_rows:
        groups[(row["case"], row["section"])].append(float(row["time_sec"]))
    rows = []
    for (case, section), vals in sorted(groups.items()):
        avg = sum(vals) / len(vals)
        mn = min(vals)
        mx = max(vals)
        std = statistics.pstdev(vals) if len(vals) > 1 else 0.0
        rows.append({
            "case": case,
            "section": section,
            "min": mn,
            "avg": avg,
            "max": mx,
            "std": std,
            "max_over_avg": mx / avg if avg > 0 else 0.0,
            "source": "raw_log",
            "parent_section": "",
            "level": "",
            "thread_avg_sec": "",
            "thread_max_sec": "",
            "thread_min_sec": "",
        })
    return rows


def write_csv(path: Path, rows: list[dict], fieldnames: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", action="append", default=[], help="file or directory; may be repeated")
    parser.add_argument("--output", default=os.environ.get("ANALYSIS_DIR", "benchmark_analysis"))
    args = parser.parse_args()

    inputs = args.input or [os.environ.get("OUTPUT_ROOT", "benchmark_results"), "."]
    paths = iter_inputs(inputs)
    raw_rows = []
    summary_rows = []
    for path in paths:
        if path.name == "ensemble_timer_summary.csv" or path.name.endswith("_ensemble_timer_summary.csv"):
            raw, summary = parse_ensemble_summary(path)
            raw_rows.extend(raw)
            summary_rows.extend(summary)
        else:
            raw_rows.extend(parse_text_log(path))

    if not summary_rows:
        summary_rows = summarize_raw(raw_rows)

    out = Path(args.output)
    write_csv(out / "timing_raw.csv", raw_rows, ["case", "rank", "section", "time_sec", "source"])
    write_csv(out / "timing_summary.csv", summary_rows, [
        "case", "section", "min", "avg", "max", "std", "max_over_avg", "source",
        "parent_section", "level", "thread_avg_sec", "thread_max_sec", "thread_min_sec"
    ])
    print(f"parsed {len(paths)} files, wrote {out/'timing_raw.csv'} and {out/'timing_summary.csv'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
