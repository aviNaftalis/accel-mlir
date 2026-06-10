#ifndef ACCEL_ACCELOPS_H
#define ACCEL_ACCELOPS_H

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

// Pull in the auto-generated op class declarations.
#define GET_OP_CLASSES
#include "Accel/AccelOps.h.inc"

#endif // ACCEL_ACCELOPS_H
