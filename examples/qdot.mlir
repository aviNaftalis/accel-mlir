// INT8-quantized dot product: the same reduction as dot.mlir, but the operands
// are int8 and accumulation is int32 (accel.qmac). This is what a quantizing
// compiler emits for edge inference — 4x smaller data, integer SIMD, and a wide
// accumulator so the sum can't overflow.
//
// Integer addition is associative, so this vectorizes under plain -O2 (no
// fast-math needed). The accompanying harness does the f32<->int8 quant/dequant.
func.func @qdot_accel(%a: memref<?xi8>, %b: memref<?xi8>) -> i32
    attributes {llvm.emit_c_interface} {
  %c0 = arith.constant 0 : index
  %c1 = arith.constant 1 : index
  %zero = arith.constant 0 : i32
  %n = memref.dim %a, %c0 : memref<?xi8>
  %sum = scf.for %i = %c0 to %n step %c1 iter_args(%acc = %zero) -> i32 {
    %ai = memref.load %a[%i] : memref<?xi8>
    %bi = memref.load %b[%i] : memref<?xi8>
    %m = accel.qmac %ai, %bi, %acc : i8, i8, i32
    scf.yield %m : i32
  }
  return %sum : i32
}
