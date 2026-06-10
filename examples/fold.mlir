// Demonstrates the op folders. Run with:
//
//   accel-opt examples/fold.mlir --canonicalize
//
// Both functions collapse to a single constant: the algebraic identity
// 0*x + c == c, and full constant folding of 2*3 + 4 == 10.
func.func @fold_identity(%x: f32) -> f32 {
  %zero = accel.constant 0.0
  %c = accel.constant 7.0
  %r = accel.mac %zero, %x, %c : f32   // 0*x + 7  -->  7
  return %r : f32
}

func.func @fold_const() -> f32 {
  %a = accel.constant 2.0
  %b = accel.constant 3.0
  %c = accel.constant 4.0
  %r = accel.mac %a, %b, %c : f32      // 2*3 + 4  -->  10
  return %r : f32
}
