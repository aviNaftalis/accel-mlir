// RUN: accel-opt %s --fuse-mac | FileCheck %s

// CHECK-LABEL: func.func @fuse
// CHECK-NOT: arith.mulf
// CHECK-NOT: arith.addf
// CHECK: accel.mac
func.func @fuse(%a: f32, %b: f32, %c: f32) -> f32 {
  %0 = arith.mulf %a, %b : f32
  %1 = arith.addf %0, %c : f32
  return %1 : f32
}
