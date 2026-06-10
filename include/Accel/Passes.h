#ifndef ACCEL_PASSES_H
#define ACCEL_PASSES_H

#include <memory>

namespace mlir {
class Pass;

namespace accel {

// The raising/optimization pass: arith.mulf + arith.addf -> accel.mac.
std::unique_ptr<Pass> createFuseMacPass();
void registerFuseMacPass();

// The lowering pass: accel.* -> LLVM dialect.
std::unique_ptr<Pass> createAccelToLLVMPass();
void registerAccelToLLVMPass();

} // namespace accel
} // namespace mlir

#endif // ACCEL_PASSES_H
