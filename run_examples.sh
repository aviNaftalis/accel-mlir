#!/usr/bin/env bash
# Compile every runnable example through the full pipeline
#   .mlir --fuse-mac --convert-accel-to-llvm --convert-func-to-llvm
#         --reconcile-unrealized-casts | mlir-translate | clang + driver
# then run it and check the result.
set -uo pipefail

LLVM=/usr/lib/llvm-21/bin
ROOT="$(cd "$(dirname "$0")" && pwd)"
OPT="$ROOT/build/bin/accel-opt"
OUT="$ROOT/build/examples-out"
mkdir -p "$OUT"

# name | entry-source | driver | expected-substring
EXAMPLES=(
  "mac|examples/mac.mlir|examples/driver.c|= 10.0"
  "dot4|examples/dot4.mlir|examples/dot4_driver.c|= 70.0"
  "poly|examples/poly.mlir|examples/poly_driver.c|= 41.0"
)

pass=0; fail=0
for row in "${EXAMPLES[@]}"; do
  IFS='|' read -r name src driver expect <<<"$row"
  "$OPT" "$ROOT/$src" --fuse-mac --convert-accel-to-llvm --convert-arith-to-llvm \
      --convert-func-to-llvm --reconcile-unrealized-casts 2>/dev/null \
    | "$LLVM/mlir-translate" --mlir-to-llvmir 2>/dev/null > "$OUT/$name.ll"
  "$LLVM/clang" "$OUT/$name.ll" "$ROOT/$driver" -o "$OUT/$name" 2>/dev/null
  result="$("$OUT/$name")"
  if [[ "$result" == *"$expect"* ]]; then
    echo "PASS  $result"; pass=$((pass+1))
  else
    echo "FAIL  $name -> $result (wanted '$expect')"; fail=$((fail+1))
  fi
done
echo "=== $pass passed, $fail failed ==="
[ "$fail" -eq 0 ]
