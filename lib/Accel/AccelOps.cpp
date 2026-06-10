#include "Accel/AccelOps.h"

#include "Accel/AccelDialect.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Matchers.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/PatternMatch.h"
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

//===----------------------------------------------------------------------===//
// Canonicalization: algebraic simplification + strength reduction
//===----------------------------------------------------------------------===//

namespace {
/// Eliminates the multiply in accel.mac when one factor is a small constant:
///   mac(a, 1, c) -> a + c          (multiply by one is free)
///   mac(a, 2, c) -> (a + a) + c    (strength reduction: mul-by-2 -> add)
/// and the symmetric cases where the constant is the first factor.
struct StrengthReduceMac : public OpRewritePattern<MacOp> {
  using OpRewritePattern<MacOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(MacOp op,
                                PatternRewriter &rewriter) const override {
    Value a = op.getA(), b = op.getB(), c = op.getC();

    auto isConst = [](Value v, double want) {
      llvm::APFloat f(0.0f);
      return matchPattern(v, m_ConstantFloat(&f)) && f.isExactlyValue(want);
    };

    // multiply by one  ->  a single add
    if (isConst(b, 1.0)) {
      rewriter.replaceOpWithNewOp<arith::AddFOp>(op, a, c);
      return success();
    }
    if (isConst(a, 1.0)) {
      rewriter.replaceOpWithNewOp<arith::AddFOp>(op, b, c);
      return success();
    }

    // multiply by two  ->  self-add (strength reduction)
    if (isConst(b, 2.0)) {
      Value dbl = arith::AddFOp::create(rewriter, op.getLoc(), a, a);
      rewriter.replaceOpWithNewOp<arith::AddFOp>(op, dbl, c);
      return success();
    }
    if (isConst(a, 2.0)) {
      Value dbl = arith::AddFOp::create(rewriter, op.getLoc(), b, b);
      rewriter.replaceOpWithNewOp<arith::AddFOp>(op, dbl, c);
      return success();
    }

    return failure();
  }
};
} // namespace

void MacOp::getCanonicalizationPatterns(RewritePatternSet &results,
                                        MLIRContext *context) {
  results.add<StrengthReduceMac>(context);
}
