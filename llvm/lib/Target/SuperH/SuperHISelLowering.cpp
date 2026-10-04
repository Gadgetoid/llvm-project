//===-- SuperHISelLowering.cpp - SH DAG Lowering Interface ------*- C++ -*-===//
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
// Two calling conventions are implemented. Both pass the argument list as a
// memory image of 32-bit words whose first four words travel in R4-R7.
//
// GNU/Renesas: stack arguments start at the caller's stack pointer and
// values are returned in R0-R3.
//
// Windows CE (triple environment "wince"), from Microsoft's SH-3 Calling
// Standard rev 1.4: the caller always reserves 16 bytes at its stack pointer
// as a home area for R4-R7, so stack arguments start at offset 16 and the
// whole list is contiguous in memory. 64-bit scalars and structures are
// 4-byte aligned and may be split between R7 and the stack. Only values of
// up to 32 bits are returned in R0; anything larger is returned through a
// hidden pointer passed in R4, which the callee also returns in R0.
//
//===----------------------------------------------------------------------===//

#include "SuperHISelLowering.h"
#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "SuperHInstrInfo.h"
#include "SuperHMachineFunctionInfo.h"
#include "SuperHRegisterInfo.h"
#include "SuperHSubtarget.h"
#include "SuperHTargetMachine.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "sh-isel-lowering"

static const MCPhysReg ArgumentRegisters[] = {SH::R4, SH::R5, SH::R6, SH::R7};

SuperHTargetLowering::SuperHTargetLowering(const TargetMachine &TM,
                                           const SuperHSubtarget &STI)
    : TargetLowering(TM, STI), Subtarget(&STI) {
  addRegisterClass(MVT::i32, &SH::GPRRegClass);
  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(SH::R15);
  setBooleanContents(ZeroOrOneBooleanContent);
  setSchedulingPreference(Sched::RegPressure);
  setMinFunctionAlignment(Align(4));
  setPrefFunctionAlignment(Align(4));
  setMinStackArgumentAlignment(Align(4));
  setMaxAtomicSizeInBitsSupported(0);

  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1, Promote);
  }
  setTruncStoreAction(MVT::i32, MVT::i1, Promote);
  setTruncStoreAction(MVT::i32, MVT::i8, Legal);
  setTruncStoreAction(MVT::i32, MVT::i16, Legal);

  setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  setOperationAction(ISD::ExternalSymbol, MVT::i32, Custom);
  setOperationAction(ISD::BlockAddress, MVT::i32, Custom);
  setOperationAction(ISD::JumpTable, MVT::i32, Custom);
  setOperationAction(ISD::ConstantPool, MVT::i32, Custom);

  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::BRCOND, MVT::Other, Expand);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::SETCC, MVT::i32, Custom);
  setOperationAction(ISD::SELECT, MVT::i32, Expand);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);

  for (unsigned Op : {ISD::SDIV, ISD::UDIV, ISD::SREM, ISD::UREM,
                      ISD::SDIVREM, ISD::UDIVREM})
    setOperationAction(Op, MVT::i32, Expand);

  setOperationAction(ISD::ROTL, MVT::i32, Custom);
  setOperationAction(ISD::ROTR, MVT::i32, Custom);
  setOperationAction(ISD::SHL_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRL_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::SRA_PARTS, MVT::i32, Expand);
  setOperationAction(ISD::CTPOP, MVT::i32, Expand);
  setOperationAction(ISD::CTLZ, MVT::i32, Expand);
  setOperationAction(ISD::CTTZ, MVT::i32, Expand);

  setOperationAction(ISD::UADDO, MVT::i32, Custom);
  setOperationAction(ISD::USUBO, MVT::i32, Custom);
  setOperationAction(ISD::UADDO_CARRY, MVT::i32, Custom);
  setOperationAction(ISD::USUBO_CARRY, MVT::i32, Custom);

  setOperationAction(ISD::SMUL_LOHI, MVT::i32, Legal);
  setOperationAction(ISD::UMUL_LOHI, MVT::i32, Legal);

  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32, Expand);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);

  setOperationAction(ISD::VASTART, MVT::Other, Custom);
  setOperationAction(ISD::VAARG, MVT::Other, Expand);
  setOperationAction(ISD::VACOPY, MVT::Other, Expand);
  setOperationAction(ISD::VAEND, MVT::Other, Expand);

  setOperationAction(ISD::FRAMEADDR, MVT::i32, Custom);
  setOperationAction(ISD::RETURNADDR, MVT::i32, Custom);

  setOperationAction(ISD::TRAP, MVT::Other, Legal);
  setOperationAction(ISD::DEBUGTRAP, MVT::Other, Legal);
}

bool SuperHTargetLowering::useSoftFloat() const { return true; }

//===----------------------------------------------------------------------===//
// Calling conventions
//===----------------------------------------------------------------------===//

// Promotes a sub-word integer argument to a full word. Returns false for a
// type that cannot be passed in a word.
static bool promoteToWord(MVT &LocVT, CCValAssign::LocInfo &LocInfo,
                          ISD::ArgFlagsTy ArgFlags) {
  if (LocVT == MVT::i1 || LocVT == MVT::i8 || LocVT == MVT::i16) {
    LocVT = MVT::i32;
    if (ArgFlags.isSExt())
      LocInfo = CCValAssign::SExt;
    else if (ArgFlags.isZExt())
      LocInfo = CCValAssign::ZExt;
    else
      LocInfo = CCValAssign::AExt;
  }
  return LocVT == MVT::i32;
}

bool llvm::CC_SuperH_WinCE(unsigned ValNo, MVT ValVT, MVT LocVT,
                           CCValAssign::LocInfo LocInfo,
                           ISD::ArgFlagsTy ArgFlags, Type *OrigTy,
                           CCState &State) {
  if (ArgFlags.isByVal()) {
    unsigned Size = alignTo(ArgFlags.getByValSize(), 4);
    unsigned Offset = State.AllocateStack(Size, Align(4));
    for (unsigned Word = Offset; Word < Offset + Size && Word < 16; Word += 4)
      State.AllocateReg(ArgumentRegisters[Word / 4]);
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
    return false;
  }

  if (!promoteToWord(LocVT, LocInfo, ArgFlags))
    return true;

  unsigned Offset = State.AllocateStack(4, Align(4));
  if (Offset < 16) {
    MCPhysReg Reg = ArgumentRegisters[Offset / 4];
    State.AllocateReg(Reg);
    State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
    return false;
  }
  State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
  return false;
}

bool llvm::CC_SuperH(unsigned ValNo, MVT ValVT, MVT LocVT,
                     CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags,
                     Type *OrigTy, CCState &State) {
  if (ArgFlags.isByVal()) {
    unsigned Size = alignTo(ArgFlags.getByValSize(), 4);
    unsigned Offset = State.AllocateStack(Size, Align(4));
    State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
    return false;
  }

  if (!promoteToWord(LocVT, LocInfo, ArgFlags))
    return true;

  if (MCRegister Reg = State.AllocateReg(ArgumentRegisters)) {
    State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
    return false;
  }
  unsigned Offset = State.AllocateStack(4, Align(4));
  State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
  return false;
}

CCAssignFn *SuperHTargetLowering::getArgumentAssignFn() const {
  return Subtarget->isTargetWinCE() ? CC_SuperH_WinCE : CC_SuperH;
}

static unsigned getMaxReturnWords(const SuperHSubtarget &STI) {
  return STI.isTargetWinCE() ? 1 : 4;
}

static const MCPhysReg ReturnRegisters[] = {SH::R0, SH::R1, SH::R2, SH::R3};

static SDValue extendArgument(SDValue Value, const CCValAssign &VA,
                              const SDLoc &DL, SelectionDAG &DAG) {
  switch (VA.getLocInfo()) {
  case CCValAssign::Full:
    return Value;
  case CCValAssign::SExt:
    return DAG.getNode(ISD::SIGN_EXTEND, DL, VA.getLocVT(), Value);
  case CCValAssign::ZExt:
    return DAG.getNode(ISD::ZERO_EXTEND, DL, VA.getLocVT(), Value);
  case CCValAssign::AExt:
    return DAG.getNode(ISD::ANY_EXTEND, DL, VA.getLocVT(), Value);
  case CCValAssign::BCvt:
    return DAG.getBitcast(VA.getLocVT(), Value);
  default:
    llvm_unreachable("unexpected argument location info");
  }
}

static SDValue truncateArgument(SDValue Value, const CCValAssign &VA,
                                const SDLoc &DL, SelectionDAG &DAG) {
  switch (VA.getLocInfo()) {
  case CCValAssign::SExt:
    Value = DAG.getNode(ISD::AssertSext, DL, VA.getLocVT(), Value,
                        DAG.getValueType(VA.getValVT()));
    break;
  case CCValAssign::ZExt:
    Value = DAG.getNode(ISD::AssertZext, DL, VA.getLocVT(), Value,
                        DAG.getValueType(VA.getValVT()));
    break;
  case CCValAssign::BCvt:
    return DAG.getBitcast(VA.getValVT(), Value);
  default:
    break;
  }
  if (VA.getValVT() != VA.getLocVT())
    Value = DAG.getNode(ISD::TRUNCATE, DL, VA.getValVT(), Value);
  return Value;
}

SDValue SuperHTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  auto *FuncInfo = MF.getInfo<SuperHMachineFunctionInfo>();
  bool WinCE = Subtarget->isTargetWinCE();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, getArgumentAssignFn());

  SmallVector<SDValue, 8> Stores;
  for (unsigned I = 0, E = ArgLocs.size(); I != E; ++I) {
    CCValAssign &VA = ArgLocs[I];
    ISD::ArgFlagsTy Flags = Ins[I].Flags;

    if (Flags.isByVal()) {
      unsigned Offset = VA.getLocMemOffset();
      unsigned Size = alignTo(Flags.getByValSize(), 4);
      int FI = MFI.CreateFixedObject(Size, Offset, /*IsImmutable=*/false);
      SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
      if (WinCE) {
        for (unsigned Word = Offset; Word < Offset + Size && Word < 16;
             Word += 4) {
          Register VReg =
              MF.addLiveIn(ArgumentRegisters[Word / 4], &SH::GPRRegClass);
          SDValue Value = DAG.getCopyFromReg(Chain, DL, VReg, MVT::i32);
          SDValue Address = DAG.getNode(ISD::ADD, DL, MVT::i32, FIN,
                                        DAG.getConstant(Word - Offset, DL,
                                                        MVT::i32));
          Stores.push_back(DAG.getStore(
              Chain, DL, Value, Address,
              MachinePointerInfo::getFixedStack(MF, FI, Word - Offset)));
        }
      }
      InVals.push_back(FIN);
      continue;
    }

    if (VA.isRegLoc()) {
      Register VReg = MF.addLiveIn(VA.getLocReg(), &SH::GPRRegClass);
      SDValue Value = DAG.getCopyFromReg(Chain, DL, VReg, MVT::i32);
      InVals.push_back(truncateArgument(Value, VA, DL, DAG));
      continue;
    }

    assert(VA.isMemLoc() && "argument not in a register or memory");
    int FI = MFI.CreateFixedObject(4, VA.getLocMemOffset(),
                                   /*IsImmutable=*/true);
    SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
    SDValue Value = DAG.getLoad(MVT::i32, DL, Chain, FIN,
                                MachinePointerInfo::getFixedStack(MF, FI));
    InVals.push_back(truncateArgument(Value, VA, DL, DAG));
  }

  if (IsVarArg) {
    if (!WinCE)
      report_fatal_error("SuperH: variadic functions need the Windows CE "
                         "calling convention");
    // The calling standard stores all four argument registers to the home
    // area, so that the whole argument list can be walked in memory.
    unsigned FirstVariadic = CCInfo.getStackSize();
    FuncInfo->setVarArgsFrameIndex(
        MFI.CreateFixedObject(4, FirstVariadic, /*IsImmutable=*/true));
    for (unsigned Word = 0; Word < WinCEHomeAreaSize; Word += 4) {
      Register VReg =
          MF.addLiveIn(ArgumentRegisters[Word / 4], &SH::GPRRegClass);
      SDValue Value = DAG.getCopyFromReg(Chain, DL, VReg, MVT::i32);
      int FI = MFI.CreateFixedObject(4, Word, /*IsImmutable=*/false);
      Stores.push_back(DAG.getStore(Chain, DL, Value,
                                    DAG.getFrameIndex(FI, MVT::i32),
                                    MachinePointerInfo::getFixedStack(MF, FI)));
    }
  }

  for (unsigned I = 0, E = Ins.size(); I != E; ++I) {
    if (!Ins[I].Flags.isSRet())
      continue;
    Register Reg = MF.getRegInfo().createVirtualRegister(&SH::GPRRegClass);
    FuncInfo->setSRetReturnReg(Reg);
    Chain = DAG.getCopyToReg(Chain, DL, Reg, InVals[I]);
    break;
  }

  if (!Stores.empty()) {
    Stores.push_back(Chain);
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, Stores);
  }
  return Chain;
}

bool SuperHTargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context,
    const Type *RetTy) const {
  if (Outs.size() > getMaxReturnWords(*Subtarget))
    return false;
  for (const ISD::OutputArg &Out : Outs)
    if (Out.VT.getSizeInBits() > 32)
      return false;
  return true;
}

SDValue
SuperHTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                  bool IsVarArg,
                                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                                  const SmallVectorImpl<SDValue> &OutVals,
                                  const SDLoc &DL, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  auto *FuncInfo = MF.getInfo<SuperHMachineFunctionInfo>();

  SDValue Glue;
  SmallVector<SDValue, 6> RetOps(1, Chain);
  for (unsigned I = 0, E = Outs.size(); I != E; ++I) {
    SDValue Value = OutVals[I];
    if (Value.getValueType() != MVT::i32) {
      ISD::NodeType Extend = Outs[I].Flags.isSExt()   ? ISD::SIGN_EXTEND
                             : Outs[I].Flags.isZExt() ? ISD::ZERO_EXTEND
                                                      : ISD::ANY_EXTEND;
      Value = DAG.getNode(Extend, DL, MVT::i32, Value);
    }
    Chain = DAG.getCopyToReg(Chain, DL, ReturnRegisters[I], Value, Glue);
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(ReturnRegisters[I], MVT::i32));
  }

  // A function returning through a hidden pointer returns the pointer too.
  if (Outs.empty() && FuncInfo->getSRetReturnReg()) {
    SDValue Pointer =
        DAG.getCopyFromReg(Chain, DL, FuncInfo->getSRetReturnReg(), MVT::i32);
    Chain = DAG.getCopyToReg(Chain, DL, SH::R0, Pointer, Glue);
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(SH::R0, MVT::i32));
  }

  RetOps[0] = Chain;
  if (Glue.getNode())
    RetOps.push_back(Glue);
  return DAG.getNode(SHISD::RET, DL, MVT::Other, RetOps);
}

SDValue SuperHTargetLowering::LowerCall(CallLoweringInfo &CLI,
                                        SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  SDLoc &DL = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  MachineFunction &MF = DAG.getMachineFunction();
  bool WinCE = Subtarget->isTargetWinCE();

  CLI.IsTailCall = false;

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CLI.CallConv, CLI.IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeCallOperands(Outs, getArgumentAssignFn());

  unsigned NumBytes = CCInfo.getStackSize();
  if (WinCE)
    NumBytes = std::max(NumBytes, WinCEHomeAreaSize);
  NumBytes = alignTo(NumBytes, 4);

  Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, DL);
  SDValue StackPointer = DAG.getCopyFromReg(Chain, DL, SH::R15, MVT::i32);

  auto stackAddress = [&](unsigned Offset) {
    return DAG.getNode(ISD::ADD, DL, MVT::i32, StackPointer,
                       DAG.getIntPtrConstant(Offset, DL));
  };

  SmallVector<std::pair<MCRegister, SDValue>, 4> RegsToPass;
  SmallVector<SDValue, 8> MemOps;
  for (unsigned I = 0, E = ArgLocs.size(); I != E; ++I) {
    CCValAssign &VA = ArgLocs[I];
    ISD::ArgFlagsTy Flags = Outs[I].Flags;
    SDValue Arg = OutVals[I];

    if (Flags.isByVal()) {
      unsigned Offset = VA.getLocMemOffset();
      unsigned Size = Flags.getByValSize();
      Align ByValAlign = Flags.getNonZeroByValAlign();
      unsigned Word = Offset;
      if (WinCE) {
        for (; Word < Offset + Size && Word < 16; Word += 4) {
          unsigned Within = Word - Offset;
          unsigned Bytes = std::min(4u, Size - Within);
          SDValue Source = DAG.getNode(ISD::ADD, DL, MVT::i32, Arg,
                                       DAG.getIntPtrConstant(Within, DL));
          Align ElementAlign = commonAlignment(ByValAlign, Within);
          SDValue Value;
          if (Bytes == 4)
            Value = DAG.getLoad(MVT::i32, DL, Chain, Source,
                                MachinePointerInfo(), ElementAlign);
          else
            Value = DAG.getExtLoad(ISD::ZEXTLOAD, DL, MVT::i32, Chain, Source,
                                   MachinePointerInfo(),
                                   EVT::getIntegerVT(*DAG.getContext(),
                                                     Bytes * 8),
                                   ElementAlign);
          MemOps.push_back(Value.getValue(1));
          RegsToPass.push_back({ArgumentRegisters[Word / 4], Value});
        }
      }
      if (Word < Offset + Size) {
        unsigned Within = Word - Offset;
        SDValue Source = DAG.getNode(ISD::ADD, DL, MVT::i32, Arg,
                                     DAG.getIntPtrConstant(Within, DL));
        SDValue Copy = DAG.getMemcpy(
            Chain, DL, stackAddress(Word), Source,
            DAG.getConstant(Size - Within, DL, MVT::i32), Align(4),
            commonAlignment(ByValAlign, Within), /*isVol=*/false,
            /*AlwaysInline=*/true, /*CI=*/nullptr, std::nullopt,
            MachinePointerInfo(), MachinePointerInfo());
        MemOps.push_back(Copy);
      }
      continue;
    }

    Arg = extendArgument(Arg, VA, DL, DAG);
    if (VA.isRegLoc()) {
      RegsToPass.push_back({VA.getLocReg(), Arg});
      continue;
    }
    assert(VA.isMemLoc() && "argument not in a register or memory");
    MemOps.push_back(DAG.getStore(
        Chain, DL, Arg, stackAddress(VA.getLocMemOffset()),
        MachinePointerInfo::getStack(MF, VA.getLocMemOffset())));
  }

  if (!MemOps.empty())
    Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOps);

  SDValue Glue;
  for (auto &[Reg, Value] : RegsToPass) {
    Chain = DAG.getCopyToReg(Chain, DL, Reg, Value, Glue);
    Glue = Chain.getValue(1);
  }

  if (auto *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getNode(SHISD::WRAPPER, DL, MVT::i32,
                         DAG.getTargetGlobalAddress(G->getGlobal(), DL,
                                                    MVT::i32, G->getOffset()));
  else if (auto *S = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getNode(SHISD::WRAPPER, DL, MVT::i32,
                         DAG.getTargetExternalSymbol(S->getSymbol(), MVT::i32));

  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);
  for (auto &[Reg, Value] : RegsToPass)
    Ops.push_back(DAG.getRegister(Reg, Value.getValueType()));
  const TargetRegisterInfo *TRI = Subtarget->getRegisterInfo();
  Ops.push_back(DAG.getRegisterMask(TRI->getCallPreservedMask(MF, CLI.CallConv)));
  if (Glue.getNode())
    Ops.push_back(Glue);

  Chain = DAG.getNode(SHISD::CALL, DL, DAG.getVTList(MVT::Other, MVT::Glue),
                      Ops);
  Glue = Chain.getValue(1);

  Chain = DAG.getCALLSEQ_END(Chain, NumBytes, 0, Glue, DL);
  Glue = Chain.getValue(1);

  return lowerCallResult(Chain, Glue, CLI.CallConv, CLI.IsVarArg, CLI.Ins, DL,
                         DAG, InVals);
}

SDValue SuperHTargetLowering::lowerCallResult(
    SDValue Chain, SDValue Glue, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  assert(Ins.size() <= getMaxReturnWords(*Subtarget) &&
         "return value should have been demoted");
  for (unsigned I = 0, E = Ins.size(); I != E; ++I) {
    SDValue Value =
        DAG.getCopyFromReg(Chain, DL, ReturnRegisters[I], MVT::i32, Glue);
    Chain = Value.getValue(1);
    Glue = Value.getValue(2);
    if (Ins[I].VT != MVT::i32)
      Value = DAG.getNode(ISD::TRUNCATE, DL, Ins[I].VT, Value);
    InVals.push_back(Value);
  }
  return Chain;
}

//===----------------------------------------------------------------------===//
// Custom lowering
//===----------------------------------------------------------------------===//

SDValue SuperHTargetLowering::lowerAddress(SDValue Op,
                                           SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Target;
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress: {
    auto *N = cast<GlobalAddressSDNode>(Op);
    Target = DAG.getTargetGlobalAddress(N->getGlobal(), DL, MVT::i32,
                                        N->getOffset());
    break;
  }
  case ISD::ExternalSymbol:
    Target = DAG.getTargetExternalSymbol(
        cast<ExternalSymbolSDNode>(Op)->getSymbol(), MVT::i32);
    break;
  case ISD::BlockAddress: {
    auto *N = cast<BlockAddressSDNode>(Op);
    Target = DAG.getTargetBlockAddress(N->getBlockAddress(), MVT::i32,
                                       N->getOffset());
    break;
  }
  case ISD::JumpTable:
    Target = DAG.getTargetJumpTable(cast<JumpTableSDNode>(Op)->getIndex(),
                                    MVT::i32);
    break;
  case ISD::ConstantPool: {
    auto *N = cast<ConstantPoolSDNode>(Op);
    if (N->isMachineConstantPoolEntry())
      Target = DAG.getTargetConstantPool(N->getMachineCPVal(), MVT::i32,
                                         N->getAlign(), N->getOffset());
    else
      Target = DAG.getTargetConstantPool(N->getConstVal(), MVT::i32,
                                         N->getAlign(), N->getOffset());
    break;
  }
  default:
    llvm_unreachable("not an address");
  }
  return DAG.getNode(SHISD::WRAPPER, DL, MVT::i32, Target);
}

static bool isConstantValue(SDValue V, int64_t Value) {
  if (auto *C = dyn_cast<ConstantSDNode>(V))
    return C->getSExtValue() == Value;
  return false;
}

SDValue SuperHTargetLowering::emitCompare(SDValue LHS, SDValue RHS,
                                          ISD::CondCode CC, bool &Invert,
                                          const SDLoc &DL,
                                          SelectionDAG &DAG) const {
  Invert = false;
  auto node = [&](unsigned Opcode, SDValue A, SDValue B) {
    return DAG.getNode(Opcode, DL, MVT::i32, A, B);
  };
  auto unary = [&](unsigned Opcode, SDValue A) {
    return DAG.getNode(Opcode, DL, MVT::i32, A);
  };

  switch (CC) {
  case ISD::SETNE:
    Invert = true;
    [[fallthrough]];
  case ISD::SETEQ:
    if (isConstantValue(RHS, 0)) {
      if (LHS.getOpcode() == ISD::AND && LHS.hasOneUse())
        return node(SHISD::TST, LHS.getOperand(0), LHS.getOperand(1));
      return node(SHISD::TST, LHS, LHS);
    }
    return node(SHISD::CMPEQ, LHS, RHS);
  case ISD::SETGT:
    if (isConstantValue(RHS, 0))
      return unary(SHISD::CMPPL, LHS);
    if (isConstantValue(RHS, -1))
      return unary(SHISD::CMPPZ, LHS);
    return node(SHISD::CMPGT, LHS, RHS);
  case ISD::SETGE:
    if (isConstantValue(RHS, 0))
      return unary(SHISD::CMPPZ, LHS);
    if (isConstantValue(RHS, 1))
      return unary(SHISD::CMPPL, LHS);
    return node(SHISD::CMPGE, LHS, RHS);
  case ISD::SETLT:
    if (isConstantValue(RHS, 0)) {
      Invert = true;
      return unary(SHISD::CMPPZ, LHS);
    }
    if (isConstantValue(RHS, 1)) {
      Invert = true;
      return unary(SHISD::CMPPL, LHS);
    }
    if (isConstantValue(LHS, 0))
      return unary(SHISD::CMPPL, RHS);
    return node(SHISD::CMPGT, RHS, LHS);
  case ISD::SETLE:
    if (isConstantValue(RHS, 0)) {
      Invert = true;
      return unary(SHISD::CMPPL, LHS);
    }
    if (isConstantValue(RHS, -1)) {
      Invert = true;
      return unary(SHISD::CMPPZ, LHS);
    }
    return node(SHISD::CMPGE, RHS, LHS);
  case ISD::SETUGT:
    if (isConstantValue(RHS, 0)) {
      Invert = true;
      return node(SHISD::TST, LHS, LHS);
    }
    return node(SHISD::CMPHI, LHS, RHS);
  case ISD::SETUGE:
    return node(SHISD::CMPHS, LHS, RHS);
  case ISD::SETULT:
    return node(SHISD::CMPHI, RHS, LHS);
  case ISD::SETULE:
    if (isConstantValue(RHS, 0))
      return node(SHISD::TST, LHS, LHS);
    return node(SHISD::CMPHS, RHS, LHS);
  default:
    llvm_unreachable("unexpected integer condition code");
  }
}

SDValue SuperHTargetLowering::lowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  bool Invert;
  SDValue T =
      emitCompare(Op.getOperand(2), Op.getOperand(3), CC, Invert, DL, DAG);
  return DAG.getNode(Invert ? SHISD::BF : SHISD::BT, DL, MVT::Other, Chain,
                     Op.getOperand(4), T);
}

SDValue SuperHTargetLowering::lowerSETCC(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(2))->get();
  bool Invert;
  SDValue T =
      emitCompare(Op.getOperand(0), Op.getOperand(1), CC, Invert, DL, DAG);
  if (!Invert)
    return DAG.getNode(SHISD::MOVT, DL, MVT::i32, T);
  // negc of -1 gives 1 - T.
  SDValue NegC = DAG.getNode(SHISD::NEGC, DL, DAG.getVTList(MVT::i32, MVT::i32),
                             DAG.getAllOnesConstant(DL, MVT::i32), T);
  return NegC.getValue(0);
}

SDValue SuperHTargetLowering::lowerSELECT_CC(SDValue Op,
                                             SelectionDAG &DAG) const {
  SDLoc DL(Op);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
  bool Invert;
  SDValue T =
      emitCompare(Op.getOperand(0), Op.getOperand(1), CC, Invert, DL, DAG);
  SDValue TrueValue = Op.getOperand(2);
  SDValue FalseValue = Op.getOperand(3);
  if (Invert)
    std::swap(TrueValue, FalseValue);
  return DAG.getNode(SHISD::SELECT, DL, MVT::i32, TrueValue, FalseValue, T);
}

SDValue SuperHTargetLowering::lowerVASTART(SDValue Op,
                                           SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  auto *FuncInfo = MF.getInfo<SuperHMachineFunctionInfo>();
  SDLoc DL(Op);
  SDValue FrameIndex =
      DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(), MVT::i32);
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
  return DAG.getStore(Op.getOperand(0), DL, FrameIndex, Op.getOperand(1),
                      MachinePointerInfo(SV));
}

SDValue SuperHTargetLowering::lowerFRAMEADDR(SDValue Op,
                                             SelectionDAG &DAG) const {
  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  MFI.setFrameAddressIsTaken(true);
  SDLoc DL(Op);
  if (Op.getConstantOperandVal(0) != 0)
    return DAG.getConstant(0, DL, MVT::i32);
  return DAG.getCopyFromReg(DAG.getEntryNode(), DL, SH::R14, MVT::i32);
}

SDValue SuperHTargetLowering::lowerRETURNADDR(SDValue Op,
                                              SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  MF.getFrameInfo().setReturnAddressIsTaken(true);
  SDLoc DL(Op);
  if (Op.getConstantOperandVal(0) != 0)
    return DAG.getConstant(0, DL, MVT::i32);
  Register Reg = MF.addLiveIn(SH::PR, &SH::GPRRegClass);
  return DAG.getCopyFromReg(DAG.getEntryNode(), DL, Reg, MVT::i32);
}

SDValue SuperHTargetLowering::lowerCarryOp(SDValue Op,
                                           SelectionDAG &DAG) const {
  SDLoc DL(Op);
  unsigned Opcode = Op.getOpcode();
  bool IsAdd = Opcode == ISD::UADDO || Opcode == ISD::UADDO_CARRY;
  SDValue T;
  if (Opcode == ISD::UADDO || Opcode == ISD::USUBO) {
    T = DAG.getNode(SHISD::CLRT, DL, MVT::i32);
  } else {
    SDValue Carry = Op.getOperand(2);
    if (Carry.getOpcode() == SHISD::MOVT)
      T = Carry.getOperand(0);
    else
      T = DAG.getNode(SHISD::CMPPL, DL, MVT::i32, Carry);
  }
  SDValue Result =
      DAG.getNode(IsAdd ? SHISD::ADDC : SHISD::SUBC, DL,
                  DAG.getVTList(MVT::i32, MVT::i32), Op.getOperand(0),
                  Op.getOperand(1), T);
  SDValue CarryOut = DAG.getNode(SHISD::MOVT, DL, MVT::i32, Result.getValue(1));
  return DAG.getMergeValues({Result.getValue(0), CarryOut}, DL);
}

SDValue SuperHTargetLowering::LowerOperation(SDValue Op,
                                             SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:
  case ISD::ExternalSymbol:
  case ISD::BlockAddress:
  case ISD::JumpTable:
  case ISD::ConstantPool:
    return lowerAddress(Op, DAG);
  case ISD::BR_CC:
    return lowerBR_CC(Op, DAG);
  case ISD::SETCC:
    return lowerSETCC(Op, DAG);
  case ISD::SELECT_CC:
    return lowerSELECT_CC(Op, DAG);
  case ISD::VASTART:
    return lowerVASTART(Op, DAG);
  case ISD::FRAMEADDR:
    return lowerFRAMEADDR(Op, DAG);
  case ISD::RETURNADDR:
    return lowerRETURNADDR(Op, DAG);
  case ISD::UADDO:
  case ISD::USUBO:
  case ISD::UADDO_CARRY:
  case ISD::USUBO_CARRY:
    return lowerCarryOp(Op, DAG);
  case ISD::ROTL:
  case ISD::ROTR: {
    // rotl and rotr by one, and rotations by 16, have instructions.
    if (auto *C = dyn_cast<ConstantSDNode>(Op.getOperand(1))) {
      uint64_t Amount = C->getZExtValue() & 31;
      if (Amount == 1 || Amount == 16)
        return SDValue();
      if (Amount == 31)
        return DAG.getNode(Op.getOpcode() == ISD::ROTL ? ISD::ROTR : ISD::ROTL,
                           SDLoc(Op), MVT::i32, Op.getOperand(0),
                           DAG.getConstant(1, SDLoc(Op), MVT::i32));
    }
    return expandROT(Op.getNode(), /*AllowVectorOps=*/false, DAG);
  }
  default:
    llvm_unreachable("unexpected operation to lower");
  }
}

void SuperHTargetLowering::ReplaceNodeResults(SDNode *N,
                                              SmallVectorImpl<SDValue> &Results,
                                              SelectionDAG &DAG) const {
  llvm_unreachable("no illegal result types expected");
}

SDValue SuperHTargetLowering::PerformDAGCombine(SDNode *N,
                                                DAGCombinerInfo &DCI) const {
  switch (N->getOpcode()) {
  case SHISD::CMPPL: {
    // cmp/pl of a movt is T itself.
    SDValue Operand = N->getOperand(0);
    if (Operand.getOpcode() == SHISD::MOVT)
      return Operand.getOperand(0);
    break;
  }
  default:
    break;
  }
  return SDValue();
}

//===----------------------------------------------------------------------===//
// Addressing modes
//===----------------------------------------------------------------------===//

bool SuperHTargetLowering::isLegalAddressingMode(const DataLayout &DL,
                                                 const AddrMode &AM, Type *Ty,
                                                 unsigned AddrSpace,
                                                 Instruction *I) const {
  if (AM.BaseGV)
    return false;
  unsigned Size = Ty->isSized() ? DL.getTypeStoreSize(Ty) : 4;
  if (Size > 4)
    Size = 4;
  switch (AM.Scale) {
  case 0:
    if (AM.BaseOffs == 0)
      return true;
    return AM.BaseOffs > 0 && AM.BaseOffs <= 15 * (int64_t)Size &&
           AM.BaseOffs % Size == 0;
  case 1:
    return AM.BaseOffs == 0 && AM.HasBaseReg;
  default:
    return false;
  }
}

bool SuperHTargetLowering::isLegalICmpImmediate(int64_t Imm) const {
  return Imm == 0;
}

bool SuperHTargetLowering::isLegalAddImmediate(int64_t Imm) const {
  return isInt<8>(Imm);
}

bool SuperHTargetLowering::isTruncateFree(Type *SrcTy, Type *DstTy) const {
  return SrcTy->isIntegerTy() && DstTy->isIntegerTy() &&
         SrcTy->getPrimitiveSizeInBits() > DstTy->getPrimitiveSizeInBits();
}

bool SuperHTargetLowering::isTruncateFree(EVT SrcVT, EVT DstVT) const {
  return SrcVT.isInteger() && DstVT.isInteger() &&
         SrcVT.getSizeInBits() > DstVT.getSizeInBits();
}

//===----------------------------------------------------------------------===//
// Custom inserters
//===----------------------------------------------------------------------===//

// Whether T, as read by MI, is read again after it.
static bool isTLiveAfter(MachineInstr &MI, MachineBasicBlock *MBB) {
  for (MachineInstr &Next :
       make_range(std::next(MachineBasicBlock::iterator(MI)), MBB->end())) {
    if (Next.readsRegister(SH::SR, /*TRI=*/nullptr))
      return true;
    if (Next.definesRegister(SH::SR, /*TRI=*/nullptr))
      return false;
  }
  for (MachineBasicBlock *Successor : MBB->successors())
    if (Successor->isLiveIn(SH::SR))
      return true;
  return false;
}

MachineBasicBlock *
SuperHTargetLowering::emitSelect(MachineInstr &MI,
                                 MachineBasicBlock *MBB) const {
  const TargetInstrInfo &TII = *Subtarget->getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();
  MachineFunction *MF = MBB->getParent();
  const BasicBlock *LLVMBB = MBB->getBasicBlock();
  MachineFunction::iterator Insert = ++MBB->getIterator();

  //  MBB:   ...
  //         bt SinkMBB
  //  FalseMBB:
  //  SinkMBB:
  //         dst = phi [tval, MBB], [fval, FalseMBB]
  MachineBasicBlock *FalseMBB = MF->CreateMachineBasicBlock(LLVMBB);
  MachineBasicBlock *SinkMBB = MF->CreateMachineBasicBlock(LLVMBB);
  MF->insert(Insert, FalseMBB);
  MF->insert(Insert, SinkMBB);
  if (isTLiveAfter(MI, MBB)) {
    FalseMBB->addLiveIn(SH::SR);
    SinkMBB->addLiveIn(SH::SR);
  }

  SinkMBB->splice(SinkMBB->begin(), MBB,
                  std::next(MachineBasicBlock::iterator(MI)), MBB->end());
  SinkMBB->transferSuccessorsAndUpdatePHIs(MBB);

  MBB->addSuccessor(FalseMBB);
  MBB->addSuccessor(SinkMBB);
  BuildMI(MBB, DL, TII.get(SH::PseudoBT)).addMBB(SinkMBB);
  FalseMBB->addSuccessor(SinkMBB);

  BuildMI(*SinkMBB, SinkMBB->begin(), DL, TII.get(SH::PHI),
          MI.getOperand(0).getReg())
      .addReg(MI.getOperand(1).getReg())
      .addMBB(MBB)
      .addReg(MI.getOperand(2).getReg())
      .addMBB(FalseMBB);

  MI.eraseFromParent();
  return SinkMBB;
}

MachineBasicBlock *SuperHTargetLowering::EmitInstrWithCustomInserter(
    MachineInstr &MI, MachineBasicBlock *MBB) const {
  switch (MI.getOpcode()) {
  case SH::SELECT:
    return emitSelect(MI, MBB);
  default:
    llvm_unreachable("unexpected instruction for custom insertion");
  }
}

//===----------------------------------------------------------------------===//
// Registers and inline assembly
//===----------------------------------------------------------------------===//

Register
SuperHTargetLowering::getRegisterByName(const char *RegName, LLT VT,
                                        const MachineFunction &MF) const {
  Register Reg = StringSwitch<Register>(RegName)
                     .Cases({"r15", "sp"}, SH::R15)
                     .Case("r14", SH::R14)
                     .Case("gbr", SH::GBR)
                     .Default(Register());
  if (Reg)
    return Reg;
  report_fatal_error(Twine("invalid register name \"") + RegName + "\"");
}

TargetLowering::ConstraintType
SuperHTargetLowering::getConstraintType(StringRef Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
    case 'z':
      return C_RegisterClass;
    default:
      break;
    }
  }
  return TargetLowering::getConstraintType(Constraint);
}

std::pair<unsigned, const TargetRegisterClass *>
SuperHTargetLowering::getRegForInlineAsmConstraint(
    const TargetRegisterInfo *TRI, StringRef Constraint, MVT VT) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
      return {0U, &SH::GPRRegClass};
    case 'z':
      return {SH::R0, &SH::R0CRegClass};
    default:
      break;
    }
  }
  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}
