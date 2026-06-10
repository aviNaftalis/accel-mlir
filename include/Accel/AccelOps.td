#ifndef ACCEL_OPS
#define ACCEL_OPS

include "AccelDialect.td"
include "mlir/IR/BuiltinTypes.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

//===----------------------------------------------------------------------===//
// accel.constant
//===----------------------------------------------------------------------===//

def Accel_ConstantOp : Accel_Op<"constant", [Pure, ConstantLike]> {
  let summary = "Materialize an f32 constant.";
  let description = [{
    ```mlir
    %0 = accel.constant 3.0 : f32
    ```
  }];

  let arguments = (ins F32Attr:$value);
  let results = (outs F32:$result);

  let assemblyFormat = "$value attr-dict";
  let hasFolder = 1;
}

//===----------------------------------------------------------------------===//
// accel.mac
//===----------------------------------------------------------------------===//

def Accel_MacOp : Accel_Op<"mac", [Pure]> {
  let summary = "Fused multiply-accumulate: result = a * b + c.";
  let description = [{
    The accelerator primitive. Lowers to a `llvm.fmul` followed by a
    `llvm.fadd`, demonstrating a one-to-many lowering pattern.

    ```mlir
    %r = accel.mac %a, %b, %c : f32
    ```
  }];

  let arguments = (ins F32:$a, F32:$b, F32:$c);
  let results = (outs F32:$result);

  let assemblyFormat = "$a `,` $b `,` $c attr-dict `:` type($result)";
  let hasFolder = 1;
  let hasCanonicalizer = 1;
}

//===----------------------------------------------------------------------===//
// accel.qmac
//===----------------------------------------------------------------------===//

def Accel_QMacOp : Accel_Op<"qmac", [Pure]> {
  let summary = "Quantized multiply-accumulate: i32 acc += (i32)a * (i32)b.";
  let description = [{
    The integer MAC primitive of quantized inference: two `int8` operands are
    multiplied and accumulated into an `int32` (wide accumulation prevents
    overflow). This is the op an INT8-quantized dot product / matmul is built
    from.

    ```mlir
    %acc1 = accel.qmac %a, %b, %acc0 : i8, i8, i32
    ```
  }];

  let arguments = (ins I8:$a, I8:$b, I32:$acc);
  let results = (outs I32:$result);

  let assemblyFormat =
      "$a `,` $b `,` $acc attr-dict `:` type($a) `,` type($b) `,` type($acc)";
}

#endif // ACCEL_OPS
