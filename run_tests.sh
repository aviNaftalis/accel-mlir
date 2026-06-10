#!/usr/bin/env bash
# FileCheck tests for the accel dialect, its fuse pass, and its lowering.
set -uo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
OPT="$ROOT/build/bin/accel-opt"
FC="${FILECHECK:-/usr/lib/llvm-21/bin/FileCheck}"

declare -A TESTS=(
  ["roundtrip.mlir"]=""
  ["fuse.mlir"]="--fuse-mac"
  ["lower.mlir"]="--convert-accel-to-llvm"
  ["canonicalize.mlir"]="--canonicalize"
  ["qmac.mlir"]="--convert-accel-to-llvm"
)

pass=0; fail=0
for t in "${!TESTS[@]}"; do
  f="$ROOT/test/$t"
  if "$OPT" "$f" ${TESTS[$t]} 2>/dev/null | "$FC" "$f" >/dev/null 2>&1; then
    echo "PASS  $t"; pass=$((pass+1))
  else
    echo "FAIL  $t"; fail=$((fail+1))
  fi
done
echo "=== $pass passed, $fail failed ==="
[ "$fail" -eq 0 ]
