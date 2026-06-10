#!/usr/bin/env bash
# End-to-end demo: arith MAC -> accel.mac -> LLVM dialect -> LLVM IR -> native binary.
set -euo pipefail

LLVM=/usr/lib/llvm-21/bin
ROOT="$(cd "$(dirname "$0")" && pwd)"
OPT="$ROOT/build/bin/accel-opt"
OUT="$ROOT/build/demo-out"
mkdir -p "$OUT"

echo "==> 1. raise: arith.mulf + arith.addf  ->  accel.mac"
"$OPT" "$ROOT/examples/mac.mlir" --fuse-mac -o "$OUT/opt.mlir"
cat "$OUT/opt.mlir"

echo "==> 2. lower: accel + func  ->  llvm dialect"
"$OPT" "$OUT/opt.mlir" \
  --convert-accel-to-llvm \
  --convert-arith-to-llvm \
  --convert-func-to-llvm \
  --reconcile-unrealized-casts -o "$OUT/llvm.mlir"
cat "$OUT/llvm.mlir"

echo "==> 3. translate: llvm dialect  ->  LLVM IR"
"$LLVM/mlir-translate" --mlir-to-llvmir "$OUT/llvm.mlir" -o "$OUT/out.ll"

echo "==> 4. compile + link + run"
"$LLVM/clang" "$OUT/out.ll" "$ROOT/examples/driver.c" -o "$OUT/demo"
"$OUT/demo"
