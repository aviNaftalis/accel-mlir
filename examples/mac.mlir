// A multiply-accumulate expressed with generic arith ops.
//
// The pipeline RAISES this to the accelerator primitive accel.mac, then LOWERS
// it all the way to the LLVM dialect:
//
//   accel-opt mac.mlir --fuse-mac \
//     | accel-opt --convert-accel-to-llvm --convert-func-to-llvm \
//                 --reconcile-unrealized-casts
//
// Arguments (not constants) keep the body from folding away, so the mac
// survives to be lowered to llvm.fmul + llvm.fadd.
func.func @compute(%a: f32, %b: f32, %c: f32) -> f32 {
  %0 = arith.mulf %a, %b : f32
  %1 = arith.addf %0, %c : f32
  return %1 : f32
}
