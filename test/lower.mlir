// RUN: accel-opt %s --convert-accel-to-llvm | FileCheck %s

// CHECK-LABEL: llvm.func @lower
// CHECK-NOT: accel.mac
// CHECK: llvm.fmul %{{.*}}, %{{.*}}  {fastmathFlags = #llvm.fastmath<contract, reassoc>}
// CHECK: llvm.fadd %{{.*}}, %{{.*}}  {fastmathFlags = #llvm.fastmath<contract, reassoc>}
llvm.func @lower(%a: f32, %b: f32, %c: f32) -> f32 {
  %0 = accel.mac %a, %b, %c : f32
  llvm.return %0 : f32
}
