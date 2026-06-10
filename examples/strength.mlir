// Optimization showcase. Run:
//
//   accel-opt examples/strength.mlir --canonicalize
//
//   - mac(x, 1, c)  -->  x + c          (multiply-by-one elimination)
//   - mac(x, 2, c)  -->  (x + x) + c    (strength reduction: mul-by-2 -> add)
//   - mac(2, 3, 4)  -->  10             (constant folding + materialization)
//
// Add --cse to also collapse the duplicated computation in @redundant.
func.func @reduce(%x: f32, %c: f32) -> (f32, f32) {
  %one = accel.constant 1.0
  %two = accel.constant 2.0
  %r1 = accel.mac %x, %one, %c : f32   // x*1 + c  ->  x + c
  %r2 = accel.mac %x, %two, %c : f32   // x*2 + c  ->  (x+x) + c
  return %r1, %r2 : f32, f32
}

func.func @fold_chain() -> f32 {
  %a = accel.constant 2.0
  %b = accel.constant 3.0
  %c = accel.constant 4.0
  %r = accel.mac %a, %b, %c : f32      // 2*3 + 4  ->  10
  return %r : f32
}

func.func @redundant(%x: f32, %y: f32, %c: f32) -> f32 {
  %p = accel.mac %x, %y, %c : f32
  %q = accel.mac %x, %y, %c : f32      // identical to %p -> --cse removes it
  %s = arith.addf %p, %q : f32
  return %s : f32
}
