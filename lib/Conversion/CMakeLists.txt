add_mlir_library(MLIRAccelConversion
  FuseMacPass.cpp
  AccelToLLVM.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Accel

  DEPENDS
  AccelOpsIncGen

  LINK_LIBS PUBLIC
  MLIRAccel
  MLIRIR
  MLIRPass
  MLIRArithDialect
  MLIRFuncDialect
  MLIRLLVMDialect
  MLIRLLVMCommonConversion
  MLIRTransforms
  MLIRTransformUtils
  )
