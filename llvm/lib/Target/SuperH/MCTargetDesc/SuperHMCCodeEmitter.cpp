//===-- SuperHMCCodeEmitter.cpp - Convert SuperH code to machine code -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SuperHMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SuperHFixupKinds.h"
#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");

namespace {

class SuperHMCCodeEmitter : public MCCodeEmitter {
  const MCInstrInfo &MCII;
  MCContext &Ctx;

public:
  SuperHMCCodeEmitter(const MCInstrInfo &MCII, MCContext &Ctx)
      : MCII(MCII), Ctx(Ctx) {}

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getImm8OpValue(const MCInst &MI, unsigned OpNo,
                          SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;

  template <unsigned Scale>
  unsigned getDispOpValue(const MCInst &MI, unsigned OpNo,
                          SmallVectorImpl<MCFixup> &Fixups,
                          const MCSubtargetInfo &STI) const;

  unsigned getBranch8OpValue(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const {
    return getPCRelValue(MI, OpNo, Fixups, SH::fixup_sh_branch8, 2);
  }
  unsigned getBranch12OpValue(const MCInst &MI, unsigned OpNo,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &STI) const {
    return getPCRelValue(MI, OpNo, Fixups, SH::fixup_sh_branch12, 2);
  }
  unsigned getPCRel8x2OpValue(const MCInst &MI, unsigned OpNo,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &STI) const {
    return getPCRelValue(MI, OpNo, Fixups, SH::fixup_sh_pcrel8x2, 2);
  }
  unsigned getPCRel8x4OpValue(const MCInst &MI, unsigned OpNo,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &STI) const {
    return getPCRelValue(MI, OpNo, Fixups, SH::fixup_sh_pcrel8x4, 4);
  }

private:
  unsigned getPCRelValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups, unsigned Kind,
                         unsigned Scale) const;
};

} // end anonymous namespace

void SuperHMCCodeEmitter::encodeInstruction(const MCInst &MI,
                                            SmallVectorImpl<char> &CB,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  uint16_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
  support::endian::write<uint16_t>(CB, Bits,
                                   Ctx.getAsmInfo().isLittleEndian()
                                       ? llvm::endianness::little
                                       : llvm::endianness::big);
  ++MCNumEmitted;
}

unsigned
SuperHMCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
  if (MO.isImm())
    return MO.getImm();
  report_fatal_error("SuperH: unexpected symbolic operand");
}

unsigned SuperHMCCodeEmitter::getImm8OpValue(const MCInst &MI, unsigned OpNo,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isImm())
    return MO.getImm() & 0xFF;
  int64_t Value;
  if (MO.getExpr()->evaluateAsAbsolute(Value))
    return Value & 0xFF;
  Ctx.reportError(SMLoc(), "SuperH: immediate must be a constant");
  return 0;
}

template <unsigned Scale>
unsigned SuperHMCCodeEmitter::getDispOpValue(const MCInst &MI, unsigned OpNo,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  int64_t Value = 0;
  if (MO.isImm())
    Value = MO.getImm();
  else if (!MO.getExpr()->evaluateAsAbsolute(Value))
    Ctx.reportError(SMLoc(), "SuperH: displacement must be a constant");
  return Value / Scale;
}

unsigned SuperHMCCodeEmitter::getPCRelValue(const MCInst &MI, unsigned OpNo,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            unsigned Kind,
                                            unsigned Scale) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isImm())
    return MO.getImm() / Scale;
  Fixups.push_back(MCFixup::create(0, MO.getExpr(), MCFixupKind(Kind),
                                   /*PCRel=*/true));
  return 0;
}

#include "SuperHGenMCCodeEmitter.inc"

MCCodeEmitter *llvm::createSuperHMCCodeEmitter(const MCInstrInfo &MCII,
                                               MCContext &Ctx) {
  return new SuperHMCCodeEmitter(MCII, Ctx);
}
