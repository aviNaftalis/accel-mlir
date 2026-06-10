# accel-mlir

A small **out-of-tree MLIR dialect** that lowers all the way to native code. It
exists to demonstrate the full MLIR/LLVM stack end to end:

> **generic `arith` IR → raise to a custom dialect → lower to the LLVM dialect → LLVM IR → native binary**

The dialect is built around `accel.mac` — fused multiply-accumulate (`a*b + c`),
the primitive at the heart of every AI accelerator and systolic array.

![demo](demo/demo.gif)

## What it exercises

| MLIR concept | Where |
|---|---|
| Dialect & op definition in ODS / TableGen | [`include/Accel/*.td`](include/Accel) |
| Custom assembly format, verifiers, folders | `AccelOps.td`, [`lib/Accel/AccelOps.cpp`](lib/Accel/AccelOps.cpp) |
| Pattern rewriting / IR transformation (an *optimization*) | [`lib/Conversion/FuseMacPass.cpp`](lib/Conversion/FuseMacPass.cpp) — raises `arith.mulf`+`arith.addf` into `accel.mac` |
| The dialect-conversion framework (a *lowering*) | [`lib/Conversion/AccelToLLVM.cpp`](lib/Conversion/AccelToLLVM.cpp) — `ConversionTarget`, `TypeConverter`, one-to-many op lowering |
| An `mlir-opt`-style driver | [`tools/accel-opt`](tools/accel-opt) |
| Codegen to a runnable binary | [`run_demo.sh`](run_demo.sh) |

## Build

Built against the **distro LLVM/MLIR 21** packages — no source build required.

```bash
sudo apt-get install -y libmlir-21-dev mlir-21-tools llvm-21-dev clang-21 lld-21

cmake -G Ninja -B build \
  -DMLIR_DIR=/usr/lib/llvm-21/lib/cmake/mlir \
  -DLLVM_DIR=/usr/lib/llvm-21/lib/cmake/llvm \
  -DCMAKE_C_COMPILER=clang-21 -DCMAKE_CXX_COMPILER=clang++-21
cmake --build build
```

## Run the end-to-end demo

```bash
./run_demo.sh
```

It takes [`examples/mac.mlir`](examples/mac.mlir) (a MAC written with generic
`arith` ops), runs the two passes, translates to LLVM IR, links it with a tiny
C driver, and runs it:

```
compute(2, 3, 4) = 10.0 (expected 10.0)
```

## Examples

Each lives in [`examples/`](examples) and exercises the pipeline differently.
`./run_examples.sh` compiles all the runnable ones to native binaries and checks
their results.

| File | What it shows |
|---|---|
| [`mac.mlir`](examples/mac.mlir) | The base case: one MAC, written in `arith`, raised then lowered. |
| [`dot4.mlir`](examples/dot4.mlir) | A 4-element dot product → a **chain of 4 `accel.mac`** (a MAC array). |
| [`poly.mlir`](examples/poly.mlir) | Polynomial eval by Horner's method, written **directly in the dialect**. |
| [`fold.mlir`](examples/fold.mlir) | The **folders / constant materializer**: `0*x+c → c` and `2*3+4 → 10` under `--canonicalize`. |

```bash
./run_examples.sh
# PASS  compute(2, 3, 4) = 10.0 (expected 10.0)
# PASS  dot4([1,2,3,4], [5,6,7,8]) = 70.0 (expected 70.0)
# PASS  poly(2) = 41.0 (expected 41.0)
# === 3 passed, 0 failed ===

build/bin/accel-opt examples/fold.mlir --canonicalize   # watch the ops fold away
```

## Recording the demo

The GIF above is generated from real command output:

```bash
python3 demo/make_cast.py        # runs the commands, writes demo/demo.cast
agg demo/demo.cast demo/demo.gif # render to GIF (asciinema-gif generator)
```

## The pipeline, stage by stage

```bash
# 1. raise generic arithmetic into the accelerator primitive
build/bin/accel-opt examples/mac.mlir --fuse-mac
#   arith.mulf + arith.addf  =>  accel.mac

# 2. lower the custom dialect (and func) to the LLVM dialect
build/bin/accel-opt ... --convert-accel-to-llvm --convert-func-to-llvm \
                        --reconcile-unrealized-casts
#   accel.mac  =>  llvm.fmul + llvm.fadd

# 3. translate to LLVM IR
/usr/lib/llvm-21/bin/mlir-translate --mlir-to-llvmir ...

# 4. clang compiles the IR + driver into a native executable
```

## Tests

`test/*.mlir` are `FileCheck` tests covering parse/print round-tripping, the
fuse pass, and the lowering:

```bash
./run_tests.sh
# PASS  roundtrip.mlir
# PASS  fuse.mlir
# PASS  lower.mlir
# === 3 passed, 0 failed ===
```
