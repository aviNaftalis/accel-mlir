// RUN: accel-opt %s | FileCheck %s

// CHECK-LABEL: func.func @mac_roundtrip
func.func @mac_roundtrip(%a: f32, %b: f32, %c: f32) -> f32 {
  // CHECK: accel.mac %{{.*}}, %{{.*}}, %{{.*}} : f32
  %0 = accel.mac %a, %b, %c : f32
  return %0 : f32
}

// CHECK-LABEL: func.func @const_roundtrip
func.func @const_roundtrip() -> f32 {
  // CHECK: accel.constant 3.000000e+00
  %0 = accel.constant 3.0
  return %0 : f32
}
