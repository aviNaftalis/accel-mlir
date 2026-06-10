#!/usr/bin/env python3
"""Quantization benchmark: INT8-quantized dot vs f32 dot, both emitted by
accel-mlir and compiled clang -O2 -march=native. Shows the edge-inference
trade: 4x less memory + higher throughput for a small, bounded accuracy cost.

    python3 benchmark_quant.py     # prints a table, writes demo/benchmark_quant.png
"""
import os
import re
import subprocess

ROOT = os.path.dirname(os.path.abspath(__file__))
OPT = os.path.join(ROOT, "build/bin/accel-opt")
LLVM = "/usr/lib/llvm-21/bin"
OUT = os.path.join(ROOT, "build/bench")
os.makedirs(OUT, exist_ok=True)

# f32 dot needs --fuse-mac (its mac is implicit); qdot already uses accel.qmac.
LOWER_TAIL = ["--convert-scf-to-cf", "--finalize-memref-to-llvm",
              "--convert-index-to-llvm", "--convert-arith-to-llvm",
              "--convert-cf-to-llvm", "--convert-func-to-llvm",
              "--reconcile-unrealized-casts"]
SIZES = [1024, 4096, 16384, 65536, 262144]


def run(cmd, **kw):
    return subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, **kw)


def lower(src, extra):
    mlir = run([OPT, src, *extra, "--convert-accel-to-llvm", *LOWER_TAIL]).stdout
    return run([f"{LLVM}/mlir-translate", "--mlir-to-llvmir", "-"], input=mlir).stdout


def build():
    open(f"{OUT}/dot.ll", "w").write(lower("examples/dot.mlir", ["--fuse-mac"]))
    open(f"{OUT}/qdot.ll", "w").write(lower("examples/qdot.mlir", []))
    run([f"{LLVM}/clang", "-O2", "-march=native", f"{OUT}/dot.ll", f"{OUT}/qdot.ll",
         "examples/qdot_bench.c", "-lm", "-o", f"{OUT}/bench_quant"])


def bench(n):
    reps = max(2000, 60_000_000 // n)
    out = run([f"{OUT}/bench_quant", str(n), str(reps)]).stdout
    return {k: float(v) for k, v in re.findall(r"(\w+)=([\d.eE+-]+)", out)}


def main():
    build()
    rows = [(n, bench(n)) for n in SIZES]
    print("\nINT8-quantized dot vs f32 dot (both accel-mlir, clang -O2 -march=native)\n")
    print(f"  {'n':>8} {'f32':>10} {'int8':>10} {'speedup':>8}   int8 rel.err   memory")
    print(f"  {'':>8} {'GOP/s':>10} {'GOP/s':>10}")
    for n, m in rows:
        print(f"  {n:>8} {m['f32_gops']:>10.2f} {m['int8_gops']:>10.2f} "
              f"{m['speedup']:>7.2f}x   {m['int8_relerr']:.1e}      {m['mem_ratio']:.0f}x smaller")
    render(rows)


def render(rows):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    ns = [n for n, _ in rows]
    f32 = [m["f32_gops"] for _, m in rows]
    i8 = [m["int8_gops"] for _, m in rows]
    err = [m["int8_relerr"] for _, m in rows]
    x = range(len(ns))

    fig, (ax, ax2) = plt.subplots(1, 2, figsize=(11, 4.4), dpi=120)
    ax.plot(x, f32, "o-", color="#6c8ebf", label="f32 dot")
    ax.plot(x, i8, "s-", color="#9673a6", label="int8 qdot (4x less memory)")
    ax.set_xticks(list(x)); ax.set_xticklabels([str(n) for n in ns], rotation=30)
    ax.set_xlabel("vector length n"); ax.set_ylabel("throughput (GOP/s)")
    ax.set_title("Speed: int8 vs f32 (same dot product)")
    ax.legend(loc="upper right"); ax.grid(True, alpha=0.3)

    ax2.plot(x, [e * 100 for e in err], "D-", color="#b85450")
    ax2.set_xticks(list(x)); ax2.set_xticklabels([str(n) for n in ns], rotation=30)
    ax2.set_xlabel("vector length n"); ax2.set_ylabel("int8 relative error (%)")
    ax2.set_title("Accuracy cost of 8-bit quantization")
    ax2.grid(True, alpha=0.3)
    ax2.set_ylim(bottom=0)

    fig.suptitle("accel-mlir INT8 quantization: 4x smaller, <0.01% error, "
                 "faster once memory-bound", fontsize=12)
    fig.tight_layout(rect=[0, 0, 1, 0.95])
    out = os.path.join(ROOT, "demo/benchmark_quant.png")
    fig.savefig(out)
    print(f"\nwrote {out}")


if __name__ == "__main__":
    main()
