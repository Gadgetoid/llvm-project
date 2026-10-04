//===-- SuperHFillDelaySlots.cpp - Fill SuperH branch delay slots ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Gives every instruction with a delay slot (bra, jmp, jsr, rts, ...) its slot
// instruction and bundles the pair. The slot gets an earlier instruction from
// the same block when one can be moved past everything in between without
// changing the result, and a nop otherwise.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "SuperH.h"
#include "SuperHInstrInfo.h"
#include "SuperHSubtarget.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineInstrBundle.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

#define DEBUG_TYPE "sh-fill-delay-slots"

static cl::opt<bool> DisableDelaySlotFiller(
    "sh-disable-delay-slot-filler", cl::Hidden, cl::init(false),
    cl::desc("Fill SuperH delay slots with nops only"));

static cl::opt<unsigned> DelaySlotSearchLimit(
    "sh-delay-slot-search-limit", cl::Hidden, cl::init(8),
    cl::desc("How many instructions to look back for a delay slot candidate"));

namespace {

class SuperHFillDelaySlots : public MachineFunctionPass {
  const SuperHInstrInfo *TII = nullptr;
  const TargetRegisterInfo *TRI = nullptr;

public:
  static char ID;

  SuperHFillDelaySlots() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().setNoVRegs();
  }

  StringRef getPassName() const override {
    return "SuperH delay slot filler";
  }

private:
  MachineInstr *findCandidate(MachineBasicBlock &MBB, MachineInstr &Branch,
                              SmallVectorImpl<MachineInstr *> &Between);
  bool canMovePast(const MachineInstr &Candidate, const MachineInstr &Other);
  bool conflictsWithBranch(const MachineInstr &Candidate,
                           const MachineInstr &Branch);
};

} // end anonymous namespace

char SuperHFillDelaySlots::ID = 0;

INITIALIZE_PASS(SuperHFillDelaySlots, DEBUG_TYPE, "SuperH delay slot filler",
                false, false)

static bool readsOrWrites(const MachineInstr &MI, Register Reg,
                          const TargetRegisterInfo *TRI) {
  return MI.readsRegister(Reg, TRI) || MI.modifiesRegister(Reg, TRI);
}

// Whether Candidate, which comes before Other, can be moved after it.
bool SuperHFillDelaySlots::canMovePast(const MachineInstr &Candidate,
                                       const MachineInstr &Other) {
  if (Other.isDebugInstr())
    return true;
  if (Other.isCall() || Other.isTerminator() || Other.isInlineAsm() ||
      Other.hasUnmodeledSideEffects() || Other.isBundle())
    return false;
  if ((Candidate.mayStore() && (Other.mayLoad() || Other.mayStore())) ||
      (Candidate.mayLoad() && Other.mayStore()))
    return false;
  for (const MachineOperand &MO : Candidate.operands()) {
    if (!MO.isReg() || !MO.getReg())
      continue;
    Register Reg = MO.getReg();
    if (MO.isDef() && readsOrWrites(Other, Reg, TRI))
      return false;
    if (MO.isUse() && Other.modifiesRegister(Reg, TRI))
      return false;
  }
  return true;
}

// Whether placing Candidate in Branch's delay slot changes what the branch
// does. The slot still runs before control reaches the target, but after the
// branch has read the registers that decide the target: jmp, jsr and braf
// read their operand, rts reads PR, and jsr writes PR before the slot.
bool SuperHFillDelaySlots::conflictsWithBranch(const MachineInstr &Candidate,
                                               const MachineInstr &Branch) {
  for (const MachineOperand &MO : Branch.explicit_operands())
    if (MO.isReg() && MO.getReg() && Candidate.modifiesRegister(MO.getReg(), TRI))
      return true;
  if (Branch.readsRegister(SH::SR, TRI) &&
      Candidate.modifiesRegister(SH::SR, TRI))
    return true;
  return Candidate.readsRegister(SH::PR, TRI) ||
         Candidate.modifiesRegister(SH::PR, TRI);
}

MachineInstr *
SuperHFillDelaySlots::findCandidate(MachineBasicBlock &MBB,
                                    MachineInstr &Branch,
                                    SmallVectorImpl<MachineInstr *> &Between) {
  if (DisableDelaySlotFiller)
    return nullptr;
  MachineBasicBlock::iterator I = Branch.getIterator();
  for (unsigned Count = 0; I != MBB.begin() && Count < DelaySlotSearchLimit;) {
    --I;
    MachineInstr &MI = *I;
    if (MI.isDebugInstr())
      continue;
    ++Count;
    if (MI.isBundled() || MI.isBundle())
      return nullptr;
    // Keep the prologue in the shape the standard's unwinder expects.
    if (MI.getFlag(MachineInstr::FrameSetup))
      return nullptr;
    if (TII->isDelaySlotCandidate(MI) && !conflictsWithBranch(MI, Branch) &&
        all_of(Between, [&](MachineInstr *Other) {
          return canMovePast(MI, *Other);
        }))
      return &MI;
    if (MI.isCall() || MI.isTerminator() || MI.isInlineAsm() ||
        MI.hasUnmodeledSideEffects() || MI.isLabel())
      return nullptr;
    Between.push_back(&MI);
  }
  return nullptr;
}

bool SuperHFillDelaySlots::runOnMachineFunction(MachineFunction &MF) {
  TII = MF.getSubtarget<SuperHSubtarget>().getInstrInfo();
  TRI = MF.getSubtarget().getRegisterInfo();
  bool Changed = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineBasicBlock::instr_iterator I = MBB.instr_begin();
         I != MBB.instr_end(); ++I) {
      MachineInstr &Branch = *I;
      if (!Branch.hasDelaySlot() && Branch.getOpcode() != SH::PseudoBR)
        continue;
      if (Branch.isBundledWithSucc())
        continue;

      SmallVector<MachineInstr *, 8> Between;
      MachineInstr *Slot = findCandidate(MBB, Branch, Between);
      if (Slot) {
        Slot->removeFromParent();
        MBB.insertAfter(Branch.getIterator(), Slot);
        // The slot now reads its registers after everything it was moved
        // past, so none of those is their last use any more.
        Between.push_back(&Branch);
        for (const MachineOperand &MO : Slot->operands())
          if (MO.isReg() && MO.isUse() && MO.getReg())
            for (MachineInstr *Other : Between)
              Other->clearRegisterKills(MO.getReg(), TRI);
        // A call's argument registers are read by the callee, after the
        // slot; the call itself no longer sees a value from before it.
        for (MachineOperand &MO : Branch.implicit_operands())
          if (MO.isReg() && MO.isUse() &&
              Slot->modifiesRegister(MO.getReg(), TRI)) {
            MO.setIsUndef(true);
            MO.setIsKill(false);
          }
      } else {
        BuildMI(MBB, std::next(Branch.getIterator()), Branch.getDebugLoc(),
                TII->get(SH::NOP));
      }
      finalizeBundle(MBB, Branch.getIterator(),
                     std::next(Branch.getIterator(), 2));
      Changed = true;
      ++I;
    }
  }
  return Changed;
}

FunctionPass *llvm::createSuperHFillDelaySlotsPass() {
  return new SuperHFillDelaySlots();
}
