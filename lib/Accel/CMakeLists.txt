add_mlir_dialect_library(MLIRAccel
  AccelDialect.cpp
  AccelOps.cpp

  ADDITIONAL_HEADER_DIRS
  ${PROJECT_SOURCE_DIR}/include/Accel

  DEPENDS
  AccelOpsIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRSupport
  MLIRArithDialect
  )
