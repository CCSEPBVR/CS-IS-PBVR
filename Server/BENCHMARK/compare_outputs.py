#!/usr/bin/env python3
"""Compare PBVR outputs with tolerance for floating-point text data."""
from __future__ import annotations

import argparse
import csv
import hashlib
import math
import re
from pathlib import Path

FLOAT_RE = re.compile(r"[-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?")


def file_hash(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def numeric_tokens(path: Path) -> list[float] | None:
    try:
        text = path.read_text(errors="strict")
    except Exception:
        return None
    vals = []
    for m in FLOAT_RE.finditer(text):
        try:
            vals.append(float(m.group(0)))
        except ValueError:
            pass
    return vals


def compare_file(ref: Path, cand: Path, rtol: float, atol: float) -> tuple[bool, str]:
    if not cand.exists():
        return False, "candidate missing"
    if ref.stat().st_size == cand.stat().st_size and file_hash(ref) == file_hash(cand):
        return True, "exact"
    a = numeric_tokens(ref)
    b = numeric_tokens(cand)
    if a is None or b is None or not a or not b:
        return False, "binary/text differs"
    if len(a) != len(b):
        return False, f"numeric token count differs: {len(a)} vs {len(b)}"
    max_abs = 0.0
    max_rel = 0.0
    for x, y in zip(a, b):
        diff = abs(x - y)
        denom = max(abs(x), abs(y), atol)
        max_abs = max(max_abs, diff)
        max_rel = max(max_rel, diff / denom)
        if not math.isfinite(diff) or diff > atol + rtol * max(abs(x), abs(y)):
            return False, f"numeric mismatch max_abs>tol at values {x} vs {y}"
    return True, f"numeric within tolerance max_abs={max_abs:.6g} max_rel={max_rel:.6g}"


def collect_files(root: Path) -> list[Path]:
    if root.is_file():
        return [root]
    return [p for p in root.rglob("*") if p.is_file()]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--reference", required=True)
    parser.add_argument("--candidate", required=True)
    parser.add_argument("--output", default="benchmark_analysis/comparison_summary.csv")
    parser.add_argument("--rtol", type=float, default=1e-5)
    parser.add_argument("--atol", type=float, default=1e-7)
    parser.add_argument("--allow-missing", action="store_true")
    args = parser.parse_args()

    ref = Path(args.reference)
    cand = Path(args.candidate)
    rows = []
    ok_all = True
    for ref_file in collect_files(ref):
        rel = ref_file.name if ref.is_file() else str(ref_file.relative_to(ref))
        cand_file = cand if cand.is_file() else cand / rel
        ok, detail = compare_file(ref_file, cand_file, args.rtol, args.atol)
        if not ok and args.allow_missing and detail == "candidate missing":
            ok = True
        ok_all = ok_all and ok
        rows.append({"path": rel, "ok": int(ok), "detail": detail})

    out = Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    with out.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["path", "ok", "detail"])
        writer.writeheader()
        writer.writerows(rows)
    print(f"wrote {out}; status={'OK' if ok_all else 'NG'}")
    return 0 if ok_all else 1


if __name__ == "__main__":
    raise SystemExit(main())
