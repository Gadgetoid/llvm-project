//===-- SuperHMCInstLower.cpp - Lower MachineInstr to MCInst --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SuperHMCInstLower.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

const MCExpr *
SuperHMCInstLower::lowerExpression(const MachineOperand &MO) const {
  MCSymbol *Symbol;
  int64_t Offset = 0;
  switch (MO.getType()) {
  case MachineOperand::MO_Immediate:
    return MCConstantExpr::create(MO.getImm(), Ctx);
  case MachineOperand::MO_GlobalAddress:
    Symbol = Printer.getSymbol(MO.getGlobal());
    Offset = MO.getOffset();
    break;
  case MachineOperand::MO_ExternalSymbol:
    Symbol = Printer.GetExternalSymbolSymbol(MO.getSymbolName());
    Offset = MO.getOffset();
    break;
  case MachineOperand::MO_MachineBasicBlock:
    Symbol = MO.getMBB()->getSymbol();
    break;
  case MachineOperand::MO_BlockAddress:
    Symbol = Printer.GetBlockAddressSymbol(MO.getBlockAddress());
    Offset = MO.getOffset();
    break;
  case MachineOperand::MO_JumpTableIndex:
    Symbol = Printer.GetJTISymbol(MO.getIndex());
    break;
  case MachineOperand::MO_ConstantPoolIndex:
    Symbol = Printer.GetCPISymbol(MO.getIndex());
    Offset = MO.getOffset();
    break;
  case MachineOperand::MO_MCSymbol:
    Symbol = MO.getMCSymbol();
    break;
  default:
    llvm_unreachable("unexpected symbolic operand");
  }
  const MCExpr *Expr = MCSymbolRefExpr::create(Symbol, Ctx);
  if (Offset)
    Expr = MCBinaryExpr::createAdd(Expr, MCConstantExpr::create(Offset, Ctx),
                                   Ctx);
  return Expr;
}

void SuperHMCInstLower::lowerInstruction(const MachineInstr &MI,
                                         MCInst &OutMI) const {
  OutMI.setOpcode(MI.getOpcode());
  for (const MachineOperand &MO : MI.operands()) {
    switch (MO.getType()) {
    case MachineOperand::MO_Register:
      if (MO.isImplicit())
        continue;
      OutMI.addOperand(MCOperand::createReg(MO.getReg()));
      break;
    case MachineOperand::MO_Immediate:
      OutMI.addOperand(MCOperand::createImm(MO.getImm()));
      break;
    case MachineOperand::MO_RegisterMask:
      break;
    default:
      OutMI.addOperand(MCOperand::createExpr(lowerExpression(MO)));
      break;
    }
  }
}
