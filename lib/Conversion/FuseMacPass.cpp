//===- FuseMacPass.cpp - Raise arith.mulf+arith.addf to accel.mac --------===//
//
// An optimization (raising) pass: it recognizes the multiply-accumulate idiom
// expressed with generic `arith` ops and rewrites it into the single
// accelerator primitive `accel.mac`. This is the "I can analyze and transform
// IR" half of the project.
//
//===----------------------------------------------------------------------===//

#include "Accel/AccelDialect.h"
#include "Accel/AccelOps.h"
#include "Accel/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;

namespace {

/// Matches `addf(mulf(a, b), c)` (and the commuted form) and rewrites it to
/// `accel.mac a, b, c`.
struct FuseMulAddPattern : public OpRewritePattern<arith::AddFOp> {
  using OpRewritePattern<arith::AddFOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(arith::AddFOp addOp,
                                PatternRewriter &rewriter) const override {
    // Keep the demo focused on scalar f32.
    if (!addOp.getType().isF32())
      return failure();

    // The multiply can be on either side of the add.
    Value addend = addOp.getRhs();
    auto mul = addOp.getLhs().getDefiningOp<arith::MulFOp>();
    if (!mul) {
      mul = addOp.getRhs().getDefiningOp<arith::MulFOp>();
      addend = addOp.getLhs();
    }
    if (!mul)
      return failure();

    rewriter.replaceOpWithNewOp<accel::MacOp>(
        addOp, addOp.getType(), mul.getLhs(), mul.getRhs(), addend);
    return success();
  }
};

struct FuseMacPass
    : public PassWrapper<FuseMacPass, OperationPass<>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(FuseMacPass)

  StringRef getArgument() const final { return "fuse-mac"; }
  StringRef getDescription() const final {
    return "Fuse arith.mulf + arith.addf into accel.mac.";
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<accel::AccelDialect, arith::ArithDialect>();
  }

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add<FuseMulAddPattern>(&getContext());
    if (failed(applyPatternsGreedily(getOperation(), std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace

std::unique_ptr<Pass> mlir::accel::createFuseMacPass() {
  return std::make_unique<FuseMacPass>();
}

void mlir::accel::registerFuseMacPass() { PassRegistration<FuseMacPass>(); }
