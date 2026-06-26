#!/usr/bin/env python3
# CS-path particle invariant comparison (baseline vs candidate).
#   coord / color / kvsml / coord-minmax : exact (deterministic).
#   normal : tolerance (alignment/FP optimizations shift normals at ULP level;
#            physically identical). Exclude auxiliary p_/g_/pt_ outputs.
import sys, os, glob, struct
base, cand, tol = sys.argv[1], sys.argv[2], float(sys.argv[3])
def floats(p):
    d = open(p, "rb").read(); n = len(d) // 4
    return struct.unpack("<%df" % n, d[:n * 4])
ng = []
for bf in sorted(glob.glob(os.path.join(base, "*"))):
    name = os.path.basename(bf)
    if name.startswith(("p_", "g_", "pt_")) or not os.path.isfile(bf):
        continue
    cf = os.path.join(cand, name)
    if not os.path.exists(cf):
        ng.append(name + "(missing)"); continue
    if name.endswith("_normal.dat"):
        a, b = floats(bf), floats(cf)
        if len(a) != len(b):
            ng.append(name + "(count %d!=%d)" % (len(a), len(b))); continue
        m = max((abs(x - y) for x, y in zip(a, b)), default=0.0)
        if m > tol:
            ng.append(name + "(normal max_abs=%.3e>%.1e)" % (m, tol))
    else:
        if open(bf, "rb").read() != open(cf, "rb").read():
            ng.append(name + "(exact diff)")
if ng:
    print("NG: " + "; ".join(ng)); sys.exit(1)
print("OK (coord/color/kvsml exact; normals within tol)"); sys.exit(0)
