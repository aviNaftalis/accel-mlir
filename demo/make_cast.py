#!/usr/bin/env python3
"""Generate an asciinema v2 .cast of the accel-mlir demo by running the real
commands and capturing their output, then (optionally) render it to GIF:

    python3 demo/make_cast.py
    agg demo/demo.cast demo/demo.gif
"""
import json
import os
import subprocess

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OPT = os.path.join(ROOT, "build/bin/accel-opt")
LOWER = ("--fuse-mac --convert-accel-to-llvm --convert-arith-to-llvm "
         "--convert-func-to-llvm --reconcile-unrealized-casts")

PROMPT = "[1;32m~/accel-mlir[0m$ "
WIDTH, HEIGHT = 104, 34
TYPE_DELAY = 0.025      # seconds per typed character
AFTER_CMD = 0.5         # pause before output appears
AFTER_OUT = 1.6         # pause to read output

# (text typed at the prompt, command actually executed)
STEPS = [
    ("# 1. raise generic arithmetic into the accelerator MAC primitive",
     None),
    (f"accel-opt examples/mac.mlir --fuse-mac",
     f"{OPT} examples/mac.mlir --fuse-mac"),
    ("# 2. lower the accel dialect all the way to the LLVM dialect",
     None),
    ("accel-opt examples/mac.mlir \\\n      --fuse-mac --convert-accel-to-llvm \\\n"
     "      --convert-arith-to-llvm --convert-func-to-llvm \\\n"
     "      --reconcile-unrealized-casts",
     f"{OPT} examples/mac.mlir {LOWER}"),
    ("# 3. compile every example to a native binary and run it",
     None),
    ("./run_examples.sh",
     os.path.join(ROOT, "run_examples.sh")),
]


def main():
    events = []
    t = 0.5

    def emit(data):
        nonlocal t
        events.append([round(t, 3), "o", data])

    emit(PROMPT)
    for typed, cmd in STEPS:
        # animate typing
        for ch in typed:
            t += TYPE_DELAY
            emit("\r\n" if ch == "\n" else ch)
        t += 0.15
        emit("\r\n")
        if cmd is not None:
            t += AFTER_CMD
            out = subprocess.run(cmd, shell=True, cwd=ROOT,
                                 capture_output=True, text=True)
            text = (out.stdout + out.stderr).replace("\n", "\r\n")
            emit(text)
            t += AFTER_OUT
        else:
            t += 0.4
        emit(PROMPT)
    t += 1.0

    header = {"version": 2, "width": WIDTH, "height": HEIGHT,
              "timestamp": 0, "env": {"SHELL": "/bin/bash", "TERM": "xterm-256color"}}
    out_path = os.path.join(ROOT, "demo/demo.cast")
    with open(out_path, "w") as f:
        f.write(json.dumps(header) + "\n")
        for ev in events:
            f.write(json.dumps(ev) + "\n")
    print(f"wrote {out_path} ({len(events)} events, ~{t:.1f}s)")


if __name__ == "__main__":
    main()
