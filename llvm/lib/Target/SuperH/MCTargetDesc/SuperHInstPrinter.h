//===-- SuperHInstPrinter.h - Convert SuperH MCInst to assembly -*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SUPERH_MCTARGETDESC_SUPERHINSTPRINTER_H
#define LLVM_LIB_TARGET_SUPERH_MCTARGETDESC_SUPERHINSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"

namespace llvm {

class SuperHInstPrinter : public MCInstPrinter {
public:
  SuperHInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                    const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  void printRegName(raw_ostream &OS, MCRegister Reg) override;
  void printInst(const MCInst *MI, uint64_t Address, StringRef Annot,
                 const MCSubtargetInfo &STI, raw_ostream &OS) override;

  std::pair<const char *, uint64_t>
  getMnemonic(const MCInst &MI) const override;
  void printInstruction(const MCInst *MI, uint64_t Address, raw_ostream &OS);
  static const char *getRegisterName(MCRegister Reg);

  void printOperand(const MCInst *MI, unsigned OpNo, raw_ostream &OS);
  void printImm(const MCInst *MI, unsigned OpNo, raw_ostream &OS);
  void printDisp(const MCInst *MI, unsigned OpNo, raw_ostream &OS);
  void printBranchTarget(const MCInst *MI, uint64_t Address, unsigned OpNo,
                         raw_ostream &OS);
  template <unsigned Scale>
  void printPCRelLoad(const MCInst *MI, uint64_t Address, unsigned OpNo,
                      raw_ostream &OS);
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SUPERH_MCTARGETDESC_SUPERHINSTPRINTER_H
