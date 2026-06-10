// A dot product as a reduction loop over two vectors, accumulating with
// accel.mac. This is where the dialect earns its keep: accel.mac is defined to
// be reassociatable, so the lowered FMAs carry `reassoc` and LLVM is free to
// vectorize this reduction. The equivalent strict-C loop at -O2 stays a serial,
// latency-bound scalar chain.
//
// `llvm.emit_c_interface` makes a _mlir_ciface_dot_accel wrapper callable from C.
func.func @dot_accel(%a: memref<?xf32>, %b: memref<?xf32>) -> f32
    attributes {llvm.emit_c_interface} {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %zero = arith.constant 0.0 : f32
  %n = memref.dim %a, %c0 : memref<?xf32>
  %sum = scf.for %i = %c0 to %n step %c1 iter_args(%acc = %zero) -> f32 {
    %ai = memref.load %a[%i] : memref<?xf32>
    %bi = memref.load %b[%i] : memref<?xf32>
    %m = accel.mac %ai, %bi, %acc : f32
    scf.yield %m : f32
  }
  return %sum : f32
}
