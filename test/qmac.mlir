// RUN: accel-opt %s --convert-accel-to-llvm | FileCheck %s

// CHECK-LABEL: llvm.func @q
// CHECK-NOT: accel.qmac
// CHECK: llvm.sext
// CHECK: llvm.mul
// CHECK: llvm.add
llvm.func @q(%a: i8, %b: i8, %acc: i32) -> i32 {
  %0 = accel.qmac %a, %b, %acc : i8, i8, i32
  llvm.return %0 : i32
}
