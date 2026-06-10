#include "Accel/AccelOps.h"

#include "Accel/AccelDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"
#include "llvm/ADT/APFloat.h"

using namespace mlir;
using namespace mlir::accel;

// Auto-generated op definitions.
#define GET_OP_CLASSES
#include "Accel/AccelOps.cpp.inc"

//===----------------------------------------------------------------------===//
// Folders
//===----------------------------------------------------------------------===//

OpFoldResult ConstantOp::fold(ConstantOp::FoldAdaptor adaptor) {
  // accel.constant is its own constant value.
  return getValueAttr();
}

OpFoldResult MacOp::fold(MacOp::FoldAdaptor adaptor) {
  auto a = llvm::dyn_cast_or_null<FloatAttr>(adaptor.getA());
  auto b = llvm::dyn_cast_or_null<FloatAttr>(adaptor.getB());
  auto c = llvm::dyn_cast_or_null<FloatAttr>(adaptor.getC());

  // Algebraic identity: 0 * x + c == c (even when c is not a constant).
  if ((a && a.getValue().isZero()) || (b && b.getValue().isZero()))
    return getC();

  // Full constant folding: a * b + c.
  if (a && b && c) {
    llvm::APFloat result = a.getValue();
    result.multiply(b.getValue(), llvm::APFloat::rmNearestTiesToEven);
    result.add(c.getValue(), llvm::APFloat::rmNearestTiesToEven);
    return FloatAttr::get(getType(), result);
  }

  return {};
}
