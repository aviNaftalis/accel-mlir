// Polynomial evaluation by Horner's method, written *directly* in the accel
// dialect (no raising needed). Each step is exactly one multiply-accumulate,
// which is why Horner's scheme maps so cleanly onto MAC hardware:
//
//   p(x) = 2*x^3 + 3*x^2 + 4*x + 5
//        = ((2*x + 3)*x + 4)*x + 5
func.func @poly(%x: f32) -> f32 {
  %c2 = accel.constant 2.0
  %c3 = accel.constant 3.0
  %c4 = accel.constant 4.0
  %c5 = accel.constant 5.0

  %r0 = accel.mac %c2, %x, %c3 : f32   // 2*x + 3
  %r1 = accel.mac %r0, %x, %c4 : f32   // (2*x+3)*x + 4
  %r2 = accel.mac %r1, %x, %c5 : f32   // ((2*x+3)*x+4)*x + 5
  return %r2 : f32
}
