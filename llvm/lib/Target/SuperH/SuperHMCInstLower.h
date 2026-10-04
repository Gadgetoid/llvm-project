//===-- SuperHMCInstLower.h - Lower MachineInstr to MCInst ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SUPERH_SUPERHMCINSTLOWER_H
#define LLVM_LIB_TARGET_SUPERH_SUPERHMCINSTLOWER_H

namespace llvm {
class AsmPrinter;
class MachineInstr;
class MachineOperand;
class MCContext;
class MCExpr;
class MCInst;
class MCOperand;

/// Lowers MachineInstrs and their operands to MC.
class SuperHMCInstLower {
  MCContext &Ctx;
  AsmPrinter &Printer;

public:
  SuperHMCInstLower(MCContext &Ctx, AsmPrinter &Printer)
      : Ctx(Ctx), Printer(Printer) {}

  void lowerInstruction(const MachineInstr &MI, MCInst &OutMI) const;

  /// The expression for a symbolic or immediate operand.
  const MCExpr *lowerExpression(const MachineOperand &MO) const;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SUPERH_SUPERHMCINSTLOWER_H
