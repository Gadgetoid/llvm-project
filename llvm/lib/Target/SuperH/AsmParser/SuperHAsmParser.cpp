//===-- SuperHAsmParser.cpp - Parse SuperH assembly to MCInst -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Parses GNU-syntax SuperH assembly. Memory operands are split into tokens
// ("@", "(", ")", "+", "-") and operands, the way the instruction definitions
// spell them, so "mov.l @(4,r1),r2" becomes: mov.l @ ( 4 r1 ) r2.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "TargetInfo/SuperHTargetInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCParser/AsmLexer.h"
#include "llvm/MC/MCParser/MCAsmParser.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"

using namespace llvm;

#define DEBUG_TYPE "sh-asm-parser"

// The generated matcher uses the target name as its namespace.
namespace llvm {
namespace SuperH {
using namespace SH;
} // namespace SuperH
} // namespace llvm

namespace {

class SuperHOperand : public MCParsedAsmOperand {
  enum KindTy { Token, Register, Immediate } Kind;
  SMLoc StartLoc, EndLoc;
  StringRef Tok;
  MCRegister Reg;
  const MCExpr *Imm = nullptr;

public:
  SuperHOperand(KindTy Kind, SMLoc S, SMLoc E)
      : Kind(Kind), StartLoc(S), EndLoc(E) {}

  static std::unique_ptr<SuperHOperand> createToken(StringRef Str, SMLoc S) {
    auto Op = std::make_unique<SuperHOperand>(Token, S, S);
    Op->Tok = Str;
    return Op;
  }
  static std::unique_ptr<SuperHOperand> createReg(MCRegister Reg, SMLoc S,
                                                  SMLoc E) {
    auto Op = std::make_unique<SuperHOperand>(Register, S, E);
    Op->Reg = Reg;
    return Op;
  }
  static std::unique_ptr<SuperHOperand> createImm(const MCExpr *Value,
                                                  SMLoc S, SMLoc E) {
    auto Op = std::make_unique<SuperHOperand>(Immediate, S, E);
    Op->Imm = Value;
    return Op;
  }

  bool isToken() const override { return Kind == Token; }
  bool isReg() const override { return Kind == Register; }
  bool isImm() const override { return Kind == Immediate; }
  bool isMem() const override { return false; }

  StringRef getToken() const {
    assert(isToken());
    return Tok;
  }
  MCRegister getReg() const override {
    assert(isReg());
    return Reg;
  }
  const MCExpr *getImm() const {
    assert(isImm());
    return Imm;
  }

  SMLoc getStartLoc() const override { return StartLoc; }
  SMLoc getEndLoc() const override { return EndLoc; }

  bool getConstant(int64_t &Value) const {
    if (!isImm())
      return false;
    if (const auto *CE = dyn_cast<MCConstantExpr>(Imm)) {
      Value = CE->getValue();
      return true;
    }
    return false;
  }

  template <unsigned Bits> bool isSImm() const {
    int64_t Value;
    return getConstant(Value) && isInt<Bits>(Value);
  }

  template <unsigned Bits> bool isUImm() const {
    int64_t Value;
    return getConstant(Value) && isUInt<Bits>(Value);
  }

  template <unsigned Bits, unsigned Scale> bool isDisp() const {
    int64_t Value;
    return getConstant(Value) && Value >= 0 && Value % Scale == 0 &&
           isUInt<Bits>(Value / Scale);
  }

  bool isBranchTarget() const { return isImm(); }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    int64_t Value;
    if (getConstant(Value))
      Inst.addOperand(MCOperand::createImm(Value));
    else
      Inst.addOperand(MCOperand::createExpr(getImm()));
  }

  void print(raw_ostream &OS, const MCAsmInfo &MAI) const override {
    switch (Kind) {
    case Token:
      OS << "Token: " << Tok;
      break;
    case Register:
      OS << "Register: " << Reg.id();
      break;
    case Immediate:
      OS << "Immediate: ";
      MAI.printExpr(OS, *Imm);
      break;
    }
  }
};

class SuperHAsmParser : public MCTargetAsmParser {
#define GET_ASSEMBLER_HEADER
#include "SuperHGenAsmMatcher.inc"

  bool parseRegister(MCRegister &Reg, SMLoc &StartLoc, SMLoc &EndLoc) override;
  ParseStatus tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                               SMLoc &EndLoc) override;
  bool parseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;
  bool matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

  bool parseOperand(OperandVector &Operands);
  bool parseMemoryOperand(OperandVector &Operands);
  MCRegister matchRegister(StringRef Name) const;

public:
  SuperHAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                  const MCInstrInfo &MII)
      : MCTargetAsmParser(STI, MII) {
    setAvailableFeatures(ComputeAvailableFeatures(STI.getFeatureBits()));
  }
};

} // end anonymous namespace

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "SuperHGenAsmMatcher.inc"

MCRegister SuperHAsmParser::matchRegister(StringRef Name) const {
  std::string Lower = Name.lower();
  if (Lower == "sp")
    return SH::R15;
  return MatchRegisterName(Lower);
}

ParseStatus SuperHAsmParser::tryParseRegister(MCRegister &Reg,
                                              SMLoc &StartLoc,
                                              SMLoc &EndLoc) {
  const AsmToken &Tok = getTok();
  if (Tok.isNot(AsmToken::Identifier))
    return ParseStatus::NoMatch;
  Reg = matchRegister(Tok.getIdentifier());
  if (!Reg)
    return ParseStatus::NoMatch;
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  Lex();
  return ParseStatus::Success;
}

bool SuperHAsmParser::parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                    SMLoc &EndLoc) {
  if (!tryParseRegister(Reg, StartLoc, EndLoc).isSuccess())
    return Error(StartLoc, "invalid register name");
  return false;
}

// Parses "@Rn", "@Rn+", "@-Rn" and "@(a,b)". "@(disp,pc)" becomes a single
// PC-relative operand, as the label form would.
bool SuperHAsmParser::parseMemoryOperand(OperandVector &Operands) {
  SMLoc AtLoc = getTok().getLoc();
  Lex();

  if (getTok().is(AsmToken::LParen)) {
    SMLoc ParenLoc = getTok().getLoc();
    Lex();
    SmallVector<std::unique_ptr<SuperHOperand>, 2> Inner;
    for (unsigned Index = 0; Index < 2; ++Index) {
      if (Index && parseToken(AsmToken::Comma, "expected ','"))
        return true;
      SMLoc S = getTok().getLoc();
      MCRegister Reg;
      SMLoc RS, RE;
      if (tryParseRegister(Reg, RS, RE).isSuccess()) {
        Inner.push_back(SuperHOperand::createReg(Reg, RS, RE));
        continue;
      }
      const MCExpr *Expr;
      SMLoc E;
      if (getParser().parseExpression(Expr, E))
        return true;
      Inner.push_back(SuperHOperand::createImm(Expr, S, E));
    }
    SMLoc CloseLoc = getTok().getLoc();
    if (parseToken(AsmToken::RParen, "expected ')'"))
      return true;
    if (Inner[1]->isReg() && Inner[1]->getReg() == SH::PC &&
        Inner[0]->isImm()) {
      Operands.push_back(std::move(Inner[0]));
      return false;
    }
    Operands.push_back(SuperHOperand::createToken("@", AtLoc));
    Operands.push_back(SuperHOperand::createToken("(", ParenLoc));
    Operands.push_back(std::move(Inner[0]));
    Operands.push_back(std::move(Inner[1]));
    Operands.push_back(SuperHOperand::createToken(")", CloseLoc));
    return false;
  }

  Operands.push_back(SuperHOperand::createToken("@", AtLoc));
  if (getTok().is(AsmToken::Minus)) {
    Operands.push_back(SuperHOperand::createToken("-", getTok().getLoc()));
    Lex();
  }
  MCRegister Reg;
  SMLoc S, E;
  if (!tryParseRegister(Reg, S, E).isSuccess())
    return Error(getTok().getLoc(), "expected register");
  Operands.push_back(SuperHOperand::createReg(Reg, S, E));
  if (getTok().is(AsmToken::Plus)) {
    Operands.push_back(SuperHOperand::createToken("+", getTok().getLoc()));
    Lex();
  }
  return false;
}

bool SuperHAsmParser::parseOperand(OperandVector &Operands) {
  if (getTok().is(AsmToken::At))
    return parseMemoryOperand(Operands);

  if (getTok().is(AsmToken::Hash)) {
    Lex();
    SMLoc S = getTok().getLoc();
    const MCExpr *Expr;
    SMLoc E;
    if (getParser().parseExpression(Expr, E))
      return true;
    Operands.push_back(SuperHOperand::createImm(Expr, S, E));
    return false;
  }

  MCRegister Reg;
  SMLoc S, E;
  if (tryParseRegister(Reg, S, E).isSuccess()) {
    Operands.push_back(SuperHOperand::createReg(Reg, S, E));
    return false;
  }

  S = getTok().getLoc();
  const MCExpr *Expr;
  if (getParser().parseExpression(Expr, E))
    return true;
  Operands.push_back(SuperHOperand::createImm(Expr, S, E));
  return false;
}

bool SuperHAsmParser::parseInstruction(ParseInstructionInfo &Info,
                                       StringRef Name, SMLoc NameLoc,
                                       OperandVector &Operands) {
  // Mnemonics such as cmp/eq and bt/s reach us in pieces.
  std::string Mnemonic = Name.lower();
  while (getTok().is(AsmToken::Slash)) {
    Lex();
    if (getTok().isNot(AsmToken::Identifier))
      return Error(getTok().getLoc(), "expected mnemonic suffix");
    Mnemonic += "/" + getTok().getIdentifier().lower();
    Lex();
  }
  Operands.push_back(SuperHOperand::createToken(
      getContext().allocateString(Mnemonic), NameLoc));

  if (getTok().is(AsmToken::EndOfStatement)) {
    Lex();
    return false;
  }

  if (parseOperand(Operands))
    return true;
  while (getTok().is(AsmToken::Comma)) {
    Lex();
    if (parseOperand(Operands))
      return true;
  }
  if (getTok().isNot(AsmToken::EndOfStatement))
    return Error(getTok().getLoc(), "unexpected token in operand list");
  Lex();
  return false;
}

bool SuperHAsmParser::matchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                              OperandVector &Operands,
                                              MCStreamer &Out,
                                              uint64_t &ErrorInfo,
                                              bool MatchingInlineAsm) {
  MCInst Inst;
  unsigned Result =
      MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm);
  switch (Result) {
  case Match_Success:
    Inst.setLoc(IDLoc);
    Out.emitInstruction(Inst, getSTI());
    return false;
  case Match_MissingFeature:
    return Error(IDLoc, "instruction not supported by this CPU");
  case Match_MnemonicFail:
    return Error(IDLoc, "unrecognized instruction mnemonic");
  case Match_InvalidOperand: {
    SMLoc ErrorLoc = IDLoc;
    if (ErrorInfo != ~0ULL && ErrorInfo < Operands.size())
      ErrorLoc = Operands[ErrorInfo]->getStartLoc();
    return Error(ErrorLoc, "invalid operand for instruction");
  }
  default:
    return Error(IDLoc, "invalid operands for instruction");
  }
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeSuperHAsmParser() {
  RegisterMCAsmParser<SuperHAsmParser> X(getTheSuperHTarget());
  RegisterMCAsmParser<SuperHAsmParser> Y(getTheSuperHLETarget());
}
