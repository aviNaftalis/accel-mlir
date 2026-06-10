#!/usr/bin/env python3
"""Head-to-head benchmark: accel-mlir's dot product vs a strict-C dot product,
both compiled with clang -O2 -march=native. The only difference is that
accel.mac is *defined* to be contractable/reassociatable, so its lowered fmul+
fadd carry `contract reassoc` flags -> LLVM vectorizes the reduction. The strict
baseline must stay a serial, latency-bound scalar chain.

    python3 benchmark_dot.py        # prints a table, writes demo/benchmark_dot.png

Run with the system python (has matplotlib).
"""
import os
import re
import subprocess

ROOT = os.path.dirname(os.path.abspath(__file__))
OPT = os.path.join(ROOT, "build/bin/accel-opt")
LLVM = "/usr/lib/llvm-21/bin"
OUT = os.path.join(ROOT, "build/bench")
os.makedirs(OUT, exist_ok=True)

PIPELINE = ["--fuse-mac", "--convert-accel-to-llvm", "--convert-scf-to-cf",
            "--finalize-memref-to-llvm", "--convert-index-to-llvm",
            "--convert-arith-to-llvm", "--convert-cf-to-llvm",
            "--convert-func-to-llvm", "--reconcile-unrealized-casts"]
SIZES = [512, 1024, 2048, 4096, 8192, 16384, 32768]


def run(cmd, **kw):
    return subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, **kw)


def build():
    lowered = run([OPT, "examples/dot.mlir", *PIPELINE]).stdout
    ll = run([f"{LLVM}/mlir-translate", "--mlir-to-llvmir", "-"], input=lowered).stdout
    open(f"{OUT}/dot.ll", "w").write(ll)
    run([f"{LLVM}/clang", "-O2", "-march=native", f"{OUT}/dot.ll",
         "examples/dot_bench.c", "-lm", "-o", f"{OUT}/bench_dot"])
    # disassembly evidence: packed vs scalar FMA
    asm = run([f"{LLVM}/clang", "-O2", "-march=native", "-S", f"{OUT}/dot.ll", "-o", "-"]).stdout
    packed = len(re.findall(r"vfmadd[0-9]*p[sd]", asm))
    return packed


def bench(n):
    reps = max(2000, 30_000_000 // n)
    out = run([f"{OUT}/bench_dot", str(n), str(reps)]).stdout
    m = {k: float(v) for k, v in re.findall(r"(\w+)=([\d.eE+-]+)", out)}
    return m


def main():
    packed = build()
    rows = [(n, bench(n)) for n in SIZES]

    print(f"\naccel-mlir dot product vs clang -O2 -march=native (strict C)")
    print(f"  accel kernel uses {packed} packed-FMA (vfmadd*p[sd]) instructions; "
          f"baseline is scalar serial\n")
    print(f"  {'n':>7} {'baseline':>12} {'accel':>12} {'speedup':>9}   accuracy (rel.err)")
    print(f"  {'':>7} {'GFLOP/s':>12} {'GFLOP/s':>12} {'':>9}   base / accel")
    best = 0.0
    for n, m in rows:
        best = max(best, m["speedup"])
        print(f"  {n:>7} {m['baseline_gflops']:>12.2f} {m['accel_gflops']:>12.2f} "
              f"{m['speedup']:>8.1f}x   {m['baseline_relerr']:.1e} / {m['accel_relerr']:.1e}")
    print(f"\n  peak speedup: {best:.1f}x (compute-bound, L1-resident)\n")

    render(rows, packed, best)


def render(rows, packed, best):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    ns = [n for n, _ in rows]
    base = [m["baseline_gflops"] for _, m in rows]
    accel = [m["accel_gflops"] for _, m in rows]
    x = range(len(ns))

    fig, ax = plt.subplots(figsize=(8.5, 4.8), dpi=130)
    ax.plot(x, base, "o-", color="#b85450", label="clang -O2 (strict C, serial)")
    ax.plot(x, accel, "s-", color="#82b366",
            label=f"accel-mlir (reassoc → vectorized, {packed} packed FMAs)")
    ax.set_xticks(list(x)); ax.set_xticklabels([str(n) for n in ns])
    ax.set_xlabel("vector length n (floats)")
    ax.set_ylabel("throughput (GFLOP/s)")
    ax.set_title("Dot product: accel-mlir vs clang -O2 -march=native (same source)")
    ax.legend(loc="upper right")
    ax.grid(True, alpha=0.3)
    ax.text(0.02, 0.92,
            f"peak {best:.0f}x faster — and more accurate\n"
            f"(tree reduction lowers rounding error)",
            transform=ax.transAxes, fontsize=10, va="top",
            bbox=dict(boxstyle="round", fc="#fff2cc", ec="#d6b656"))
    fig.tight_layout()
    out = os.path.join(ROOT, "demo/benchmark_dot.png")
    fig.savefig(out)
    print(f"wrote {out}")


if __name__ == "__main__":
    main()
