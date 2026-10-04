//===- SuperHISelDAGToDAG.cpp - A DAG to DAG instruction selector for SH -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the SuperH target.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "SuperH.h"
#include "SuperHInstrInfo.h"
#include "SuperHSubtarget.h"
#include "SuperHTargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "sh-isel"
#define PASS_NAME "SuperH DAG->DAG Pattern Instruction Selection"

namespace {

class SuperHDAGToDAGISel : public SelectionDAGISel {
  const SuperHSubtarget *Subtarget = nullptr;

public:
  SuperHDAGToDAGISel() = delete;

  SuperHDAGToDAGISel(SuperHTargetMachine &TM, CodeGenOptLevel OptLevel)
      : SelectionDAGISel(TM, OptLevel) {}

  bool runOnMachineFunction(MachineFunction &MF) override {
    Subtarget = &MF.getSubtarget<SuperHSubtarget>();
    return SelectionDAGISel::runOnMachineFunction(MF);
  }

  bool SelectInlineAsmMemoryOperand(const SDValue &Op,
                                    InlineAsm::ConstraintCode ConstraintID,
                                    std::vector<SDValue> &OutOps) override;

  template <unsigned Scale>
  bool selectAddrDisp(SDValue Addr, SDValue &Base, SDValue &Disp);
  bool selectAddrIndex(SDValue Addr, SDValue &Index, SDValue &Base);
  bool selectAddrFrame(SDValue Addr, SDValue &Base, SDValue &Offset);

#include "SuperHGenDAGISel.inc"

private:
  void Select(SDNode *N) override;
};

class SuperHDAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;
  explicit SuperHDAGToDAGISelLegacy(SuperHTargetMachine &TM,
                                    CodeGenOptLevel OptLevel)
      : SelectionDAGISelLegacy(
            ID, std::make_unique<SuperHDAGToDAGISel>(TM, OptLevel)) {}
};

} // end anonymous namespace

char SuperHDAGToDAGISelLegacy::ID = 0;

INITIALIZE_PASS(SuperHDAGToDAGISelLegacy, DEBUG_TYPE, PASS_NAME, false, false)

static bool isFrameIndexBased(SDValue Addr) {
  if (isa<FrameIndexSDNode>(Addr))
    return true;
  return Addr.getOpcode() == ISD::ADD && isa<FrameIndexSDNode>(Addr.getOperand(0));
}

template <unsigned Scale>
bool SuperHDAGToDAGISel::selectAddrDisp(SDValue Addr, SDValue &Base,
                                        SDValue &Disp) {
  if (!CurDAG->isBaseWithConstantOffset(Addr) || isFrameIndexBased(Addr))
    return false;
  int64_t Offset = cast<ConstantSDNode>(Addr.getOperand(1))->getSExtValue();
  if (Offset <= 0 || Offset > 15 * (int64_t)Scale || Offset % Scale)
    return false;
  Base = Addr.getOperand(0);
  Disp = CurDAG->getTargetConstant(Offset, SDLoc(Addr), MVT::i32);
  return true;
}

bool SuperHDAGToDAGISel::selectAddrIndex(SDValue Addr, SDValue &Index,
                                         SDValue &Base) {
  if (Addr.getOpcode() != ISD::ADD)
    return false;
  SDValue LHS = Addr.getOperand(0);
  SDValue RHS = Addr.getOperand(1);
  if (isa<ConstantSDNode>(RHS) || isa<ConstantSDNode>(LHS) ||
      isa<FrameIndexSDNode>(LHS) || isa<FrameIndexSDNode>(RHS) ||
      LHS.getOpcode() == SHISD::WRAPPER || RHS.getOpcode() == SHISD::WRAPPER)
    return false;
  Index = RHS;
  Base = LHS;
  return true;
}

bool SuperHDAGToDAGISel::selectAddrFrame(SDValue Addr, SDValue &Base,
                                         SDValue &Offset) {
  SDLoc DL(Addr);
  if (auto *FIN = dyn_cast<FrameIndexSDNode>(Addr)) {
    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i32);
    Offset = CurDAG->getTargetConstant(0, DL, MVT::i32);
    return true;
  }
  if (CurDAG->isBaseWithConstantOffset(Addr)) {
    if (auto *FIN = dyn_cast<FrameIndexSDNode>(Addr.getOperand(0))) {
      Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i32);
      Offset = CurDAG->getSignedTargetConstant(
          cast<ConstantSDNode>(Addr.getOperand(1))->getSExtValue(), DL,
          MVT::i32);
      return true;
    }
  }
  return false;
}

bool SuperHDAGToDAGISel::SelectInlineAsmMemoryOperand(
    const SDValue &Op, InlineAsm::ConstraintCode ConstraintID,
    std::vector<SDValue> &OutOps) {
  switch (ConstraintID) {
  case InlineAsm::ConstraintCode::m:
    OutOps.push_back(Op);
    return false;
  default:
    return true;
  }
}

void SuperHDAGToDAGISel::Select(SDNode *N) {
  if (N->isMachineOpcode()) {
    N->setNodeId(-1);
    return;
  }

  SDLoc DL(N);
  switch (N->getOpcode()) {
  case ISD::FrameIndex: {
    int FI = cast<FrameIndexSDNode>(N)->getIndex();
    SDValue TFI = CurDAG->getTargetFrameIndex(FI, MVT::i32);
    CurDAG->SelectNodeTo(N, SH::FRAMEADDR, MVT::i32, TFI,
                         CurDAG->getTargetConstant(0, DL, MVT::i32));
    return;
  }
  case ISD::ADD: {
    SDValue Base, Offset;
    if (selectAddrFrame(SDValue(N, 0), Base, Offset)) {
      CurDAG->SelectNodeTo(N, SH::FRAMEADDR, MVT::i32, Base, Offset);
      return;
    }
    break;
  }
  default:
    break;
  }

  SelectCode(N);
}

FunctionPass *llvm::createSuperHISelDag(SuperHTargetMachine &TM,
                                        CodeGenOptLevel OptLevel) {
  return new SuperHDAGToDAGISelLegacy(TM, OptLevel);
}
