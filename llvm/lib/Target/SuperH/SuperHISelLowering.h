//===-- SuperHISelLowering.h - SH DAG Lowering Interface --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that SuperH uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SUPERH_SUPERHISELLOWERING_H
#define LLVM_LIB_TARGET_SUPERH_SUPERHISELLOWERING_H

#include "SuperH.h"
#include "SuperHSelectionDAGInfo.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
class SuperHSubtarget;

/// Assigns argument locations for the SuperH calling conventions. Arguments
/// are laid out as a memory image of 32-bit words; the first four words go in
/// R4-R7. Under the Windows CE convention every word, including those in
/// registers, has a stack offset: the caller reserves the first 16 bytes as a
/// home area for R4-R7.
bool CC_SuperH(unsigned ValNo, MVT ValVT, MVT LocVT,
               CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
               Type *OrigTy, CCState &State);
bool CC_SuperH_WinCE(unsigned ValNo, MVT ValVT, MVT LocVT,
                     CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                     Type *OrigTy, CCState &State);

class SuperHTargetLowering : public TargetLowering {
  const SuperHSubtarget *Subtarget;

public:
  SuperHTargetLowering(const TargetMachine &TM, const SuperHSubtarget &STI);

  bool useSoftFloat() const override;

  MVT getScalarShiftAmountTy(const DataLayout &, EVT) const override {
    return MVT::i32;
  }

  EVT getSetCCResultType(const DataLayout &DL, LLVMContext &Context,
                         EVT VT) const override {
    return MVT::i32;
  }

  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  void ReplaceNodeResults(SDNode *N, SmallVectorImpl<SDValue> &Results,
                          SelectionDAG &DAG) const override;

  SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerCall(CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      LLVMContext &Context, const Type *RetTy) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals,
                      const SDLoc &DL, SelectionDAG &DAG) const override;

  bool isLegalAddressingMode(const DataLayout &DL, const AddrMode &AM,
                             Type *Ty, unsigned AddrSpace,
                             Instruction *I = nullptr) const override;

  bool isLegalICmpImmediate(int64_t Imm) const override;
  bool isLegalAddImmediate(int64_t Imm) const override;

  bool isTruncateFree(Type *SrcTy, Type *DstTy) const override;
  bool isTruncateFree(EVT SrcVT, EVT DstVT) const override;

  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr &MI,
                              MachineBasicBlock *MBB) const override;

  Register getRegisterByName(const char *RegName, LLT VT,
                             const MachineFunction &MF) const override;

  ConstraintType getConstraintType(StringRef Constraint) const override;
  std::pair<unsigned, const TargetRegisterClass *>
  getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                               StringRef Constraint, MVT VT) const override;

  /// Size of the area the caller reserves for R4-R7 under the Windows CE
  /// calling standard.
  static constexpr unsigned WinCEHomeAreaSize = 16;

private:
  CCAssignFn *getArgumentAssignFn() const;

  SDValue lowerAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerSETCC(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerVASTART(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerFRAMEADDR(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerRETURNADDR(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerCarryOp(SDValue Op, SelectionDAG &DAG) const;
  SDValue lowerMULLOHI(SDValue Op, SelectionDAG &DAG) const;

  /// Emits a comparison of LHS and RHS that sets T. On return, Invert says
  /// whether T holds the opposite of the condition.
  SDValue emitCompare(SDValue LHS, SDValue RHS, ISD::CondCode CC,
                      bool &Invert, const SDLoc &DL, SelectionDAG &DAG) const;

  SDValue lowerCallResult(SDValue Chain, SDValue InGlue,
                          CallingConv::ID CallConv, bool IsVarArg,
                          const SmallVectorImpl<ISD::InputArg> &Ins,
                          const SDLoc &DL, SelectionDAG &DAG,
                          SmallVectorImpl<SDValue> &InVals) const;

  MachineBasicBlock *emitSelect(MachineInstr &MI,
                                MachineBasicBlock *MBB) const;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_SUPERH_SUPERHISELLOWERING_H
