//===-- SuperHInstrInfo.cpp - SuperH Instruction Information --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SuperH implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "SuperHInstrInfo.h"
#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "SuperHSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/MC/MCInstBuilder.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "sh-instrinfo"

#define GET_INSTRINFO_CTOR_DTOR
#include "SuperHGenInstrInfo.inc"

SuperHInstrInfo::SuperHInstrInfo(const SuperHSubtarget &STI)
    : SuperHGenInstrInfo(STI, RI, SH::ADJCALLSTACKDOWN, SH::ADJCALLSTACKUP),
      RI(STI) {}

void SuperHInstrInfo::anchor() {}

unsigned SuperHInstrInfo::getShiftCost(int64_t Amount, ShiftKind Kind) {
  if (Amount < 0 || Amount > 31)
    return ~0U;
  if (Kind == ShiftArithmeticRight)
    return Amount;
  return Amount / 16 + (Amount % 16) / 8 + (Amount % 8) / 2 + Amount % 2;
}

bool SuperHInstrInfo::isDelaySlotCandidate(const MachineInstr &MI) const {
  if (MI.isBundle() || MI.isTerminator() || MI.isCall() ||
      MI.hasDelaySlot() || MI.isInlineAsm() || MI.isPseudo() ||
      MI.isPosition() || MI.isDebugInstr() || MI.isImplicitDef() ||
      MI.isKill())
    return false;
  if (MI.getDesc().TSFlags & SHII::DelayIllegal)
    return false;
  return getInstSizeInBytes(MI) == 2;
}

unsigned SuperHInstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  if (MI.isBundle()) {
    unsigned Size = 0;
    MachineBasicBlock::const_instr_iterator I = MI.getIterator();
    MachineBasicBlock::const_instr_iterator E = MI.getParent()->instr_end();
    while (++I != E && I->isInsideBundle())
      Size += getInstSizeInBytes(*I);
    return Size;
  }
  if (MI.isInlineAsm()) {
    const MachineFunction &MF = *MI.getMF();
    return getInlineAsmLength(MI.getOperand(0).getSymbolName(),
                              MF.getTarget().getMCAsmInfo(),
                              &MF.getSubtarget());
  }
  return MI.getDesc().getSize();
}

MCInst SuperHInstrInfo::getNop() const {
  return MCInstBuilder(SH::NOP);
}

void SuperHInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MI,
                                  const DebugLoc &DL, Register DestReg,
                                  Register SrcReg, bool KillSrc,
                                  bool RenamableDest, bool RenamableSrc) const {
  bool DestGPR = SH::GPRRegClass.contains(DestReg);
  bool SrcGPR = SH::GPRRegClass.contains(SrcReg);

  if (DestGPR && SrcGPR) {
    BuildMI(MBB, MI, DL, get(SH::MOV), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
    return;
  }

  if (DestGPR) {
    unsigned Opcode;
    switch (SrcReg) {
    case SH::PR:
      Opcode = SH::STPR;
      break;
    case SH::MACLO:
      Opcode = SH::STMACL;
      break;
    case SH::MACHI:
      Opcode = SH::STMACH;
      break;
    case SH::GBR:
      Opcode = SH::STGBR;
      break;
    case SH::SR:
      BuildMI(MBB, MI, DL, get(SH::MOVT), DestReg);
      return;
    default:
      llvm_unreachable("cannot copy from this register");
    }
    BuildMI(MBB, MI, DL, get(Opcode), DestReg);
    return;
  }

  if (SrcGPR) {
    unsigned Opcode;
    switch (DestReg) {
    case SH::PR:
      Opcode = SH::LDPR;
      break;
    case SH::MACLO:
      Opcode = SH::LDMACL;
      break;
    case SH::MACHI:
      Opcode = SH::LDMACH;
      break;
    case SH::GBR:
      Opcode = SH::LDGBR;
      break;
    case SH::SR:
      // The copy only carries T, which holds 0 or 1.
      BuildMI(MBB, MI, DL, get(SH::CMPPL))
          .addReg(SrcReg, getKillRegState(KillSrc));
      return;
    default:
      llvm_unreachable("cannot copy to this register");
    }
    BuildMI(MBB, MI, DL, get(Opcode)).addReg(SrcReg, getKillRegState(KillSrc));
    return;
  }

  llvm_unreachable("unsupported physical register copy");
}

static MachineMemOperand *getFrameMemOperand(MachineBasicBlock &MBB,
                                             int FrameIndex,
                                             MachineMemOperand::Flags Flags) {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FrameIndex), Flags,
      MFI.getObjectSize(FrameIndex), MFI.getObjectAlign(FrameIndex));
}

void SuperHInstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
    bool IsKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
    MachineInstr::MIFlag Flags) const {
  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();
  BuildMI(MBB, MI, DL, get(SH::STL_FI))
      .addReg(SrcReg, getKillRegState(IsKill))
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addMemOperand(
          getFrameMemOperand(MBB, FrameIndex, MachineMemOperand::MOStore))
      .setMIFlag(Flags);
}

void SuperHInstrInfo::loadRegFromStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register DestReg,
    int FrameIndex, const TargetRegisterClass *RC, Register VReg,
    unsigned SubReg, MachineInstr::MIFlag Flags) const {
  DebugLoc DL = MI != MBB.end() ? MI->getDebugLoc() : DebugLoc();
  BuildMI(MBB, MI, DL, get(SH::LDL_FI), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0)
      .addMemOperand(
          getFrameMemOperand(MBB, FrameIndex, MachineMemOperand::MOLoad))
      .setMIFlag(Flags);
}

Register SuperHInstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                              int &FrameIndex) const {
  if (MI.getOpcode() == SH::LDL_FI && MI.getOperand(1).isFI() &&
      MI.getOperand(2).isImm() && MI.getOperand(2).getImm() == 0) {
    FrameIndex = MI.getOperand(1).getIndex();
    return MI.getOperand(0).getReg();
  }
  return Register();
}

Register SuperHInstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                             int &FrameIndex) const {
  if (MI.getOpcode() == SH::STL_FI && MI.getOperand(1).isFI() &&
      MI.getOperand(2).isImm() && MI.getOperand(2).getImm() == 0) {
    FrameIndex = MI.getOperand(1).getIndex();
    return MI.getOperand(0).getReg();
  }
  return Register();
}

void SuperHInstrInfo::loadImmediate(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MI,
                                    const DebugLoc &DL, Register Reg,
                                    int64_t Value,
                                    MachineInstr::MIFlag Flag) const {
  if (isInt<8>(Value)) {
    BuildMI(MBB, MI, DL, get(SH::MOVI), Reg).addImm(Value).setMIFlag(Flag);
    return;
  }
  BuildMI(MBB, MI, DL, get(SH::LOADCONST), Reg)
      .addImm(SignExtend64<32>(Value))
      .setMIFlag(Flag);
}

void SuperHInstrInfo::adjustRegister(MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator MI,
                                     const DebugLoc &DL, Register Reg,
                                     int64_t Amount, Register Scratch,
                                     MachineInstr::MIFlag Flag) const {
  if (Amount == 0)
    return;
  if (isInt<8>(Amount) || !Scratch) {
    while (Amount != 0) {
      int64_t Step = std::clamp<int64_t>(Amount, -128, 127);
      BuildMI(MBB, MI, DL, get(SH::ADDI), Reg)
          .addReg(Reg)
          .addImm(Step)
          .setMIFlag(Flag);
      Amount -= Step;
    }
    return;
  }
  loadImmediate(MBB, MI, DL, Scratch, Amount, Flag);
  BuildMI(MBB, MI, DL, get(SH::ADD), Reg)
      .addReg(Reg)
      .addReg(Scratch, RegState::Kill)
      .setMIFlag(Flag);
}

void SuperHInstrInfo::expandShift(MachineInstr &MI, ShiftKind Kind) const {
  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();
  Register Reg = MI.getOperand(0).getReg();
  int64_t Amount = MI.getOperand(2).getImm();

  auto emit = [&](unsigned Opcode) {
    BuildMI(MBB, MI, DL, get(Opcode), Reg).addReg(Reg);
  };

  if (Kind == ShiftArithmeticRight) {
    for (int64_t I = 0; I < Amount; ++I)
      emit(SH::SHAR);
    return;
  }

  bool Left = Kind == ShiftLeft;
  for (; Amount >= 16; Amount -= 16)
    emit(Left ? SH::SHLL16 : SH::SHLR16);
  for (; Amount >= 8; Amount -= 8)
    emit(Left ? SH::SHLL8 : SH::SHLR8);
  for (; Amount >= 2; Amount -= 2)
    emit(Left ? SH::SHLL2 : SH::SHLR2);
  if (Amount)
    emit(Left ? SH::SHLL : SH::SHLR);
}

bool SuperHInstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  MachineBasicBlock &MBB = *MI.getParent();
  DebugLoc DL = MI.getDebugLoc();

  switch (MI.getOpcode()) {
  default:
    return false;

  case SH::MUL32:
    BuildMI(MBB, MI, DL, get(SH::MULL))
        .add(MI.getOperand(1))
        .add(MI.getOperand(2));
    BuildMI(MBB, MI, DL, get(SH::STMACL), MI.getOperand(0).getReg());
    break;

  case SH::MULHS32:
  case SH::MULHU32:
    BuildMI(MBB, MI, DL,
            get(MI.getOpcode() == SH::MULHS32 ? SH::DMULSL : SH::DMULUL))
        .add(MI.getOperand(1))
        .add(MI.getOperand(2));
    BuildMI(MBB, MI, DL, get(SH::STMACH), MI.getOperand(0).getReg());
    break;

  case SH::SMULLOHI:
  case SH::UMULLOHI:
    BuildMI(MBB, MI, DL,
            get(MI.getOpcode() == SH::SMULLOHI ? SH::DMULSL : SH::DMULUL))
        .add(MI.getOperand(2))
        .add(MI.getOperand(3));
    BuildMI(MBB, MI, DL, get(SH::STMACL), MI.getOperand(0).getReg());
    BuildMI(MBB, MI, DL, get(SH::STMACH), MI.getOperand(1).getReg());
    break;

  case SH::SHLLI:
    expandShift(MI, ShiftLeft);
    break;
  case SH::SHLRI:
    expandShift(MI, ShiftRight);
    break;
  case SH::SHARI:
    expandShift(MI, ShiftArithmeticRight);
    break;
  }

  MI.eraseFromParent();
  return true;
}

//===----------------------------------------------------------------------===//
// Branch analysis
//===----------------------------------------------------------------------===//

static bool isConditionalBranch(unsigned Opcode) {
  return Opcode == SH::PseudoBT || Opcode == SH::PseudoBF;
}

bool SuperHInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *&TBB,
                                    MachineBasicBlock *&FBB,
                                    SmallVectorImpl<MachineOperand> &Cond,
                                    bool AllowModify) const {
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end() || !isUnpredicatedTerminator(*I))
    return false;

  MachineInstr *Last = &*I;
  MachineInstr *Previous = nullptr;
  MachineBasicBlock::iterator PI = I;
  while (PI != MBB.begin()) {
    --PI;
    if (PI->isDebugInstr())
      continue;
    if (isUnpredicatedTerminator(*PI))
      Previous = &*PI;
    break;
  }

  if (Previous) {
    MachineBasicBlock::iterator Before = Previous->getIterator();
    while (Before != MBB.begin()) {
      --Before;
      if (Before->isDebugInstr())
        continue;
      if (isUnpredicatedTerminator(*Before))
        return true;
      break;
    }
  }

  unsigned LastOpcode = Last->getOpcode();
  if (!Previous) {
    if (LastOpcode == SH::PseudoBR) {
      TBB = Last->getOperand(0).getMBB();
      return false;
    }
    if (isConditionalBranch(LastOpcode)) {
      TBB = Last->getOperand(0).getMBB();
      Cond.push_back(MachineOperand::CreateImm(LastOpcode));
      return false;
    }
    return true;
  }

  unsigned PreviousOpcode = Previous->getOpcode();
  if (isConditionalBranch(PreviousOpcode) && LastOpcode == SH::PseudoBR) {
    TBB = Previous->getOperand(0).getMBB();
    Cond.push_back(MachineOperand::CreateImm(PreviousOpcode));
    FBB = Last->getOperand(0).getMBB();
    return false;
  }

  if (PreviousOpcode == SH::PseudoBR && LastOpcode == SH::PseudoBR) {
    TBB = Previous->getOperand(0).getMBB();
    if (AllowModify)
      Last->eraseFromParent();
    return false;
  }

  return true;
}

unsigned SuperHInstrInfo::insertBranch(MachineBasicBlock &MBB,
                                       MachineBasicBlock *TBB,
                                       MachineBasicBlock *FBB,
                                       ArrayRef<MachineOperand> Cond,
                                       const DebugLoc &DL,
                                       int *BytesAdded) const {
  assert(TBB && "insertBranch must not be told to insert a fallthrough");
  assert(Cond.size() <= 1 && "SuperH branch conditions have one operand");

  if (Cond.empty()) {
    BuildMI(&MBB, DL, get(SH::PseudoBR)).addMBB(TBB);
    if (BytesAdded)
      *BytesAdded = 4;
    return 1;
  }

  BuildMI(&MBB, DL, get(Cond[0].getImm())).addMBB(TBB);
  if (!FBB) {
    if (BytesAdded)
      *BytesAdded = 2;
    return 1;
  }
  BuildMI(&MBB, DL, get(SH::PseudoBR)).addMBB(FBB);
  if (BytesAdded)
    *BytesAdded = 6;
  return 2;
}

unsigned SuperHInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                       int *BytesRemoved) const {
  unsigned Count = 0;
  int Bytes = 0;
  MachineBasicBlock::iterator I = MBB.end();
  while (I != MBB.begin()) {
    --I;
    if (I->isDebugInstr())
      continue;
    unsigned Opcode = I->getOpcode();
    if (Opcode != SH::PseudoBR && !isConditionalBranch(Opcode))
      break;
    Bytes += Opcode == SH::PseudoBR ? 4 : 2;
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }
  if (BytesRemoved)
    *BytesRemoved = Bytes;
  return Count;
}

bool SuperHInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 1 && "invalid SuperH branch condition");
  Cond[0].setImm(Cond[0].getImm() == SH::PseudoBT ? SH::PseudoBF
                                                  : SH::PseudoBT);
  return false;
}

MachineBasicBlock *
SuperHInstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  return MI.getOperand(0).getMBB();
}
