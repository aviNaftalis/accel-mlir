//===- accel-opt.cpp - The accel optimizer driver ------------------------===//
//
// A drop-in `mlir-opt`-style tool that knows about the accel dialect and its
// two passes, plus all upstream dialects/passes (so --convert-func-to-llvm and
// --reconcile-unrealized-casts are available for the full lowering pipeline).
//
//===----------------------------------------------------------------------===//

#include "Accel/AccelDialect.h"
#include "Accel/Passes.h"

#include "mlir/IR/DialectRegistry.h"
#include "mlir/InitAllDialects.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  mlir::registerAllDialects(registry);
  registry.insert<mlir::accel::AccelDialect>();

  mlir::registerAllPasses();
  mlir::accel::registerFuseMacPass();
  mlir::accel::registerAccelToLLVMPass();

  return mlir::asMainReturnCode(mlir::MlirOptMain(
      argc, argv, "accel optimizer driver\n", registry));
}
