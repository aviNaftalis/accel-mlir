// RUN: accel-opt %s --canonicalize | FileCheck %s

// Strength reduction: mac(x, 2, c) -> (x + x) + c, no multiply, no accel.mac.
// CHECK-LABEL: func.func @strength_reduce
// CHECK-NOT: accel.mac
// CHECK: arith.addf %arg0, %arg0
// CHECK: arith.addf
func.func @strength_reduce(%x: f32, %c: f32) -> f32 {
  %two = accel.constant 2.0
  %r = accel.mac %x, %two, %c : f32
  return %r : f32
}

// Multiply-by-one elimination: mac(x, 1, c) -> x + c.
// CHECK-LABEL: func.func @mul_by_one
// CHECK-NOT: accel.mac
// CHECK: arith.addf
func.func @mul_by_one(%x: f32, %c: f32) -> f32 {
  %one = accel.constant 1.0
  %r = accel.mac %x, %one, %c : f32
  return %r : f32
}

// Constant folding all the way to a single constant.
// CHECK-LABEL: func.func @fold
// CHECK-NOT: accel.mac
// CHECK: accel.constant 1.000000e+01
func.func @fold() -> f32 {
  %a = accel.constant 2.0
  %b = accel.constant 3.0
  %c = accel.constant 4.0
  %r = accel.mac %a, %b, %c : f32
  return %r : f32
}
