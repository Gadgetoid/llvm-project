//===-- SuperHDisassembler.cpp - Disassembler for SuperH ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the SuperH disassembler for the SH-1 to SH-3 integer
// instruction set.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "TargetInfo/SuperHTargetInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDecoder.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;
using namespace llvm::MCD;

#define DEBUG_TYPE "sh-disassembler"

using DecodeStatus = MCDisassembler::DecodeStatus;

namespace {

class SuperHDisassembler : public MCDisassembler {
  bool IsLittleEndian;

public:
  SuperHDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx,
                     bool IsLittleEndian)
      : MCDisassembler(STI, Ctx), IsLittleEndian(IsLittleEndian) {}

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};

} // end anonymous namespace

static const MCPhysReg GPRDecoderTable[] = {
    SH::R0, SH::R1, SH::R2,  SH::R3,  SH::R4,  SH::R5,  SH::R6,  SH::R7,
    SH::R8, SH::R9, SH::R10, SH::R11, SH::R12, SH::R13, SH::R14, SH::R15};

static DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t Address,
                                           const MCDisassembler *Decoder) {
  if (RegNo > 15)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[RegNo]));
  return MCDisassembler::Success;
}

// R0 operands of the instructions that only work with R0 have no encoding.
static DecodeStatus DecodeR0CRegisterClass(MCInst &Inst,
                                           const MCDisassembler *Decoder) {
  Inst.addOperand(MCOperand::createReg(SH::R0));
  return MCDisassembler::Success;
}

static DecodeStatus decodeSImm8(MCInst &Inst, unsigned Value, uint64_t Address,
                                const MCDisassembler *Decoder) {
  Inst.addOperand(MCOperand::createImm(SignExtend64<8>(Value)));
  return MCDisassembler::Success;
}

template <unsigned Scale>
static DecodeStatus decodeDisp(MCInst &Inst, unsigned Value, uint64_t Address,
                               const MCDisassembler *Decoder) {
  Inst.addOperand(MCOperand::createImm(int64_t(Value) * Scale));
  return MCDisassembler::Success;
}

template <unsigned Bits>
static DecodeStatus decodeBranch(MCInst &Inst, unsigned Value,
                                 uint64_t Address,
                                 const MCDisassembler *Decoder) {
  Inst.addOperand(MCOperand::createImm(SignExtend64<Bits>(Value) * 2));
  return MCDisassembler::Success;
}

#include "SuperHGenDisassemblerTables.inc"

DecodeStatus SuperHDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                                ArrayRef<uint8_t> Bytes,
                                                uint64_t Address,
                                                raw_ostream &CStream) const {
  if (Bytes.size() < 2) {
    Size = 0;
    return MCDisassembler::Fail;
  }
  Size = 2;
  uint16_t Insn = IsLittleEndian ? support::endian::read16le(Bytes.data())
                                 : support::endian::read16be(Bytes.data());
  return decodeInstruction(DecoderTable16, Instr, Insn, Address, this, STI);
}

static MCDisassembler *createSuperHDisassembler(const Target &T,
                                                const MCSubtargetInfo &STI,
                                                MCContext &Ctx) {
  return new SuperHDisassembler(STI, Ctx, /*IsLittleEndian=*/false);
}

static MCDisassembler *createSuperHLEDisassembler(const Target &T,
                                                  const MCSubtargetInfo &STI,
                                                  MCContext &Ctx) {
  return new SuperHDisassembler(STI, Ctx, /*IsLittleEndian=*/true);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeSuperHDisassembler() {
  TargetRegistry::RegisterMCDisassembler(getTheSuperHTarget(),
                                         createSuperHDisassembler);
  TargetRegistry::RegisterMCDisassembler(getTheSuperHLETarget(),
                                         createSuperHLEDisassembler);
}
