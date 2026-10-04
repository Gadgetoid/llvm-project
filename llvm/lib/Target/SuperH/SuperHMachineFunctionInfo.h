//===-- SuperHMachineFunctionInfo.h - SuperH private data -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares the SuperH specific subclass of MachineFunctionInfo.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SUPERH_SUPERHMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_SUPERH_SUPERHMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

class SuperHMachineFunctionInfo : public MachineFunctionInfo {
  /// Frame index of the first variadic argument.
  int VarArgsFrameIndex = 0;

  /// Virtual register holding the hidden structure return pointer.
  Register SRetReturnReg;

  /// Bytes of callee-saved registers pushed by the prologue.
  unsigned CalleeSavedSize = 0;

  /// Counter for literal pool labels.
  unsigned NextLiteralLabel = 0;

public:
  explicit SuperHMachineFunctionInfo(const Function &F,
                                     const TargetSubtargetInfo *STI) {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

  Register getSRetReturnReg() const { return SRetReturnReg; }
  void setSRetReturnReg(Register Reg) { SRetReturnReg = Reg; }

  unsigned getCalleeSavedSize() const { return CalleeSavedSize; }
  void setCalleeSavedSize(unsigned Size) { CalleeSavedSize = Size; }

  unsigned createLiteralLabel() { return NextLiteralLabel++; }
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_SUPERH_SUPERHMACHINEFUNCTIONINFO_H
