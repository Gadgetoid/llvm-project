//===-- SuperHInstPrinter.cpp - Convert SuperH MCInst to assembly ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints a SuperH MCInst to a .s file in GNU syntax.
//
//===----------------------------------------------------------------------===//

#include "SuperHInstPrinter.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#include "SuperHGenAsmWriter.inc"

void SuperHInstPrinter::printRegName(raw_ostream &OS, MCRegister Reg) {
  OS << getRegisterName(Reg);
}

void SuperHInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                  StringRef Annot, const MCSubtargetInfo &STI,
                                  raw_ostream &OS) {
  printInstruction(MI, Address, OS);
  printAnnotation(OS, Annot);
}

void SuperHInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &OS) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    printRegName(OS, Op.getReg());
    return;
  }
  if (Op.isImm()) {
    OS << Op.getImm();
    return;
  }
  assert(Op.isExpr() && "unknown operand kind");
  MAI.printExpr(OS, *Op.getExpr());
}

void SuperHInstPrinter::printImm(const MCInst *MI, unsigned OpNo,
                                 raw_ostream &OS) {
  OS << '#';
  printOperand(MI, OpNo, OS);
}

void SuperHInstPrinter::printDisp(const MCInst *MI, unsigned OpNo,
                                  raw_ostream &OS) {
  printOperand(MI, OpNo, OS);
}

void SuperHInstPrinter::printBranchTarget(const MCInst *MI, uint64_t Address,
                                          unsigned OpNo, raw_ostream &OS) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (!Op.isImm()) {
    printOperand(MI, OpNo, OS);
    return;
  }
  if (PrintBranchImmAsAddress)
    OS << formatHex(Address + 4 + Op.getImm());
  else
    OS << ".+" << Op.getImm() + 4;
}

template <unsigned Scale>
void SuperHInstPrinter::printPCRelLoad(const MCInst *MI, uint64_t Address,
                                       unsigned OpNo, raw_ostream &OS) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (!Op.isImm()) {
    printOperand(MI, OpNo, OS);
    return;
  }
  OS << "@(" << Op.getImm() << ",pc)";
  if (PrintBranchImmAsAddress) {
    uint64_t Base = Scale == 4 ? (Address & ~3ULL) : Address;
    OS << " ! " << formatHex(Base + 4 + Op.getImm());
  }
}
