//===-- SuperHRegisterInfo.cpp - SuperH Register Information --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SuperH implementation of the TargetRegisterInfo
// class.
//
//===----------------------------------------------------------------------===//

#include "SuperHRegisterInfo.h"
#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "SuperHFrameLowering.h"
#include "SuperHInstrInfo.h"
#include "SuperHSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "sh-reginfo"

#define GET_REGINFO_TARGET_DESC
#include "SuperHGenRegisterInfo.inc"

SuperHRegisterInfo::SuperHRegisterInfo(const SuperHSubtarget &STI)
    : SuperHGenRegisterInfo(SH::PR, /*DwarfFlavour=*/0, /*EHFlavor=*/0,
                            /*PC=*/SH::PC) {}

const MCPhysReg *
SuperHRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SH_SaveList;
}

const uint32_t *
SuperHRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                         CallingConv::ID CC) const {
  return CSR_SH_RegMask;
}

const uint32_t *SuperHRegisterInfo::getNoPreservedMask() const {
  return CSR_NoRegs_RegMask;
}

BitVector SuperHRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  for (MCPhysReg Reg : {SH::R15, SH::PC, SH::GBR, SH::VBR, SH::SSR,
                        SH::SPC, SH::SGR, SH::DBR, SH::FPSCR, SH::FPUL})
    Reserved.set(Reg);
  if (getFrameLowering(MF)->hasFP(MF))
    Reserved.set(SH::R14);
  return Reserved;
}

Register SuperHRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return getFrameLowering(MF)->hasFP(MF) ? SH::R14 : SH::R15;
}

// Lets the register allocator relax R0-only values whose instructions don't
// all need R0.
const TargetRegisterClass *
SuperHRegisterInfo::getLargestLegalSuperClass(const TargetRegisterClass *RC,
                                              const MachineFunction &MF) const {
  if (RC == &SH::R0CRegClass)
    return &SH::GPRRegClass;
  return RC;
}

const TargetRegisterClass *
SuperHRegisterInfo::getCrossCopyRegClass(const TargetRegisterClass *RC) const {
  if (RC == &SH::CTRLRegClass || RC == &SH::SYSRegClass)
    return &SH::GPRRegClass;
  return RC;
}

// Joining a general register with one that must be R0 can leave an
// instruction with two operands that both need R0, so keep such copies.
bool SuperHRegisterInfo::shouldCoalesce(
    MachineInstr *MI, const TargetRegisterClass *SrcRC, unsigned SubReg,
    const TargetRegisterClass *DstRC, unsigned DstSubReg,
    const TargetRegisterClass *NewRC, LiveIntervals &LIS) const {
  return NewRC != &SH::R0CRegClass || (SrcRC == NewRC && DstRC == NewRC);
}

namespace {

/// Expands frame accesses whose offset doesn't fit the instruction.
class FrameAccessExpander {
  MachineInstr &MI;
  MachineBasicBlock &MBB;
  const SuperHInstrInfo &TII;
  DebugLoc DL;
  Register Base;
  int64_t Offset;

public:
  FrameAccessExpander(MachineInstr &MI, const SuperHInstrInfo &TII,
                      Register Base, int64_t Offset)
      : MI(MI), MBB(*MI.getParent()), TII(TII), DL(MI.getDebugLoc()),
        Base(Base), Offset(Offset) {}

  MachineInstrBuilder build(unsigned Opcode) {
    return BuildMI(MBB, MI, DL, TII.get(Opcode));
  }
  MachineInstrBuilder build(unsigned Opcode, Register Dest) {
    return BuildMI(MBB, MI, DL, TII.get(Opcode), Dest);
  }

  void copyMemOperands(MachineInstrBuilder &Builder) {
    Builder.cloneMemRefs(MI);
  }

  // Reg = Base + Offset.
  void computeAddress(Register Reg, int64_t Adjust = 0) {
    TII.loadImmediate(MBB, MI, DL, Reg, Offset + Adjust);
    build(SH::ADD, Reg).addReg(Reg).addReg(Base);
  }

  void expandLoad(unsigned DispOpcode, unsigned IndexOpcode,
                  unsigned PlainOpcode, int64_t Scale, bool DispNeedsR0) {
    Register Dest = MI.getOperand(0).getReg();
    if (Offset >= 0 && Offset <= 15 * Scale && Offset % Scale == 0 &&
        (!DispNeedsR0 || Dest == SH::R0)) {
      MachineInstrBuilder Builder = build(DispOpcode, Dest)
                                        .addReg(Base)
                                        .addImm(Offset);
      copyMemOperands(Builder);
      return;
    }
    if (Dest == SH::R0) {
      TII.loadImmediate(MBB, MI, DL, SH::R0, Offset);
      MachineInstrBuilder Builder =
          build(IndexOpcode, Dest).addReg(SH::R0).addReg(Base);
      copyMemOperands(Builder);
      return;
    }
    computeAddress(Dest);
    MachineInstrBuilder Builder =
        build(PlainOpcode, Dest).addReg(Dest, RegState::Kill);
    copyMemOperands(Builder);
  }

  void expandStore(unsigned DispOpcode, unsigned IndexOpcode,
                   unsigned PlainOpcode, int64_t Scale, bool DispNeedsR0,
                   RegScavenger *RS, int SPAdj) {
    MachineOperand &Source = MI.getOperand(0);
    Register Value = Source.getReg();
    bool Kill = Source.isKill();
    if (Offset >= 0 && Offset <= 15 * Scale && Offset % Scale == 0 &&
        (!DispNeedsR0 || Value == SH::R0)) {
      MachineInstrBuilder Builder = build(DispOpcode)
                                        .addReg(Value, getKillRegState(Kill))
                                        .addReg(Base)
                                        .addImm(Offset);
      copyMemOperands(Builder);
      return;
    }

    Register Scratch;
    if (RS)
      Scratch = RS->scavengeRegisterBackwards(SH::GPRRegClass, MI.getIterator(),
                                              /*RestoreAfter=*/false, SPAdj,
                                              /*AllowSpill=*/false);

    bool Pushed = false;
    if (!Scratch || Scratch == Value) {
      // No free register: borrow one through the stack.
      Scratch = Value == SH::R0 ? SH::R1 : SH::R0;
      build(SH::STLP, SH::R15)
          .addReg(Scratch, RegState::Undef)
          .addReg(SH::R15);
      Pushed = true;
    }
    int64_t Adjust = Pushed && Base == SH::R15 ? 4 : 0;

    if (Scratch == SH::R0) {
      TII.loadImmediate(MBB, MI, DL, SH::R0, Offset + Adjust);
      MachineInstrBuilder Builder = build(IndexOpcode)
                                        .addReg(SH::R0, RegState::Kill)
                                        .addReg(Value, getKillRegState(Kill))
                                        .addReg(Base);
      copyMemOperands(Builder);
    } else {
      computeAddress(Scratch, Adjust);
      MachineInstrBuilder Builder = build(PlainOpcode)
                                        .addReg(Value, getKillRegState(Kill))
                                        .addReg(Scratch, RegState::Kill);
      copyMemOperands(Builder);
    }

    if (Pushed)
      build(SH::LDLP, Scratch).addReg(SH::R15, RegState::Define).addReg(SH::R15);
  }

  void expandAddress() {
    Register Dest = MI.getOperand(0).getReg();
    if (isInt<8>(Offset)) {
      build(SH::MOV, Dest).addReg(Base);
      if (Offset)
        build(SH::ADDI, Dest).addReg(Dest).addImm(Offset);
      return;
    }
    computeAddress(Dest);
  }
};

} // end anonymous namespace

bool SuperHRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                             int SPAdj, unsigned FIOperandNum,
                                             RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getMF();
  const SuperHSubtarget &STI = MF.getSubtarget<SuperHSubtarget>();
  const SuperHInstrInfo &TII = *STI.getInstrInfo();
  const TargetFrameLowering *TFI = STI.getFrameLowering();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  Register Base;
  int64_t Offset =
      TFI->getFrameIndexReference(MF, FrameIndex, Base).getFixed() + SPAdj;

  if (MI.isDebugValue()) {
    MI.getOperand(FIOperandNum).ChangeToRegister(Base, /*isDef=*/false);
    const DIExpression *Expr = MI.getDebugExpression();
    Expr = DIExpression::prepend(Expr, DIExpression::ApplyOffset, Offset);
    MI.getDebugExpressionOp().setMetadata(Expr);
    return false;
  }

  Offset += MI.getOperand(FIOperandNum + 1).getImm();
  FrameAccessExpander Expander(MI, TII, Base, Offset);

  switch (MI.getOpcode()) {
  case SH::LDL_FI:
    Expander.expandLoad(SH::LDLD, SH::LDLX, SH::LDL, 4, false);
    break;
  case SH::LDW_FI:
    Expander.expandLoad(SH::LDWD, SH::LDWX, SH::LDW, 2, true);
    break;
  case SH::LDB_FI:
    Expander.expandLoad(SH::LDBD, SH::LDBX, SH::LDB, 1, true);
    break;
  case SH::STL_FI:
    Expander.expandStore(SH::STLD, SH::STLX, SH::STL, 4, false, RS, SPAdj);
    break;
  case SH::STW_FI:
    Expander.expandStore(SH::STWD, SH::STWX, SH::STW, 2, true, RS, SPAdj);
    break;
  case SH::STB_FI:
    Expander.expandStore(SH::STBD, SH::STBX, SH::STB, 1, true, RS, SPAdj);
    break;
  case SH::FRAMEADDR:
    Expander.expandAddress();
    break;
  default:
    report_fatal_error("SuperH: unexpected frame index user");
  }

  MI.eraseFromParent();
  return true;
}
