# Optimizations in accel-mlir

The middle-end implements a handful of classic compiler optimizations on the
`accel` dialect. Each is small on purpose — the point is to show the *technique*
and where it lives in MLIR, with both the theory and the implementation.

Run the showcase:

```bash
build/bin/accel-opt examples/strength.mlir --canonicalize
build/bin/accel-opt examples/strength.mlir --canonicalize --cse
```

## 1. Constant folding (partial evaluation)

If all inputs to an op are known at compile time, evaluate it then and there.
`accel.mac(2, 3, 4)` becomes the constant `10`.

- **Theory:** partial evaluation / constant propagation over the SSA value graph.
- **Code:** `MacOp::fold` and `ConstantOp::fold` in
  [`lib/Accel/AccelOps.cpp`](lib/Accel/AccelOps.cpp). `fold` returns an
  `Attribute`; the dialect's `materializeConstant` hook rebuilds it into an
  `accel.constant`, which is what lets the fold actually replace the op.

## 2. Algebraic identities

Simplifications that hold by the algebra of the operation, regardless of runtime
values: `0*x + c == c`. Implemented as an early return in `MacOp::fold` (returns
operand `c` directly, even when `c` is not constant).

## 3. Strength reduction

Replace an expensive operation with a cheaper one that computes the same result:

| Pattern | Rewrite | Why |
|---|---|---|
| `mac(x, 1, c)` | `x + c` | multiply by one is identity — drop the multiply |
| `mac(x, 2, c)` | `(x + x) + c` | multiply by two → a single add |

- **Theory:** strength reduction — the canonical example is turning multiplies
  into adds/shifts. Here `*2` becomes a self-add.
- **Code:** `StrengthReduceMac` (an `OpRewritePattern<MacOp>`) registered via
  `MacOp::getCanonicalizationPatterns` in
  [`lib/Accel/AccelOps.cpp`](lib/Accel/AccelOps.cpp). It uses `m_ConstantFloat`
  matchers and rewrites into `arith` ops (declared as a dependent dialect so it
  is always available).

## 4. Operator fusion / raising

The inverse direction: recognize the `mul`-then-`add` idiom and contract it into
the single fused `accel.mac`. This is a peephole/raising pass — the same shape
instruction selection uses to map IR onto fused hardware units (FMA, MAC arrays).

- **Code:** `FuseMulAddPattern` in
  [`lib/Conversion/FuseMacPass.cpp`](lib/Conversion/FuseMacPass.cpp), driven by
  the greedy pattern rewriter (`--fuse-mac`).

## 5. CSE and DCE (composed from upstream)

These optimizations compose with MLIR's built-in passes:

- **DCE:** dead `accel.constant`s left behind after strength reduction are
  removed by `--canonicalize` (its dead-code cleanup).
- **CSE:** two identical `accel.mac`s collapse to one under `--cse` — possible
  precisely because `accel.mac` is marked `Pure` (no side effects), so the CSE
  pass is free to deduplicate it.

## How they fit together

```
--fuse-mac        raise arith.mulf+arith.addf  ->  accel.mac
--canonicalize    fold / identities / strength reduction / DCE
--cse             deduplicate pure ops
--convert-*-to-llvm + --reconcile-unrealized-casts    lower to LLVM dialect
```

The op-count effect is visible in the [benchmark](demo/benchmark.png): the
degree-8 Horner polynomial's 16 `arith` ops fuse into 8 `accel.mac`s.
