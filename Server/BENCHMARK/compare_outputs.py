#!/usr/bin/env python3
"""Compare PBVR outputs with tolerance for floating-point text data.

For ``*.log`` files only the labeled *physical invariants* are compared
(value/coord range, cell/node counts, bounding box, volume, density and
sampling parameters). Non-deterministic lines such as ``thread time =...[msec]``
are ignored, so a run is judged correct when the data it produced is identical
regardless of the MPI/OpenMP decomposition.

Pure-timing or non-deterministic artifacts are skipped entirely:
``ensemble_timer_summary.csv``, ``timer.out.*``, ``state.txt`` and anything under
``particle_out/`` (particle binaries vary because sampling is randomized).
"""

import argparse
import csv
import hashlib
import math
import re
from pathlib import Path

FLOAT_RE = re.compile(r"[-+]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][-+]?\d+)?")
LABEL_RE = re.compile(r"\s*(.*?)\s*[=:]\s*(.*)$")

# Labeled physical invariants emitted in the PBVR run log. These are
# deterministic regardless of the MPI/OpenMP decomposition.
LOG_INVARIANT_KEYS = (
    "resolution",
    "nvariables",
    "ncells",
    "nnodes",
    "connection min",
    "connection max",
    "coord min",
    "coord max",
    "value min",
    "value max",
    "total_volume",
    "|-X",
    "|-Y",
    "|-Z",
    "max_opacity",
    "max_density",
    "sampling_step",
    "subpixel_level",
    "particle_limit",
)

# Artifacts whose content is pure timing / non-deterministic and must be skipped.
SKIP_NAMES = ("state.txt",)
SKIP_NAME_RE = re.compile(r"(ensemble_timer_summary\.csv|timer\.out\..*)$")
SKIP_DIR_PARTS = ("particle_out",)


def file_hash(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def numeric_tokens(path):
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


def log_invariants(path):
    """Return {label: [floats]} for labeled physical-invariant lines in a log."""
    try:
        text = path.read_text(errors="strict")
    except Exception:
        return None
    inv = {}
    for line in text.splitlines():
        m = LABEL_RE.match(line)
        if not m:
            continue
        label = m.group(1).strip()
        if label not in LOG_INVARIANT_KEYS or label in inv:
            continue
        vals = [float(x.group(0)) for x in FLOAT_RE.finditer(m.group(2))]
        if vals:
            inv[label] = vals
    return inv


def compare_log(ref, cand, rtol, atol):
    a = log_invariants(ref)
    b = log_invariants(cand)
    if not a:
        return False, "no invariants parsed from reference log"
    if b is None:
        return False, "candidate log unreadable"
    missing = [k for k in a if k not in b]
    if missing:
        return False, "missing invariants in candidate: %s" % ",".join(missing)
    max_rel = 0.0
    for key, av in a.items():
        bv = b[key]
        if len(av) != len(bv):
            return False, "invariant '%s' arity differs: %d vs %d" % (key, len(av), len(bv))
        for x, y in zip(av, bv):
            diff = abs(x - y)
            denom = max(abs(x), abs(y), atol)
            max_rel = max(max_rel, diff / denom)
            if not math.isfinite(diff) or diff > atol + rtol * max(abs(x), abs(y)):
                return False, "invariant '%s' mismatch: %s vs %s" % (key, x, y)
    return True, "invariants within tolerance (%d keys, max_rel=%.3g)" % (len(a), max_rel)


def compare_file(ref, cand, rtol, atol):
    if not cand.exists():
        return False, "candidate missing"
    if ref.stat().st_size == cand.stat().st_size and file_hash(ref) == file_hash(cand):
        return True, "exact"
    if ref.suffix == ".log":
        return compare_log(ref, cand, rtol, atol)
    a = numeric_tokens(ref)
    b = numeric_tokens(cand)
    if a is None or b is None or not a or not b:
        return False, "binary/text differs"
    if len(a) != len(b):
        return False, "numeric token count differs: %d vs %d" % (len(a), len(b))
    max_abs = 0.0
    max_rel = 0.0
    for x, y in zip(a, b):
        diff = abs(x - y)
        denom = max(abs(x), abs(y), atol)
        max_abs = max(max_abs, diff)
        max_rel = max(max_rel, diff / denom)
        if not math.isfinite(diff) or diff > atol + rtol * max(abs(x), abs(y)):
            return False, "numeric mismatch max_abs>tol at values %s vs %s" % (x, y)
    return True, "numeric within tolerance max_abs=%.6g max_rel=%.6g" % (max_abs, max_rel)


def _skip(path):
    if path.name in SKIP_NAMES:
        return True
    if SKIP_NAME_RE.search(path.name):
        return True
    return any(part in SKIP_DIR_PARTS for part in path.parts)


def collect_files(root):
    if root.is_file():
        return [root]
    return [p for p in root.rglob("*") if p.is_file() and not _skip(p)]


def main():
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
    print("wrote %s; status=%s" % (out, "OK" if ok_all else "NG"))
    return 0 if ok_all else 1


if __name__ == "__main__":
    raise SystemExit(main())
