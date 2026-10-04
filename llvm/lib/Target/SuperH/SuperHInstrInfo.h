//===-- SuperHInstrInfo.h - SuperH Instruction Information ------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SUPERH_SUPERHINSTRINFO_H
#define LLVM_LIB_TARGET_SUPERH_SUPERHINSTRINFO_H

#include "SuperHRegisterInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "SuperHGenInstrInfo.inc"

namespace llvm {

class SuperHSubtarget;

namespace SHII {
/// Target-specific flags of the SuperH TSFlags field.
enum {
  /// The instruction may not be placed in a delay slot.
  DelayIllegal = 1 << 0,
};
} // namespace SHII

class SuperHInstrInfo : public SuperHGenInstrInfo {
  const SuperHRegisterInfo RI;

  virtual void anchor();

public:
  explicit SuperHInstrInfo(const SuperHSubtarget &STI);

  const SuperHRegisterInfo &getRegisterInfo() const { return RI; }

  enum ShiftKind { ShiftLeft, ShiftRight, ShiftArithmeticRight };

  /// Number of fixed shift instructions needed to shift by \p Amount, or a
  /// large number if the shift has no short fixed sequence.
  static unsigned getShiftCost(int64_t Amount, ShiftKind Kind);

  /// Whether an instruction may be placed in a branch delay slot.
  bool isDelaySlotCandidate(const MachineInstr &MI) const;

  unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

  MCInst getNop() const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  void storeRegToStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
      bool IsKill, int FrameIndex, const TargetRegisterClass *RC,
      Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void loadRegFromStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
      Register DestReg, int FrameIndex, const TargetRegisterClass *RC,
      Register VReg, unsigned SubReg = 0,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  Register isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;
  Register isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

  bool expandPostRAPseudo(MachineInstr &MI) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;
  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;
  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;
  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;
  MachineBasicBlock *getBranchDestBlock(const MachineInstr &MI) const override;

  /// Emits Reg += Amount, using \p Scratch for amounts that need a literal.
  void adjustRegister(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                      const DebugLoc &DL, Register Reg, int64_t Amount,
                      Register Scratch,
                      MachineInstr::MIFlag Flag = MachineInstr::NoFlags) const;

  /// Emits Reg = Value.
  void loadImmediate(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                     const DebugLoc &DL, Register Reg, int64_t Value,
                     MachineInstr::MIFlag Flag = MachineInstr::NoFlags) const;

private:
  void expandShift(MachineInstr &MI, ShiftKind Kind) const;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_SUPERH_SUPERHINSTRINFO_H
