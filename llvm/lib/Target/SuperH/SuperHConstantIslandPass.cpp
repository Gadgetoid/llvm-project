//===- SuperHConstantIslandPass.cpp - SuperH literal pools and branches ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Lays out the final code of a function:
//
// - LOADCONST becomes a PC-relative mov.l from a literal pool. The literal must
//   be at most 1020 bytes ahead of the load, so pools are placed in the code:
//   after an unconditional branch or return where possible, otherwise behind a
//   branch around the pool.
// - bt and bf reach 256 bytes. Further conditional branches become an inverted
//   branch over an unconditional one.
// - bra reaches 4 KB. Further jumps go through a register:
//     mov.l r0,@-r15; mov.l .Ltarget,r0; jmp @r0; mov.l @r15+,r0
//
// Placing pools moves code, which can put branches or earlier loads out of
// range again, so both steps repeat until nothing changes.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "SuperH.h"
#include "SuperHInstrInfo.h"
#include "SuperHSubtarget.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineInstrBundle.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "sh-constant-islands"

namespace {

class SuperHConstantIslands : public MachineFunctionPass {
  MachineFunction *MF = nullptr;
  const SuperHInstrInfo *TII = nullptr;

  DenseMap<const MachineInstr *, unsigned> Offsets;
  SmallVector<unsigned, 16> BlockOffsets;
  SmallVector<unsigned, 16> BlockEnds;

  // The literal each LOADCONST currently loads from.
  DenseMap<MachineInstr *, MachineInstr *> Assigned;

  // Blocks holding literal pools.
  SmallPtrSet<MachineBasicBlock *, 8> Pools;

public:
  static char ID;

  SuperHConstantIslands() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().setNoVRegs();
  }

  StringRef getPassName() const override {
    return "SuperH literal pools and branch relaxation";
  }

private:
  void computeOffsets();
  unsigned getOffset(const MachineInstr &MI) const { return Offsets.lookup(&MI); }
  unsigned getEnd(const MachineInstr &MI) const {
    return getOffset(MI) + TII->getInstSizeInBytes(MI);
  }

  bool relaxBranches();
  bool relaxConditionalBranch(MachineInstr &Branch);
  bool relaxUnconditionalBranch(MachineInstr &Bundle);

  bool placeLiterals();
  bool isLiteralInRange(const MachineInstr &User,
                        const MachineInstr &Literal) const;
  MachineBasicBlock *createPool(MachineInstr &FirstUser, unsigned Deadline);
  MachineBasicBlock *splitWithJumpAround(MachineInstr &After);

  void lower();
};

} // end anonymous namespace

char SuperHConstantIslands::ID = 0;

INITIALIZE_PASS(SuperHConstantIslands, DEBUG_TYPE,
                "SuperH literal pools and branch relaxation", false, false)

static bool isBarrierBundle(const MachineInstr &MI) {
  if (!MI.isBundle())
    return false;
  const MachineInstr &Branch = *std::next(MI.getIterator());
  return Branch.isBarrier() || Branch.isReturn() ||
         Branch.getOpcode() == SH::PseudoBR;
}

static bool sameValue(const MachineOperand &A, const MachineOperand &B) {
  if (A.getType() != B.getType())
    return false;
  switch (A.getType()) {
  case MachineOperand::MO_Immediate:
    return A.getImm() == B.getImm();
  case MachineOperand::MO_GlobalAddress:
    return A.getGlobal() == B.getGlobal() && A.getOffset() == B.getOffset() &&
           A.getTargetFlags() == B.getTargetFlags();
  case MachineOperand::MO_ExternalSymbol:
    return StringRef(A.getSymbolName()) == B.getSymbolName() &&
           A.getOffset() == B.getOffset();
  case MachineOperand::MO_MachineBasicBlock:
    return A.getMBB() == B.getMBB();
  default:
    return A.isIdenticalTo(B);
  }
}

void SuperHConstantIslands::computeOffsets() {
  Offsets.clear();
  MF->RenumberBlocks();
  BlockOffsets.assign(MF->getNumBlockIDs(), 0);
  BlockEnds.assign(MF->getNumBlockIDs(), 0);
  unsigned Offset = 0;
  for (MachineBasicBlock &MBB : *MF) {
    Offset = alignTo(Offset, MBB.getAlignment());
    BlockOffsets[MBB.getNumber()] = Offset;
    for (MachineInstr &MI : MBB) {
      Offsets[&MI] = Offset;
      Offset += TII->getInstSizeInBytes(MI);
    }
    BlockEnds[MBB.getNumber()] = Offset;
  }
}

//===----------------------------------------------------------------------===//
// Branches
//===----------------------------------------------------------------------===//

// Drops successors that a block neither branches to nor falls through to.
static void pruneSuccessors(MachineBasicBlock &MBB) {
  SmallPtrSet<MachineBasicBlock *, 4> Reached;
  for (MachineInstr &MI : MBB.instrs())
    for (MachineOperand &MO : MI.operands())
      if (MO.isMBB())
        Reached.insert(MO.getMBB());
  bool FallsThrough = MBB.empty() || !(MBB.back().isBarrier() ||
                                       isBarrierBundle(MBB.back()));
  MachineFunction::iterator Next = std::next(MBB.getIterator());
  if (FallsThrough && Next != MBB.getParent()->end())
    Reached.insert(&*Next);
  SmallVector<MachineBasicBlock *, 4> Unreached;
  for (MachineBasicBlock *Successor : MBB.successors())
    if (!Reached.count(Successor))
      Unreached.push_back(Successor);
  for (MachineBasicBlock *Successor : Unreached)
    MBB.removeSuccessor(Successor);
}

static MachineInstr &getBundledBranch(MachineInstr &Bundle) {
  return *std::next(Bundle.getIterator());
}

static void buildBranchBundle(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator Where,
                              MachineBasicBlock *Target,
                              const SuperHInstrInfo &TII, const DebugLoc &DL) {
  MachineInstr *Branch =
      BuildMI(MBB, Where, DL, TII.get(SH::PseudoBR)).addMBB(Target);
  BuildMI(MBB, Where, DL, TII.get(SH::NOP));
  finalizeBundle(MBB, Branch->getIterator(),
                 std::next(Branch->getIterator(), 2));
}

bool SuperHConstantIslands::relaxConditionalBranch(MachineInstr &Branch) {
  MachineBasicBlock &MBB = *Branch.getParent();
  MachineBasicBlock *Target = Branch.getOperand(0).getMBB();
  DebugLoc DL = Branch.getDebugLoc();

  // MBB: ... bt Target; rest      becomes
  // MBB: ... bf Rest; bra Target; nop
  // Rest: rest
  MachineBasicBlock *Rest = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
  MF->insert(std::next(MBB.getIterator()), Rest);
  Rest->splice(Rest->end(), &MBB,
               std::next(MachineBasicBlock::iterator(Branch)), MBB.end());
  Rest->transferSuccessors(&MBB);
  MBB.addSuccessor(Rest);
  MBB.addSuccessor(Target);
  LivePhysRegs LiveRegs;
  computeAndAddLiveIns(LiveRegs, *Rest);

  unsigned Inverted =
      Branch.getOpcode() == SH::PseudoBT ? SH::PseudoBF : SH::PseudoBT;
  BuildMI(&MBB, DL, TII->get(Inverted)).addMBB(Rest);
  buildBranchBundle(MBB, MBB.end(), Target, *TII, DL);
  Branch.eraseFromParent();
  pruneSuccessors(*Rest);
  return true;
}

bool SuperHConstantIslands::relaxUnconditionalBranch(MachineInstr &Bundle) {
  // The long sequence isn't a terminator, so it needs a block of its own if
  // a conditional branch comes before it.
  MachineBasicBlock *Parent = Bundle.getParent();
  if (MachineBasicBlock::iterator(Bundle) != Parent->getFirstTerminator()) {
    MachineBasicBlock *Own =
        MF->CreateMachineBasicBlock(Parent->getBasicBlock());
    MF->insert(std::next(Parent->getIterator()), Own);
    Own->splice(Own->end(), Parent, MachineBasicBlock::iterator(Bundle),
                Parent->end());
    Own->transferSuccessors(Parent);
    Parent->addSuccessor(Own);
    for (MachineInstr &MI : Parent->terminators())
      for (MachineOperand &MO : MI.operands())
        if (MO.isMBB() && !Parent->isSuccessor(MO.getMBB()))
          Parent->addSuccessor(MO.getMBB());
    pruneSuccessors(*Own);
    LivePhysRegs LiveRegs;
    computeAndAddLiveIns(LiveRegs, *Own);
  }
  MachineBasicBlock &MBB = *Bundle.getParent();
  MachineInstr &Branch = getBundledBranch(Bundle);
  MachineInstr &Slot = *std::next(Branch.getIterator());
  MachineBasicBlock *Target = Branch.getOperand(0).getMBB();
  DebugLoc DL = Branch.getDebugLoc();

  // The slot instruction runs before the jump either way, so it can go first.
  Slot.unbundleFromPred();
  Branch.unbundleFromPred();
  MachineBasicBlock::iterator Where = std::next(Slot.getIterator());
  Bundle.eraseFromParent();
  Branch.eraseFromParent();
  if (Slot.getOpcode() == SH::NOP)
    Slot.eraseFromParent();

  Target->setLabelMustBeEmitted();
  BuildMI(MBB, Where, DL, TII->get(SH::STLP), SH::R15)
      .addReg(SH::R0, RegState::Undef)
      .addReg(SH::R15);
  BuildMI(MBB, Where, DL, TII->get(SH::LOADCONST), SH::R0).addMBB(Target);
  MachineInstr *Jump =
      BuildMI(MBB, Where, DL, TII->get(SH::JMP)).addReg(SH::R0, RegState::Kill);
  BuildMI(MBB, Where, DL, TII->get(SH::LDLP), SH::R0)
      .addReg(SH::R15, RegState::Define)
      .addReg(SH::R15);
  finalizeBundle(MBB, Jump->getIterator(), std::next(Jump->getIterator(), 2));
  return true;
}

bool SuperHConstantIslands::relaxBranches() {
  for (MachineBasicBlock &MBB : *MF) {
    for (MachineInstr &MI : MBB) {
      if (MI.getOpcode() == SH::PseudoBT || MI.getOpcode() == SH::PseudoBF) {
        int64_t Distance =
            int64_t(BlockOffsets[MI.getOperand(0).getMBB()->getNumber()]) -
            int64_t(getOffset(MI) + 4);
        if (Distance < -256 || Distance > 254)
          return relaxConditionalBranch(MI);
        continue;
      }
      if (MI.isBundle() &&
          getBundledBranch(MI).getOpcode() == SH::PseudoBR) {
        MachineInstr &Branch = getBundledBranch(MI);
        int64_t Distance =
            int64_t(BlockOffsets[Branch.getOperand(0).getMBB()->getNumber()]) -
            int64_t(getOffset(MI) + 4);
        if (Distance < -4096 || Distance > 4094)
          return relaxUnconditionalBranch(MI);
      }
    }
  }
  return false;
}

//===----------------------------------------------------------------------===//
// Literal pools
//===----------------------------------------------------------------------===//

static unsigned getDeadline(unsigned UserOffset) {
  return alignDown(UserOffset, 4) + 4 + 1020;
}

bool SuperHConstantIslands::isLiteralInRange(
    const MachineInstr &User, const MachineInstr &Literal) const {
  unsigned UserOffset = getOffset(User);
  unsigned LiteralOffset = getOffset(Literal);
  return LiteralOffset >= alignDown(UserOffset, 4) + 4 &&
         LiteralOffset <= getDeadline(UserOffset) && LiteralOffset % 4 == 0;
}

// Splits the code after \p After, which falls through, with a branch around
// a new pool block. Returns the pool block.
MachineBasicBlock *
SuperHConstantIslands::splitWithJumpAround(MachineInstr &After) {
  MachineBasicBlock &MBB = *After.getParent();
  DebugLoc DL = After.getDebugLoc();
  MachineBasicBlock *Rest;
  MachineBasicBlock::iterator Next = std::next(MachineBasicBlock::iterator(After));
  if (Next == MBB.end() && std::next(MBB.getIterator()) == MF->end()) {
    // Nothing follows the end of the function, such as a noreturn call.
    MachineBasicBlock *Pool = MF->CreateMachineBasicBlock();
    MF->push_back(Pool);
    return Pool;
  }
  if (Next == MBB.end()) {
    Rest = &*std::next(MBB.getIterator());
  } else {
    Rest = MF->CreateMachineBasicBlock(MBB.getBasicBlock());
    MF->insert(std::next(MBB.getIterator()), Rest);
    Rest->splice(Rest->end(), &MBB, Next, MBB.end());
    Rest->transferSuccessors(&MBB);
    MBB.addSuccessor(Rest);
    LivePhysRegs LiveRegs;
    computeAndAddLiveIns(LiveRegs, *Rest);
  }
  buildBranchBundle(MBB, MBB.end(), Rest, *TII, DL);

  MachineBasicBlock *Pool = MF->CreateMachineBasicBlock();
  MF->insert(std::next(MBB.getIterator()), Pool);
  return Pool;
}

MachineBasicBlock *SuperHConstantIslands::createPool(MachineInstr &FirstUser,
                                                     unsigned Deadline) {
  unsigned UserOffset = getOffset(FirstUser);

  // The pool also takes a literal for each later user ahead of it that has
  // none in range. Those users' deadlines are no earlier than the first
  // user's, so the pool holds them all if it starts early enough for as many
  // literals as there are values.
  SmallVector<const MachineOperand *, 16> Values;
  auto countUser = [&](MachineInstr &MI) {
    if (MI.getOpcode() != SH::LOADCONST || getOffset(MI) < UserOffset)
      return;
    auto It = Assigned.find(&MI);
    if (&MI != &FirstUser && It != Assigned.end() &&
        isLiteralInRange(MI, *It->second))
      return;
    if (none_of(Values, [&](const MachineOperand *Value) {
          return sameValue(*Value, MI.getOperand(1));
        }))
      Values.push_back(&MI.getOperand(1));
  };

  // Prefer the last block, at or after the user, that ends in a barrier and
  // leaves room for the literals after any pools already behind it.
  MachineFunction::iterator Where = MF->end();
  bool Found = false;
  for (MachineBasicBlock &MBB : *MF) {
    for (MachineInstr &MI : MBB)
      countUser(MI);
    unsigned End = BlockEnds[MBB.getNumber()];
    if (End <= UserOffset || MBB.empty() || Pools.count(&MBB) ||
        !isBarrierBundle(MBB.back()))
      continue;
    MachineFunction::iterator After = std::next(MBB.getIterator());
    while (After != MF->end() && Pools.count(&*After)) {
      End = BlockEnds[After->getNumber()];
      ++After;
    }
    if (alignTo(End, 4) + 4 * Values.size() > Deadline)
      break;
    Where = After;
    Found = true;
  }
  if (Found) {
    MachineBasicBlock *Pool = MF->CreateMachineBasicBlock();
    MF->insert(Where, Pool);
    return Pool;
  }

  // Otherwise split the code at the last point that leaves room for the
  // branch around the pool, its alignment and the literals.
  Values.clear();
  MachineInstr *SplitAfter = &FirstUser;
  for (MachineBasicBlock &MBB : *MF) {
    for (MachineInstr &MI : MBB) {
      countUser(MI);
      unsigned End = getEnd(MI);
      if (End <= UserOffset)
        continue;
      if (End + 4 + 2 + 4 * Values.size() > Deadline)
        break;
      if (MI.isTerminator() &&
          std::next(MachineBasicBlock::iterator(MI)) != MBB.end())
        continue;
      if (MI.isBundle() && isBarrierBundle(MI))
        continue;
      SplitAfter = &MI;
    }
  }
  return splitWithJumpAround(*SplitAfter);
}

bool SuperHConstantIslands::placeLiterals() {
  SmallVector<MachineInstr *, 32> Users;
  SmallVector<MachineInstr *, 32> Literals;
  for (MachineBasicBlock &MBB : *MF)
    for (MachineInstr &MI : MBB) {
      if (MI.getOpcode() == SH::LOADCONST)
        Users.push_back(&MI);
      else if (MI.getOpcode() == SH::LITERAL)
        Literals.push_back(&MI);
    }

  for (unsigned Index = 0, E = Users.size(); Index != E; ++Index) {
    MachineInstr *User = Users[Index];
    auto It = Assigned.find(User);
    if (It != Assigned.end() && isLiteralInRange(*User, *It->second))
      continue;

    // Reuse a literal with the same value that is in range.
    MachineInstr *Reuse = nullptr;
    for (MachineInstr *Literal : Literals)
      if (sameValue(Literal->getOperand(1), User->getOperand(1)) &&
          isLiteralInRange(*User, *Literal)) {
        Reuse = Literal;
        break;
      }
    if (Reuse) {
      Assigned[User] = Reuse;
      continue;
    }

    // Append to a pool that is ahead of the user and in range. Adding to
    // the end of a pool doesn't move the literals already in it.
    unsigned UserOffset = getOffset(*User);
    MachineBasicBlock *Existing = nullptr;
    for (MachineBasicBlock *Pool : Pools) {
      unsigned Start = BlockOffsets[Pool->getNumber()];
      unsigned End = BlockEnds[Pool->getNumber()];
      if (Start > UserOffset && End <= getDeadline(UserOffset) &&
          (!Existing || Start > BlockOffsets[Existing->getNumber()]))
        Existing = Pool;
    }
    if (Existing) {
      MCSymbol *Label = MF->getContext().createTempSymbol("lit", true);
      Assigned[User] = BuildMI(*Existing, Existing->end(), DebugLoc(),
                               TII->get(SH::LITERAL))
                           .addSym(Label)
                           .add(User->getOperand(1));
      return true;
    }

    // Start a new pool for this user and the ones after it.
    unsigned Deadline = getDeadline(getOffset(*User));
    LLVM_DEBUG(dbgs() << "user at " << getOffset(*User) << " deadline "
                      << Deadline << ": " << *User);
    MachineBasicBlock *Pool = createPool(*User, Deadline);
    Pool->setAlignment(Align(4));
    Pools.insert(Pool);
    computeOffsets();
    unsigned PoolStart = BlockOffsets[Pool->getNumber()];
    DebugLoc DL;
    SmallVector<MachineInstr *, 8> Entries;
    for (unsigned Next = Index; Next != E; ++Next) {
      MachineInstr *Candidate = Users[Next];
      unsigned CandidateOffset = getOffset(*Candidate);
      if (CandidateOffset >= PoolStart)
        break;
      auto Current = Assigned.find(Candidate);
      if (Next != Index && Current != Assigned.end() &&
          isLiteralInRange(*Candidate, *Current->second))
        continue;
      MachineInstr *Entry = nullptr;
      for (MachineInstr *Existing : Entries)
        if (sameValue(Existing->getOperand(1), Candidate->getOperand(1)))
          Entry = Existing;
      if (!Entry) {
        unsigned EntryOffset = PoolStart + 4 * Entries.size();
        if (EntryOffset > getDeadline(CandidateOffset))
          continue;
        MCSymbol *Label = MF->getContext().createTempSymbol("lit", true);
        Entry = BuildMI(*Pool, Pool->end(), DL, TII->get(SH::LITERAL))
                    .addSym(Label)
                    .add(Candidate->getOperand(1));
        Entries.push_back(Entry);
      }
      Assigned[Candidate] = Entry;
    }
    return true;
  }
  return false;
}

//===----------------------------------------------------------------------===//
// Final lowering
//===----------------------------------------------------------------------===//

void SuperHConstantIslands::lower() {
  SmallPtrSet<MachineInstr *, 16> UsedLiterals;
  for (MachineBasicBlock &MBB : *MF) {
    for (MachineBasicBlock::instr_iterator I = MBB.instr_begin(),
                                           E = MBB.instr_end();
         I != E;) {
      MachineInstr &MI = *I++;
      switch (MI.getOpcode()) {
      case SH::LOADCONST: {
        MachineInstr *Literal = Assigned.lookup(&MI);
        assert(Literal && "literal load without a pool entry");
        UsedLiterals.insert(Literal);
        BuildMI(MBB, MI.getIterator(), MI.getDebugLoc(), TII->get(SH::LDLPC),
                MI.getOperand(0).getReg())
            .addSym(Literal->getOperand(0).getMCSymbol());
        MI.eraseFromParent();
        break;
      }
      case SH::PseudoBT:
        MI.setDesc(TII->get(SH::BT));
        break;
      case SH::PseudoBF:
        MI.setDesc(TII->get(SH::BF));
        break;
      case SH::PseudoBR:
        MI.setDesc(TII->get(SH::BRA));
        break;
      default:
        break;
      }
    }
  }

  for (MachineBasicBlock &MBB : *MF)
    for (MachineInstr &MI : llvm::make_early_inc_range(MBB))
      if (MI.getOpcode() == SH::LITERAL && !UsedLiterals.count(&MI))
        MI.eraseFromParent();
}

bool SuperHConstantIslands::runOnMachineFunction(MachineFunction &Fn) {
  MF = &Fn;
  TII = Fn.getSubtarget<SuperHSubtarget>().getInstrInfo();
  Assigned.clear();
  Pools.clear();

  bool Changed;
  unsigned Iterations = 0;
  do {
    if (++Iterations > 1000)
      report_fatal_error("SuperH: literal pool placement did not converge");
    computeOffsets();
    Changed = relaxBranches();
    if (Changed) {
      LLVM_DEBUG(dbgs() << "relaxed a branch\n");
      continue;
    }
    Changed = placeLiterals();
    LLVM_DEBUG(if (Changed) dbgs() << "placed a pool\n");
  } while (Changed);

  lower();
  return true;
}

FunctionPass *llvm::createSuperHConstantIslandPass() {
  return new SuperHConstantIslands();
}
