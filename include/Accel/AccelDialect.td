#ifndef ACCEL_DIALECT
#define ACCEL_DIALECT

include "mlir/IR/OpBase.td"

//===----------------------------------------------------------------------===//
// The `accel` dialect: a tiny stand-in for an AI-accelerator IR, built around
// the multiply-accumulate (MAC) primitive that dominates neural-network and
// systolic-array hardware.
//===----------------------------------------------------------------------===//

def Accel_Dialect : Dialect {
  let name = "accel";
  let summary = "A minimal accelerator dialect centered on multiply-accumulate.";
  let description = [{
    Models a hardware accelerator at a very high level. Its hero op,
    `accel.mac`, computes `a * b + c` in one step, mirroring the fused MAC unit
    found in real ML accelerators. The dialect lowers all the way to the LLVM
    dialect so programs can be compiled to native code.
  }];
  let cppNamespace = "::mlir::accel";

  // Lets folders that produce a constant attribute be rebuilt into an
  // accel.constant op (see AccelDialect::materializeConstant).
  let hasConstantMaterializer = 1;
}

// Base class for all ops in this dialect.
class Accel_Op<string mnemonic, list<Trait> traits = []> :
    Op<Accel_Dialect, mnemonic, traits>;

#endif // ACCEL_DIALECT
