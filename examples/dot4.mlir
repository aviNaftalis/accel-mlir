// 4-element dot product, written as a multiply-accumulate chain with generic
// arith ops. The --fuse-mac pass raises every (mulf, addf) pair into an
// accel.mac, turning this into the exact shape an accelerator's MAC array runs:
//
//   dot = a0*b0 + a1*b1 + a2*b2 + a3*b3
//
// After --fuse-mac this becomes four chained accel.mac ops.
func.func @dot4(%a0: f32, %a1: f32, %a2: f32, %a3: f32,
                %b0: f32, %b1: f32, %b2: f32, %b3: f32) -> f32 {
  %zero = arith.constant 0.0 : f32

  %m0 = arith.mulf %a0, %b0 : f32
  %s0 = arith.addf %m0, %zero : f32

  %m1 = arith.mulf %a1, %b1 : f32
  %s1 = arith.addf %m1, %s0 : f32

  %m2 = arith.mulf %a2, %b2 : f32
  %s2 = arith.addf %m2, %s1 : f32

  %m3 = arith.mulf %a3, %b3 : f32
  %s3 = arith.addf %m3, %s2 : f32

  return %s3 : f32
}
