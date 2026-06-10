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

#endif // ACCEL_OPS
