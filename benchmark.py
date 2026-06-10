#!/usr/bin/env python3
"""Benchmark the accel-mlir pipeline on examples/horner8.acl and render a chart.

Measures two things:
  1. IR op-count reduction from --fuse-mac (a compiler-level metric).
  2. Runtime throughput of the compiled native function.

Outputs a table to stdout and a chart to demo/benchmark.png. Run with the
system python (which has matplotlib); it shells out to .venv for the front-end.

    python3 benchmark.py
"""
import os
import re
import subprocess

ROOT = os.path.dirname(os.path.abspath(__file__))
VENV_PY = os.path.join(ROOT, ".venv/bin/python")
OPT = os.path.join(ROOT, "build/bin/accel-opt")
LLVM = "/usr/lib/llvm-21/bin"
OUT = os.path.join(ROOT, "build/bench")
SRC = "examples/horner8.acl"
LOWER = ["--fuse-mac", "--convert-accel-to-llvm", "--convert-arith-to-llvm",
         "--convert-func-to-llvm", "--reconcile-unrealized-casts"]
os.makedirs(OUT, exist_ok=True)


def run(cmd, **kw):
    return subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, **kw)


def counts(mlir):
    return {"mulf": mlir.count("arith.mulf"),
            "addf": mlir.count("arith.addf"),
            "mac":  mlir.count("accel.mac")}


def main():
    # 1. front-end -> MLIR
    raw = run([VENV_PY, "frontend/aclc.py", SRC]).stdout
    before = counts(raw)
    # 2. raise into accel.mac
    fused = run([OPT, "-", "--fuse-mac"], input=raw).stdout
    after = counts(fused)

    # 3. compile to native and time it
    llvm_ir = run([OPT, "-", *LOWER], input=raw).stdout
    open(os.path.join(OUT, "horner8.ll"), "w").write(
        run([f"{LLVM}/mlir-translate", "--mlir-to-llvmir", "-"], input=llvm_ir).stdout)
    run([f"{LLVM}/clang", "-O2", f"{OUT}/horner8.ll",
         "examples/bench_driver.c", "-o", f"{OUT}/bench"])
    bench = run([f"{OUT}/bench"]).stdout.strip()
    m = {k: float(v) for k, v in re.findall(r"(\w+)=([\d.]+)", bench)}

    before_total = before["mulf"] + before["addf"]
    after_total = after["mulf"] + after["addf"] + after["mac"]
    print(f"\nhorner8: degree-8 polynomial (Horner form)")
    print(f"  front-end arith ops : {before['mulf']} mulf + {before['addf']} addf = {before_total}")
    print(f"  after --fuse-mac    : {after['mac']} accel.mac  ({before_total} -> {after_total} ops)")
    print(f"  runtime             : {m['ns_per_call']:.2f} ns/call, "
          f"{m['mcalls_per_sec']:.0f} Mcalls/s\n")

    render_chart(before, after, m)


def render_chart(before, after, m):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    cats = ["arith.mulf", "arith.addf", "accel.mac"]
    bvals = [before["mulf"], before["addf"], before["mac"]]
    avals = [after["mulf"], after["addf"], after["mac"]]
    x = range(len(cats))
    w = 0.38

    fig, ax = plt.subplots(figsize=(8, 4.5), dpi=130)
    ax.bar([i - w / 2 for i in x], bvals, w, label="front-end (arith)", color="#6c8ebf")
    ax.bar([i + w / 2 for i in x], avals, w, label="after --fuse-mac", color="#82b366")
    for i, (bv, av) in enumerate(zip(bvals, avals)):
        if bv:
            ax.text(i - w / 2, bv + 0.1, str(bv), ha="center", va="bottom", fontsize=9)
        if av:
            ax.text(i + w / 2, av + 0.1, str(av), ha="center", va="bottom", fontsize=9)

    ax.set_xticks(list(x)); ax.set_xticklabels(cats)
    ax.set_ylabel("op count")
    bt = before["mulf"] + before["addf"]
    at = after["mulf"] + after["addf"] + after["mac"]
    ax.set_title(f"accel-mlir: horner8.acl  —  fusion reduces {bt} arith ops to {at} MACs")
    ax.legend(loc="upper right")
    ax.text(0.02, 0.92,
            f"compiled runtime: {m['ns_per_call']:.2f} ns/call  "
            f"({m['mcalls_per_sec']:.0f} Mcalls/s)",
            transform=ax.transAxes, fontsize=10,
            bbox=dict(boxstyle="round", fc="#fff2cc", ec="#d6b656"))
    ax.set_ylim(0, max(bvals + avals) + 2)
    fig.tight_layout()
    out = os.path.join(ROOT, "demo/benchmark.png")
    fig.savefig(out)
    print(f"wrote {out}")


if __name__ == "__main__":
    main()
