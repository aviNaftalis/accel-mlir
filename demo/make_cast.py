#!/usr/bin/env python3
"""Generate asciinema .cast recordings of everything accel-mlir runs, by
executing the real commands and capturing their output. Render with agg:

    python3 demo/make_cast.py
    agg demo/pipeline.cast  demo/pipeline.gif
    agg demo/run.cast       demo/run.gif

Two reels:
  * pipeline - front-end -> raise -> optimize -> lower (how it works)
  * run      - examples + tests + benchmark           (it all works)
"""
import json
import os
import subprocess

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OPT = os.path.join(ROOT, "build/bin/accel-opt")
ACLC = f"{os.path.join(ROOT, '.venv/bin/python')} {os.path.join(ROOT, 'frontend/aclc.py')}"
LOWER = ("--fuse-mac --convert-accel-to-llvm --convert-arith-to-llvm "
         "--convert-func-to-llvm --reconcile-unrealized-casts")

PROMPT = "[1;32m~/accel-mlir[0m$ "
WIDTH, HEIGHT = 100, 30
TYPE_DELAY = 0.02
AFTER_CMD = 0.4
AFTER_OUT = 1.5

# Each step: (text shown at the prompt, command actually run | None for a comment)
REELS = {
    "pipeline": [
        ("# front-end: compile Acl source to accel-dialect MLIR (ANTLR + Python)", None),
        ("aclc examples/quadratic.acl",
         f"{ACLC} examples/quadratic.acl"),
        ("# raise generic arithmetic into the accelerator MAC primitive", None),
        ("aclc examples/quadratic.acl | accel-opt --fuse-mac",
         f"{ACLC} examples/quadratic.acl | {OPT} - --fuse-mac"),
        ("# optimize: strength reduction + constant folding", None),
        ("accel-opt examples/strength.mlir --canonicalize",
         f"{OPT} examples/strength.mlir --canonicalize"),
        ("# lower the accel dialect all the way to the LLVM dialect", None),
        ("accel-opt examples/mac.mlir \\\n      --fuse-mac --convert-accel-to-llvm "
         "--convert-arith-to-llvm \\\n      --convert-func-to-llvm --reconcile-unrealized-casts",
         f"{OPT} examples/mac.mlir {LOWER}"),
    ],
    "run": [
        ("# compile every example (incl. the .acl front-end) to a native binary", None),
        ("./run_examples.sh", os.path.join(ROOT, "run_examples.sh")),
        ("# run the FileCheck test suite", None),
        ("./run_tests.sh", os.path.join(ROOT, "run_tests.sh")),
        ("# benchmark: op-count reduction from fusion", None),
        ("python3 benchmark.py", "python3 benchmark.py"),
        ("# the real win: dot product vs clang -O2 (FP reassociation it can't do)", None),
        ("python3 benchmark_dot.py", "python3 benchmark_dot.py"),
        ("# ML-centric: INT8 quantization -- 4x smaller, bounded accuracy loss", None),
        ("python3 benchmark_quant.py", "python3 benchmark_quant.py"),
    ],
}


def build_cast(steps):
    events = []
    t = 0.5

    def emit(data):
        nonlocal t
        events.append([round(t, 3), "o", data])

    emit(PROMPT)
    for typed, cmd in steps:
        for ch in typed:
            t += TYPE_DELAY
            emit("\r\n" if ch == "\n" else ch)
        t += 0.15
        emit("\r\n")
        if cmd is not None:
            t += AFTER_CMD
            out = subprocess.run(cmd, shell=True, cwd=ROOT,
                                 capture_output=True, text=True)
            emit((out.stdout + out.stderr).replace("\n", "\r\n"))
            t += AFTER_OUT
        else:
            t += 0.4
        emit(PROMPT)
    t += 1.0
    return events


def main():
    header = {"version": 2, "width": WIDTH, "height": HEIGHT, "timestamp": 0,
              "env": {"SHELL": "/bin/bash", "TERM": "xterm-256color"}}
    for name, steps in REELS.items():
        events = build_cast(steps)
        path = os.path.join(ROOT, f"demo/{name}.cast")
        with open(path, "w") as f:
            f.write(json.dumps(header) + "\n")
            for ev in events:
                f.write(json.dumps(ev) + "\n")
        print(f"wrote {path} ({len(events)} events)")


if __name__ == "__main__":
    main()
