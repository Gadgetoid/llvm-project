//===- SuperHFrameLowering.cpp - SuperH Frame Information -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the SuperH implementation of TargetFrameLowering.
//
// The frame follows the Windows CE SH-3 calling standard, which the GNU
// convention is compatible with:
//
//   incoming stack arguments          sp_entry + 16 ...  (sp_entry + 0 GNU)
//   home area for R4-R7 (Windows CE)  sp_entry + 0 .. 15
//   ------------------------------------------------ sp_entry
//   unnamed argument registers of a GNU variadic function
//   register save area: R14, R8-R13, PR, pushed in that order
//   locals and spill slots
//   outgoing argument area            sp + 0 ...
//   ------------------------------------------------ sp (and R14 when used)
//
// The prologue is the sequence the standard's virtual unwinder expects:
//
//   mov.l r14,@-r15 ; mov.l r8,@-r15 ; ... ; sts.l pr,@-r15
//   add #-n,r15 (or a literal and add) ; mov r15,r14
//
//===----------------------------------------------------------------------===//

#include "SuperHFrameLowering.h"
#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "SuperHInstrInfo.h"
#include "SuperHMachineFunctionInfo.h"
#include "SuperHSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDwarf.h"

using namespace llvm;

#define DEBUG_TYPE "sh-framelowering"

bool SuperHFrameLowering::hasFPImpl(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.disableFramePointerElim() ||
         MFI.hasVarSizedObjects() || MFI.isFrameAddressTaken();
}

bool SuperHFrameLowering::hasReservedCallFrame(
    const MachineFunction &MF) const {
  return !MF.getFrameInfo().hasVarSizedObjects();
}

MachineBasicBlock::iterator SuperHFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  if (!hasReservedCallFrame(MF)) {
    const SuperHInstrInfo &TII = *STI.getInstrInfo();
    int64_t Amount = I->getOperand(0).getImm();
    if (I->getOpcode() == SH::ADJCALLSTACKDOWN)
      Amount = -Amount;
    TII.adjustRegister(MBB, I, I->getDebugLoc(), SH::R15, Amount, Register());
  }
  return MBB.erase(I);
}

void SuperHFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                               BitVector &SavedRegs,
                                               RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  if (hasFP(MF))
    SavedRegs.set(SH::R14);
}

// Push order: R14, then R8 up to R13, then PR.
static unsigned getPushRank(MCRegister Reg) {
  switch (Reg) {
  case SH::R14:
    return 0;
  case SH::R8:
    return 1;
  case SH::R9:
    return 2;
  case SH::R10:
    return 3;
  case SH::R11:
    return 4;
  case SH::R12:
    return 5;
  case SH::R13:
    return 6;
  case SH::PR:
    return 7;
  default:
    llvm_unreachable("unexpected callee-saved register");
  }
}

bool SuperHFrameLowering::assignCalleeSavedSpillSlots(
    MachineFunction &MF, const TargetRegisterInfo *TRI,
    std::vector<CalleeSavedInfo> &CSI) const {
  llvm::sort(CSI, [](const CalleeSavedInfo &A, const CalleeSavedInfo &B) {
    return getPushRank(A.getReg()) < getPushRank(B.getReg());
  });
  MachineFrameInfo &MFI = MF.getFrameInfo();
  auto *FuncInfo = MF.getInfo<SuperHMachineFunctionInfo>();
  int64_t Start = -int64_t(FuncInfo->getVarArgsSaveSize());
  int64_t Offset = Start;
  for (CalleeSavedInfo &Info : CSI) {
    Offset -= 4;
    Info.setFrameIdx(MFI.CreateFixedSpillStackObject(4, Offset));
  }
  FuncInfo->setCalleeSavedSize(Start - Offset);
  return true;
}

// Emits a CFI instruction for the debugger and unwinder.
static void emitCFI(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                    const DebugLoc &DL, const MCCFIInstruction &Instruction) {
  MachineFunction &MF = *MBB.getParent();
  if (!MF.needsFrameMoves())
    return;
  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();
  BuildMI(MBB, MI, DL, TII.get(TargetOpcode::CFI_INSTRUCTION))
      .addCFIIndex(MF.addFrameInst(Instruction))
      .setMIFlag(MachineInstr::FrameSetup);
}

bool SuperHFrameLowering::spillCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    ArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  const SuperHInstrInfo &TII = *STI.getInstrInfo();
  DebugLoc DL = MBB.findDebugLoc(MI);
  const MCRegisterInfo *MRI = MBB.getParent()->getContext().getRegisterInfo();
  int64_t Offset =
      MBB.getParent()->getInfo<SuperHMachineFunctionInfo>()->getVarArgsSaveSize();
  for (const CalleeSavedInfo &Info : CSI) {
    MCRegister Reg = Info.getReg();
    if (!MBB.isLiveIn(Reg))
      MBB.addLiveIn(Reg);
    if (Reg == SH::PR)
      BuildMI(MBB, MI, DL, TII.get(SH::STPRP), SH::R15)
          .addReg(SH::R15)
          .setMIFlag(MachineInstr::FrameSetup);
    else
      BuildMI(MBB, MI, DL, TII.get(SH::STLP), SH::R15)
          .addReg(Reg, RegState::Kill)
          .addReg(SH::R15)
          .setMIFlag(MachineInstr::FrameSetup);
    Offset += 4;
    emitCFI(MBB, MI, DL, MCCFIInstruction::cfiDefCfaOffset(nullptr, Offset));
    emitCFI(MBB, MI, DL,
            MCCFIInstruction::createOffset(
                nullptr, MRI->getDwarfRegNum(Reg, true), -Offset));
  }
  return true;
}

bool SuperHFrameLowering::restoreCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    MutableArrayRef<CalleeSavedInfo> CSI,
    const TargetRegisterInfo *TRI) const {
  const SuperHInstrInfo &TII = *STI.getInstrInfo();
  DebugLoc DL = MBB.findDebugLoc(MI);
  for (const CalleeSavedInfo &Info : llvm::reverse(CSI)) {
    MCRegister Reg = Info.getReg();
    if (Reg == SH::PR)
      BuildMI(MBB, MI, DL, TII.get(SH::LDPRP), SH::R15)
          .addReg(SH::R15)
          .setMIFlag(MachineInstr::FrameDestroy);
    else
      BuildMI(MBB, MI, DL, TII.get(SH::LDLP), Reg)
          .addReg(SH::R15, RegState::Define)
          .addReg(SH::R15)
          .setMIFlag(MachineInstr::FrameDestroy);
  }
  return true;
}

StackOffset
SuperHFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                            Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  FrameReg = hasFP(MF) ? SH::R14 : SH::R15;
  return StackOffset::getFixed(MFI.getObjectOffset(FI) + MFI.getStackSize());
}

static int64_t getLocalSize(const MachineFunction &MF) {
  auto *FuncInfo = MF.getInfo<SuperHMachineFunctionInfo>();
  return MF.getFrameInfo().getStackSize() - FuncInfo->getCalleeSavedSize() -
         FuncInfo->getVarArgsSaveSize();
}

void SuperHFrameLowering::emitPrologue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  const SuperHInstrInfo &TII = *STI.getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  // Room for the unnamed argument registers goes above the pushed registers.
  if (int64_t SaveSize =
          MF.getInfo<SuperHMachineFunctionInfo>()->getVarArgsSaveSize()) {
    TII.adjustRegister(MBB, MBBI, DL, SH::R15, -SaveSize, SH::R1,
                       MachineInstr::FrameSetup);
    emitCFI(MBB, MBBI, DL,
            MCCFIInstruction::cfiDefCfaOffset(nullptr, SaveSize));
  }

  while (MBBI != MBB.end() && MBBI->getFlag(MachineInstr::FrameSetup))
    ++MBBI;
  if (MBBI != MBB.end())
    DL = MBBI->getDebugLoc();

  // R1 holds nothing on entry under either convention.
  int64_t LocalSize = getLocalSize(MF);
  TII.adjustRegister(MBB, MBBI, DL, SH::R15, -LocalSize, SH::R1,
                     MachineInstr::FrameSetup);
  int64_t StackSize = MF.getFrameInfo().getStackSize();
  if (LocalSize)
    emitCFI(MBB, MBBI, DL,
            MCCFIInstruction::cfiDefCfaOffset(nullptr, StackSize));

  if (hasFP(MF)) {
    BuildMI(MBB, MBBI, DL, TII.get(SH::MOV), SH::R14)
        .addReg(SH::R15)
        .setMIFlag(MachineInstr::FrameSetup);
    const MCRegisterInfo *MRI = MF.getContext().getRegisterInfo();
    emitCFI(MBB, MBBI, DL,
            MCCFIInstruction::createDefCfaRegister(
                nullptr, MRI->getDwarfRegNum(SH::R14, true)));
  }
}

void SuperHFrameLowering::emitEpilogue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  const SuperHInstrInfo &TII = *STI.getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.getFirstTerminator();
  DebugLoc DL = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  while (MBBI != MBB.begin() &&
         std::prev(MBBI)->getFlag(MachineInstr::FrameDestroy))
    --MBBI;

  // A scratch register for large frames: a temporary the return doesn't use.
  Register Scratch;
  MachineBasicBlock::iterator Terminator = MBB.getFirstTerminator();
  for (MCRegister Candidate : {SH::R1, SH::R2, SH::R3}) {
    if (Terminator == MBB.end() ||
        !Terminator->readsRegister(Candidate, /*TRI=*/nullptr)) {
      Scratch = Candidate;
      break;
    }
  }

  if (hasFP(MF))
    BuildMI(MBB, MBBI, DL, TII.get(SH::MOV), SH::R15)
        .addReg(SH::R14)
        .setMIFlag(MachineInstr::FrameDestroy);
  TII.adjustRegister(MBB, MBBI, DL, SH::R15, getLocalSize(MF), Scratch,
                     MachineInstr::FrameDestroy);

  if (int64_t SaveSize =
          MF.getInfo<SuperHMachineFunctionInfo>()->getVarArgsSaveSize())
    TII.adjustRegister(MBB, Terminator, DL, SH::R15, SaveSize, Scratch,
                       MachineInstr::FrameDestroy);
}
