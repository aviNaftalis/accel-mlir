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
