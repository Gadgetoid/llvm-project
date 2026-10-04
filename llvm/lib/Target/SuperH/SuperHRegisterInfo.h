//===-- SuperHRegisterInfo.h - SuperH Register Information ------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SUPERH_SUPERHREGISTERINFO_H
#define LLVM_LIB_TARGET_SUPERH_SUPERHREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "SuperHGenRegisterInfo.inc"

namespace llvm {
class SuperHSubtarget;

class SuperHRegisterInfo : public SuperHGenRegisterInfo {
public:
  SuperHRegisterInfo(const SuperHSubtarget &STI);

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;
  const uint32_t *getNoPreservedMask() const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;
  Register getFrameRegister(const MachineFunction &MF) const override;

  const TargetRegisterClass *
  getLargestLegalSuperClass(const TargetRegisterClass *RC,
                            const MachineFunction &MF) const override;

  const TargetRegisterClass *
  getCrossCopyRegClass(const TargetRegisterClass *RC) const override;

  bool shouldCoalesce(MachineInstr *MI, const TargetRegisterClass *SrcRC,
                      unsigned SubReg, const TargetRegisterClass *DstRC,
                      unsigned DstSubReg, const TargetRegisterClass *NewRC,
                      LiveIntervals &LIS) const override;

  bool requiresRegisterScavenging(const MachineFunction &MF) const override {
    return true;
  }
  bool eliminateFrameIndicesBackwards() const override { return true; }

  bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_SUPERH_SUPERHREGISTERINFO_H
