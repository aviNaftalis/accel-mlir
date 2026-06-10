#include "Accel/AccelDialect.h"
#include "Accel/AccelOps.h"

using namespace mlir;
using namespace mlir::accel;

// Auto-generated dialect definitions.
#include "Accel/AccelDialect.cpp.inc"

void AccelDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "Accel/AccelOps.cpp.inc"
      >();
}

// Rebuild a folded constant attribute into an accel.constant op so that
// canonicalization can fully fold expressions like accel.mac(2, 3, 4) -> 10.
Operation *AccelDialect::materializeConstant(OpBuilder &builder,
                                             Attribute value, Type type,
                                             Location loc) {
  if (auto floatAttr = llvm::dyn_cast<FloatAttr>(value))
    if (llvm::isa<Float32Type>(type))
      return ConstantOp::create(builder, loc, type, floatAttr);
  return nullptr;
}
