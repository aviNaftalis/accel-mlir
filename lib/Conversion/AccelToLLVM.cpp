//===- AccelToLLVM.cpp - Lower the accel dialect to the LLVM dialect -----===//
//
// The lowering half of the project. Uses MLIR's dialect-conversion framework:
// a ConversionTarget declaring the LLVM dialect legal and accel illegal, a
// trivial f32->f32 type converter, and one OpConversionPattern per accel op.
//
//===----------------------------------------------------------------------===//

#include "Accel/AccelDialect.h"
#include "Accel/AccelOps.h"
#include "Accel/Passes.h"

#include "mlir/Conversion/LLVMCommon/ConversionTarget.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

using namespace mlir;

namespace {

/// accel.constant -> llvm.mlir.constant
struct ConstantOpLowering : public ConvertOpToLLVMPattern<accel::ConstantOp> {
  using ConvertOpToLLVMPattern<accel::ConstantOp>::ConvertOpToLLVMPattern;

  LogicalResult
  matchAndRewrite(accel::ConstantOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    Type resultType = typeConverter->convertType(op.getType());
    if (!resultType)
      return failure();
    rewriter.replaceOpWithNewOp<LLVM::ConstantOp>(op, resultType,
                                                  op.getValueAttr());
    return success();
  }
};

/// accel.mac -> llvm.fmul + llvm.fadd, both carrying `contract reassoc`.
///
/// This is the whole point of the dialect. By *defining* accel.mac as a fused,
/// reassociatable multiply-accumulate (a domain decision, exactly what ML
/// frameworks assume), the lowered ops carry fast-math flags a C compiler may
/// not assume at -O2:
///   * `contract` lets the backend fuse the mul+add into one hardware FMA
///     (one rounding instead of two);
///   * `reassoc` on the add makes an accel.mac accumulation a *recognizable,
///     reassociatable reduction*, which lets LLVM vectorize it into wide
///     multi-accumulator FMAs.
/// (We emit fmul+fadd rather than llvm.fma because the loop vectorizer does not
/// recognize the opaque fma intrinsic as a reduction.)
struct MacOpLowering : public ConvertOpToLLVMPattern<accel::MacOp> {
  using ConvertOpToLLVMPattern<accel::MacOp>::ConvertOpToLLVMPattern;

  LogicalResult
  matchAndRewrite(accel::MacOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto flags = LLVM::FastmathFlagsAttr::get(
        rewriter.getContext(),
        LLVM::FastmathFlags::contract | LLVM::FastmathFlags::reassoc);

    auto mul = LLVM::FMulOp::create(rewriter, op.getLoc(), adaptor.getA(),
                                    adaptor.getB());
    mul.setFastmathFlagsAttr(flags);
    auto add = rewriter.replaceOpWithNewOp<LLVM::FAddOp>(op, mul,
                                                         adaptor.getC());
    add.setFastmathFlagsAttr(flags);
    return success();
  }
};

struct AccelToLLVMPass
    : public PassWrapper<AccelToLLVMPass, OperationPass<ModuleOp>> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(AccelToLLVMPass)

  StringRef getArgument() const final { return "convert-accel-to-llvm"; }
  StringRef getDescription() const final {
    return "Lower the accel dialect to the LLVM dialect.";
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<LLVM::LLVMDialect>();
  }

  void runOnOperation() override {
    LLVMConversionTarget target(getContext());
    target.addLegalOp<ModuleOp>();

    LLVMTypeConverter typeConverter(&getContext());

    RewritePatternSet patterns(&getContext());
    patterns.add<ConstantOpLowering, MacOpLowering>(typeConverter);

    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace

std::unique_ptr<Pass> mlir::accel::createAccelToLLVMPass() {
  return std::make_unique<AccelToLLVMPass>();
}

void mlir::accel::registerAccelToLLVMPass() {
  PassRegistration<AccelToLLVMPass>();
}
