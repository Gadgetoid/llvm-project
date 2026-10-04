//===-- SuperHAsmPrinter.cpp - SuperH LLVM assembly printer ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format SuperH assembly language.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SuperHInstPrinter.h"
#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "SuperH.h"
#include "SuperHMCInstLower.h"
#include "TargetInfo/SuperHTargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

#define DEBUG_TYPE "sh-asm-printer"

namespace {

class SuperHAsmPrinter : public AsmPrinter {
public:
  static char ID;

  explicit SuperHAsmPrinter(TargetMachine &TM,
                            std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer), ID) {}

  StringRef getPassName() const override { return "SuperH Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override;

  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       const char *ExtraCode, raw_ostream &OS) override;
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             const char *ExtraCode, raw_ostream &OS) override;

private:
  void emitSingle(const MachineInstr &MI);
};

} // end anonymous namespace

void SuperHAsmPrinter::emitSingle(const MachineInstr &MI) {
  SuperHMCInstLower Lower(OutContext, *this);
  if (MI.getOpcode() == SH::LITERAL) {
    OutStreamer->emitLabel(MI.getOperand(0).getMCSymbol());
    OutStreamer->emitValue(Lower.lowerExpression(MI.getOperand(1)), 4);
    return;
  }
  MCInst Inst;
  Lower.lowerInstruction(MI, Inst);
  EmitToStreamer(*OutStreamer, Inst);
}

void SuperHAsmPrinter::emitInstruction(const MachineInstr *MI) {
  if (!MI->isBundle()) {
    emitSingle(*MI);
    return;
  }
  MachineBasicBlock::const_instr_iterator I = MI->getIterator();
  MachineBasicBlock::const_instr_iterator E = MI->getParent()->instr_end();
  for (++I; I != E && I->isInsideBundle(); ++I)
    emitSingle(*I);
}

bool SuperHAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                       const char *ExtraCode, raw_ostream &OS) {
  if (!AsmPrinter::PrintAsmOperand(MI, OpNo, ExtraCode, OS))
    return false;
  if (ExtraCode && ExtraCode[0])
    return true;
  const MachineOperand &MO = MI->getOperand(OpNo);
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    OS << SuperHInstPrinter::getRegisterName(MO.getReg());
    return false;
  case MachineOperand::MO_Immediate:
    OS << MO.getImm();
    return false;
  case MachineOperand::MO_GlobalAddress:
    PrintSymbolOperand(MO, OS);
    return false;
  default:
    return true;
  }
}

bool SuperHAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                             unsigned OpNo,
                                             const char *ExtraCode,
                                             raw_ostream &OS) {
  if (ExtraCode && ExtraCode[0])
    return true;
  const MachineOperand &MO = MI->getOperand(OpNo);
  if (!MO.isReg())
    return true;
  OS << '@' << SuperHInstPrinter::getRegisterName(MO.getReg());
  return false;
}

char SuperHAsmPrinter::ID = 0;

INITIALIZE_PASS(SuperHAsmPrinter, "sh-asm-printer", "SuperH Assembly Printer",
                false, false)

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeSuperHAsmPrinter() {
  RegisterAsmPrinter<SuperHAsmPrinter> X(getTheSuperHTarget());
  RegisterAsmPrinter<SuperHAsmPrinter> Y(getTheSuperHLETarget());
}
