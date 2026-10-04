//===-- SuperHInstrInfo.td - SuperH Instruction Definitions -*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the SH-1, SH-2 and SH-3 integer instruction sets. The
// instruction names follow the Renesas mnemonics. Register-to-memory moves
// are named LD<size><mode> and ST<size><mode>, where size is B, W or L and
// the mode is one of:
//   (none)  @Rm
//   D       @(disp,Rm)
//   X       @(R0,Rm)
//   P       @Rm+ (loads) or @-Rn (stores)
//   G       @(disp,GBR)
//   PC      @(disp,PC)
//
// The T bit lives in SR. Comparisons define SR and conditional branches,
// MOVT, ADDC and friends read it. As with EFLAGS on X86, the DAG carries SR
// as an i32 value and the patterns name the physical register.
//
//===----------------------------------------------------------------------===//

include "SuperHInstrFormats.td"

// The register class of target-independent pointer operands.
def sh_ptr_rc : RegClassByHwMode<[DefaultMode], [GPR]>;

//===----------------------------------------------------------------------===//
// Type profiles and DAG nodes
//===----------------------------------------------------------------------===//

def SDT_SHCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_SHCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_SHCall         : SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>;
def SDT_SHWrapper      : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>,
                                              SDTCisPtrTy<0>]>;
def SDT_SHCmp          : SDTypeProfile<1, 2, [SDTCisVT<0, i32>,
                                              SDTCisVT<1, i32>,
                                              SDTCisVT<2, i32>]>;
def SDT_SHCmpZ         : SDTypeProfile<1, 1, [SDTCisVT<0, i32>,
                                              SDTCisVT<1, i32>]>;
def SDT_SHFlag         : SDTypeProfile<1, 0, [SDTCisVT<0, i32>]>;
def SDT_SHBranch       : SDTypeProfile<0, 2, [SDTCisVT<0, OtherVT>,
                                              SDTCisVT<1, i32>]>;
def SDT_SHMovT         : SDTypeProfile<1, 1, [SDTCisVT<0, i32>,
                                              SDTCisVT<1, i32>]>;
def SDT_SHSelect       : SDTypeProfile<1, 3, [SDTCisVT<0, i32>,
                                              SDTCisSameAs<0, 1>,
                                              SDTCisSameAs<0, 2>,
                                              SDTCisVT<3, i32>]>;
def SDT_SHCarryOp      : SDTypeProfile<2, 3, [SDTCisVT<0, i32>,
                                              SDTCisVT<1, i32>,
                                              SDTCisSameAs<0, 2>,
                                              SDTCisSameAs<0, 3>,
                                              SDTCisVT<4, i32>]>;
def SDT_SHNegC         : SDTypeProfile<2, 2, [SDTCisVT<0, i32>,
                                              SDTCisVT<1, i32>,
                                              SDTCisSameAs<0, 2>,
                                              SDTCisVT<3, i32>]>;

def SHcallseq_start : SDNode<"ISD::CALLSEQ_START", SDT_SHCallSeqStart,
                             [SDNPHasChain, SDNPOutGlue]>;
def SHcallseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_SHCallSeqEnd,
                             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
def SHcall          : SDNode<"SHISD::CALL", SDT_SHCall,
                             [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                              SDNPVariadic]>;
def SHret           : SDNode<"SHISD::RET", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def SHwrapper       : SDNode<"SHISD::WRAPPER", SDT_SHWrapper>;

def SHcmpeq  : SDNode<"SHISD::CMPEQ", SDT_SHCmp>;
def SHcmphs  : SDNode<"SHISD::CMPHS", SDT_SHCmp>;
def SHcmpge  : SDNode<"SHISD::CMPGE", SDT_SHCmp>;
def SHcmphi  : SDNode<"SHISD::CMPHI", SDT_SHCmp>;
def SHcmpgt  : SDNode<"SHISD::CMPGT", SDT_SHCmp>;
def SHtst    : SDNode<"SHISD::TST", SDT_SHCmp>;
def SHcmppz  : SDNode<"SHISD::CMPPZ", SDT_SHCmpZ>;
def SHcmppl  : SDNode<"SHISD::CMPPL", SDT_SHCmpZ>;
def SHclrt   : SDNode<"SHISD::CLRT", SDT_SHFlag>;
def SHsett   : SDNode<"SHISD::SETT", SDT_SHFlag>;

def SHbt     : SDNode<"SHISD::BT", SDT_SHBranch, [SDNPHasChain]>;
def SHbf     : SDNode<"SHISD::BF", SDT_SHBranch, [SDNPHasChain]>;
def SHmovt   : SDNode<"SHISD::MOVT", SDT_SHMovT>;
def SHselect : SDNode<"SHISD::SELECT", SDT_SHSelect>;

def SHaddc   : SDNode<"SHISD::ADDC", SDT_SHCarryOp>;
def SHsubc   : SDNode<"SHISD::SUBC", SDT_SHCarryOp>;
def SHnegc   : SDNode<"SHISD::NEGC", SDT_SHNegC>;


//===----------------------------------------------------------------------===//
// Operands
//===----------------------------------------------------------------------===//

class SHAsmOperand<string name> : AsmOperandClass {
  let Name = name;
  let RenderMethod = "addImmOperands";
}

def SImm8AsmOperand : SHAsmOperand<"SImm8"> {
  let PredicateMethod = "isSImm<8>";
}
def UImm8AsmOperand : SHAsmOperand<"UImm8"> {
  let PredicateMethod = "isUImm<8>";
}

def simm8 : Operand<i32>, ImmLeaf<i32, [{ return isInt<8>(Imm); }]> {
  let ParserMatchClass = SImm8AsmOperand;
  let DecoderMethod = "decodeSImm8";
  let PrintMethod = "printImm";
  let EncoderMethod = "getImm8OpValue";
  let OperandType = "OPERAND_IMMEDIATE";
}

def uimm8 : Operand<i32>, ImmLeaf<i32, [{ return isUInt<8>(Imm); }]> {
  let ParserMatchClass = UImm8AsmOperand;
  let PrintMethod = "printImm";
  let EncoderMethod = "getImm8OpValue";
  let OperandType = "OPERAND_IMMEDIATE";
}

// Displacements are held as byte offsets and scaled when encoded.
class SHDispAsmOperand<int width, int scale>
    : SHAsmOperand<"Disp" # width # "x" # scale> {
  let PredicateMethod = "isDisp<" # width # ", " # scale # ">";
}

class SHDisp<int width, int scale> : Operand<i32> {
  let ParserMatchClass = SHDispAsmOperand<width, scale>;
  let PrintMethod = "printDisp";
  let EncoderMethod = "getDispOpValue<" # scale # ">";
  let DecoderMethod = "decodeDisp<" # scale # ">";
  let OperandType = "OPERAND_IMMEDIATE";
}

def disp4x1 : SHDisp<4, 1>;
def disp4x2 : SHDisp<4, 2>;
def disp4x4 : SHDisp<4, 4>;
def disp8x1 : SHDisp<8, 1>;
def disp8x2 : SHDisp<8, 2>;
def disp8x4 : SHDisp<8, 4>;

def BrTarget8AsmOperand : SHAsmOperand<"BrTarget8"> {
  let PredicateMethod = "isBranchTarget";
}
def BrTarget12AsmOperand : SHAsmOperand<"BrTarget12"> {
  let PredicateMethod = "isBranchTarget";
}
def PCRelAsmOperand : SHAsmOperand<"PCRel"> {
  let PredicateMethod = "isBranchTarget";
}

def brtarget8 : Operand<OtherVT> {
  let ParserMatchClass = BrTarget8AsmOperand;
  let PrintMethod = "printBranchTarget";
  let DecoderMethod = "decodeBranch<8>";
  let EncoderMethod = "getBranch8OpValue";
  let OperandType = "OPERAND_PCREL";
}

def brtarget12 : Operand<OtherVT> {
  let ParserMatchClass = BrTarget12AsmOperand;
  let PrintMethod = "printBranchTarget";
  let DecoderMethod = "decodeBranch<12>";
  let EncoderMethod = "getBranch12OpValue";
  let OperandType = "OPERAND_PCREL";
}

// The target of a PC-relative literal load or mova.
class SHPCRel<string encoder, int scale> : Operand<i32> {
  let ParserMatchClass = PCRelAsmOperand;
  let PrintMethod = "printPCRelLoad<" # scale # ">";
  let EncoderMethod = encoder;
  let DecoderMethod = "decodeDisp<" # scale # ">";
  let OperandType = "OPERAND_PCREL";
}
def pcrel8x2 : SHPCRel<"getPCRel8x2OpValue", 2>;
def pcrel8x4 : SHPCRel<"getPCRel8x4OpValue", 4>;

// A 32-bit literal: an immediate, symbol or symbol plus offset.
def literal32 : Operand<i32> {
  let PrintMethod = "printOperand";
}

// Memory operand of the frame index pseudos: a frame index and an offset.
def frameaddr : Operand<iPTR> {
  let MIOperandInfo = (ops GPR, i32imm);
  let PrintMethod = "printOperand";
}

//===----------------------------------------------------------------------===//
// Complex patterns
//===----------------------------------------------------------------------===//

// Register plus scaled 4-bit displacement.
def AddrDisp1 : ComplexPattern<iPTR, 2, "selectAddrDisp<1>", [], []>;
def AddrDisp2 : ComplexPattern<iPTR, 2, "selectAddrDisp<2>", [], []>;
def AddrDisp4 : ComplexPattern<iPTR, 2, "selectAddrDisp<4>", [], []>;
// R0 plus register.
def AddrIndex : ComplexPattern<iPTR, 2, "selectAddrIndex", [], []>;
// Frame index plus offset.
def AddrFrame : ComplexPattern<iPTR, 2, "selectAddrFrame", [frameindex], []>;

//===----------------------------------------------------------------------===//
// Immediate leaves
//===----------------------------------------------------------------------===//

def imm_uimm8 : ImmLeaf<i32, [{ return isUInt<8>(Imm); }]>;
def imm_ff    : ImmLeaf<i32, [{ return Imm == 0xff; }]>;
def imm_ffff  : ImmLeaf<i32, [{ return Imm == 0xffff; }]>;
def imm_ext8  : ImmLeaf<i32, [{ return Imm >= 128 && Imm <= 255; }]>;
def imm_ext16 : ImmLeaf<i32, [{
  return Imm >= 0x8000 && Imm <= 0xffff && (Imm & 0xff80) == 0xff80;
}]>;

def NegImm8 : SDNodeXForm<imm, [{
  return CurDAG->getSignedTargetConstant(N->getSExtValue() - 256, SDLoc(N),
                                         MVT::i32);
}]>;
def NegImm16 : SDNodeXForm<imm, [{
  return CurDAG->getSignedTargetConstant(N->getSExtValue() - 65536, SDLoc(N),
                                         MVT::i32);
}]>;

//===----------------------------------------------------------------------===//
// Data transfer
//===----------------------------------------------------------------------===//

let hasSideEffects = 0 in {
let isMoveReg = 1, isAsCheapAsAMove = 1 in
def MOV   : FNM<0b0110, 0b0011, (outs GPR:$Rn), (ins GPR:$Rm),
                "mov\t$Rm, $Rn">;

let isReMaterializable = 1, isAsCheapAsAMove = 1, isMoveImm = 1 in
def MOVI  : FNI8<0b1110, (outs GPR:$Rn), (ins simm8:$imm),
                 "mov\t$imm, $Rn", [(set i32:$Rn, simm8:$imm)]>;

let mayLoad = 1, isDelayIllegal = 1 in {
def LDWPC : FND8<0b1001, (outs GPR:$Rn), (ins pcrel8x2:$disp),
                 "mov.w\t$disp, $Rn">;
def LDLPC : FND8<0b1101, (outs GPR:$Rn), (ins pcrel8x4:$disp),
                 "mov.l\t$disp, $Rn">;
}

let isDelayIllegal = 1, Defs = [R0] in
def MOVA  : FD8<0b11000111, (outs), (ins pcrel8x4:$disp),
                "mova\t$disp, r0">;

let Uses = [SR] in
def MOVT  : FN<0b0000, 0b00101001, (outs GPR:$Rn), (ins),
               "movt\t$Rn", [(set i32:$Rn, (SHmovt SR))]>;

def SWAPB : FNM<0b0110, 0b1000, (outs GPR:$Rn), (ins GPR:$Rm),
                "swap.b\t$Rm, $Rn">;
def SWAPW : FNM<0b0110, 0b1001, (outs GPR:$Rn), (ins GPR:$Rm),
                "swap.w\t$Rm, $Rn", [(set i32:$Rn, (rotl i32:$Rm, (i32 16)))]>;

let Constraints = "$src = $Rn" in
def XTRCT : FNM<0b0010, 0b1101, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                "xtrct\t$Rm, $Rn",
                [(set i32:$Rn, (or (shl i32:$Rm, (i32 16)),
                                   (srl i32:$src, (i32 16))))]>;
}

// Loads. Byte and word loads sign-extend.
let hasSideEffects = 0, mayLoad = 1 in {
def LDB   : FNM<0b0110, 0b0000, (outs GPR:$Rn), (ins GPR:$Rm),
                "mov.b\t@$Rm, $Rn">;
def LDW   : FNM<0b0110, 0b0001, (outs GPR:$Rn), (ins GPR:$Rm),
                "mov.w\t@$Rm, $Rn">;
def LDL   : FNM<0b0110, 0b0010, (outs GPR:$Rn), (ins GPR:$Rm),
                "mov.l\t@$Rm, $Rn">;

let isDelayIllegal = 0, Constraints = "$Rm = $Rm_wb" in {
def LDBP  : FNM<0b0110, 0b0100, (outs GPR:$Rn, GPR:$Rm_wb), (ins GPR:$Rm),
                "mov.b\t@${Rm}+, $Rn">;
def LDWP  : FNM<0b0110, 0b0101, (outs GPR:$Rn, GPR:$Rm_wb), (ins GPR:$Rm),
                "mov.w\t@${Rm}+, $Rn">;
def LDLP  : FNM<0b0110, 0b0110, (outs GPR:$Rn, GPR:$Rm_wb), (ins GPR:$Rm),
                "mov.l\t@${Rm}+, $Rn">;
}

def LDBD  : FMD4<0b10000100, (outs R0C:$dst), (ins GPR:$Rm, disp4x1:$disp),
                 "mov.b\t@($disp, $Rm), $dst"> {
  bits<0> dst;
}
def LDWD  : FMD4<0b10000101, (outs R0C:$dst), (ins GPR:$Rm, disp4x2:$disp),
                 "mov.w\t@($disp, $Rm), $dst"> {
  bits<0> dst;
}
def LDLD  : FNMD<0b0101, (outs GPR:$Rn), (ins GPR:$Rm, disp4x4:$disp),
                 "mov.l\t@($disp, $Rm), $Rn">;

def LDBX  : FNM<0b0000, 0b1100, (outs GPR:$Rn), (ins R0C:$idx, GPR:$Rm),
                "mov.b\t@($idx, $Rm), $Rn"> {
  bits<0> idx;
}
def LDWX  : FNM<0b0000, 0b1101, (outs GPR:$Rn), (ins R0C:$idx, GPR:$Rm),
                "mov.w\t@($idx, $Rm), $Rn"> {
  bits<0> idx;
}
def LDLX  : FNM<0b0000, 0b1110, (outs GPR:$Rn), (ins R0C:$idx, GPR:$Rm),
                "mov.l\t@($idx, $Rm), $Rn"> {
  bits<0> idx;
}

let Uses = [GBR] in {
def LDBG  : FD8<0b11000100, (outs R0C:$dst), (ins disp8x1:$disp),
                "mov.b\t@($disp, gbr), $dst"> {
  bits<0> dst;
}
def LDWG  : FD8<0b11000101, (outs R0C:$dst), (ins disp8x2:$disp),
                "mov.w\t@($disp, gbr), $dst"> {
  bits<0> dst;
}
def LDLG  : FD8<0b11000110, (outs R0C:$dst), (ins disp8x4:$disp),
                "mov.l\t@($disp, gbr), $dst"> {
  bits<0> dst;
}
}
}

// Stores.
let hasSideEffects = 0, mayStore = 1 in {
def STB   : FNM<0b0010, 0b0000, (outs), (ins GPR:$Rm, GPR:$Rn),
                "mov.b\t$Rm, @$Rn">;
def STW   : FNM<0b0010, 0b0001, (outs), (ins GPR:$Rm, GPR:$Rn),
                "mov.w\t$Rm, @$Rn">;
def STL   : FNM<0b0010, 0b0010, (outs), (ins GPR:$Rm, GPR:$Rn),
                "mov.l\t$Rm, @$Rn">;

let Constraints = "$Rn = $Rn_wb" in {
def STBP  : FNM<0b0010, 0b0100, (outs GPR:$Rn_wb), (ins GPR:$Rm, GPR:$Rn),
                "mov.b\t$Rm, @-$Rn">;
def STWP  : FNM<0b0010, 0b0101, (outs GPR:$Rn_wb), (ins GPR:$Rm, GPR:$Rn),
                "mov.w\t$Rm, @-$Rn">;
def STLP  : FNM<0b0010, 0b0110, (outs GPR:$Rn_wb), (ins GPR:$Rm, GPR:$Rn),
                "mov.l\t$Rm, @-$Rn">;
}

def STBD  : FND4<0b10000000, (outs), (ins R0C:$src, GPR:$Rn, disp4x1:$disp),
                 "mov.b\t$src, @($disp, $Rn)"> {
  bits<0> src;
}
def STWD  : FND4<0b10000001, (outs), (ins R0C:$src, GPR:$Rn, disp4x2:$disp),
                 "mov.w\t$src, @($disp, $Rn)"> {
  bits<0> src;
}
def STLD  : FNMD<0b0001, (outs), (ins GPR:$Rm, GPR:$Rn, disp4x4:$disp),
                 "mov.l\t$Rm, @($disp, $Rn)">;

def STBX  : FNM<0b0000, 0b0100, (outs), (ins GPR:$Rm, R0C:$idx, GPR:$Rn),
                "mov.b\t$Rm, @($idx, $Rn)"> {
  bits<0> idx;
}
def STWX  : FNM<0b0000, 0b0101, (outs), (ins GPR:$Rm, R0C:$idx, GPR:$Rn),
                "mov.w\t$Rm, @($idx, $Rn)"> {
  bits<0> idx;
}
def STLX  : FNM<0b0000, 0b0110, (outs), (ins GPR:$Rm, R0C:$idx, GPR:$Rn),
                "mov.l\t$Rm, @($idx, $Rn)"> {
  bits<0> idx;
}

let Uses = [GBR] in {
def STBG  : FD8<0b11000000, (outs), (ins R0C:$src, disp8x1:$disp),
                "mov.b\t$src, @($disp, gbr)"> {
  bits<0> src;
}
def STWG  : FD8<0b11000001, (outs), (ins R0C:$src, disp8x2:$disp),
                "mov.w\t$src, @($disp, gbr)"> {
  bits<0> src;
}
def STLG  : FD8<0b11000010, (outs), (ins R0C:$src, disp8x4:$disp),
                "mov.l\t$src, @($disp, gbr)"> {
  bits<0> src;
}
}
}

//===----------------------------------------------------------------------===//
// Arithmetic
//===----------------------------------------------------------------------===//

let hasSideEffects = 0, Constraints = "$src = $Rn" in {
let isCommutable = 1 in
def ADD   : FNM<0b0011, 0b1100, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                "add\t$Rm, $Rn", [(set i32:$Rn, (add i32:$src, i32:$Rm))]>;
def ADDI  : FNI8<0b0111, (outs GPR:$Rn), (ins GPR:$src, simm8:$imm),
                 "add\t$imm, $Rn",
                 [(set i32:$Rn, (add i32:$src, simm8:$imm))]>;
def SUB   : FNM<0b0011, 0b1000, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                "sub\t$Rm, $Rn", [(set i32:$Rn, (sub i32:$src, i32:$Rm))]>;

let Uses = [SR], Defs = [SR] in {
let isCommutable = 1 in
def ADDC  : FNM<0b0011, 0b1110, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                "addc\t$Rm, $Rn",
                [(set i32:$Rn, SR, (SHaddc i32:$src, i32:$Rm, SR))]>;
def SUBC  : FNM<0b0011, 0b1010, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                "subc\t$Rm, $Rn",
                [(set i32:$Rn, SR, (SHsubc i32:$src, i32:$Rm, SR))]>;
}

let Defs = [SR] in {
let isCommutable = 1 in
def ADDV  : FNM<0b0011, 0b1111, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                "addv\t$Rm, $Rn">;
def SUBV  : FNM<0b0011, 0b1011, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                "subv\t$Rm, $Rn">;
def DT    : FN<0b0100, 0b00010000, (outs GPR:$Rn), (ins GPR:$src),
               "dt\t$Rn">;
}
}

let hasSideEffects = 0 in {
def NEG   : FNM<0b0110, 0b1011, (outs GPR:$Rn), (ins GPR:$Rm),
                "neg\t$Rm, $Rn", [(set i32:$Rn, (ineg i32:$Rm))]>;
let Uses = [SR], Defs = [SR] in
def NEGC  : FNM<0b0110, 0b1010, (outs GPR:$Rn), (ins GPR:$Rm),
                "negc\t$Rm, $Rn", [(set i32:$Rn, SR, (SHnegc i32:$Rm, SR))]>;

def EXTSB : FNM<0b0110, 0b1110, (outs GPR:$Rn), (ins GPR:$Rm),
                "exts.b\t$Rm, $Rn",
                [(set i32:$Rn, (sext_inreg i32:$Rm, i8))]>;
def EXTSW : FNM<0b0110, 0b1111, (outs GPR:$Rn), (ins GPR:$Rm),
                "exts.w\t$Rm, $Rn",
                [(set i32:$Rn, (sext_inreg i32:$Rm, i16))]>;
def EXTUB : FNM<0b0110, 0b1100, (outs GPR:$Rn), (ins GPR:$Rm),
                "extu.b\t$Rm, $Rn", [(set i32:$Rn, (and i32:$Rm, 0xff))]>;
def EXTUW : FNM<0b0110, 0b1101, (outs GPR:$Rn), (ins GPR:$Rm),
                "extu.w\t$Rm, $Rn", [(set i32:$Rn, (and i32:$Rm, 0xffff))]>;
}

// Multiplication. The results go to MACL (and MACH); the selector uses the
// pseudo instructions further down, which pair them with sts.
let hasSideEffects = 0 in {
let Defs = [MACLO] in {
def MULL  : FNM<0b0000, 0b0111, (outs), (ins GPR:$Rn, GPR:$Rm),
                "mul.l\t$Rm, $Rn">;
def MULSW : FNM<0b0010, 0b1111, (outs), (ins GPR:$Rn, GPR:$Rm),
                "muls.w\t$Rm, $Rn">;
def MULUW : FNM<0b0010, 0b1110, (outs), (ins GPR:$Rn, GPR:$Rm),
                "mulu.w\t$Rm, $Rn">;
}
let Defs = [MACHI, MACLO] in {
def DMULSL : FNM<0b0011, 0b1101, (outs), (ins GPR:$Rn, GPR:$Rm),
                 "dmuls.l\t$Rm, $Rn">;
def DMULUL : FNM<0b0011, 0b0101, (outs), (ins GPR:$Rn, GPR:$Rm),
                 "dmulu.l\t$Rm, $Rn">;
}
let Defs = [MACHI, MACLO], Uses = [MACHI, MACLO, SR],
    Constraints = "$Rm = $Rm_wb, $Rn = $Rn_wb" in {
def MACL  : FNM<0b0000, 0b1111, (outs GPR:$Rm_wb, GPR:$Rn_wb),
                (ins GPR:$Rm, GPR:$Rn), "mac.l\t@${Rm}+, @${Rn}+">;
def MACW  : FNM<0b0100, 0b1111, (outs GPR:$Rm_wb, GPR:$Rn_wb),
                (ins GPR:$Rm, GPR:$Rn), "mac.w\t@${Rm}+, @${Rn}+">;
}
let Defs = [MACHI, MACLO] in
def CLRMAC : F0<0b0000000000101000, (outs), (ins), "clrmac">;
}

// Division steps.
let hasSideEffects = 0 in {
let Defs = [SR] in {
def DIV0S : FNM<0b0010, 0b0111, (outs), (ins GPR:$Rn, GPR:$Rm),
                "div0s\t$Rm, $Rn">;
def DIV0U : F0<0b0000000000011001, (outs), (ins), "div0u">;
}
let Defs = [SR], Uses = [SR], Constraints = "$src = $Rn" in
def DIV1  : FNM<0b0011, 0b0100, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                "div1\t$Rm, $Rn">;
}

//===----------------------------------------------------------------------===//
// Comparison
//===----------------------------------------------------------------------===//

// Each comparison sets T. The operand order follows the assembler:
// cmp/gt Rm,Rn sets T when Rn > Rm.
let hasSideEffects = 0, Defs = [SR] in {
let isCompare = 1 in {
let isCommutable = 1 in
def CMPEQ  : FNM<0b0011, 0b0000, (outs), (ins GPR:$Rn, GPR:$Rm),
                 "cmp/eq\t$Rm, $Rn", [(set SR, (SHcmpeq i32:$Rn, i32:$Rm))]>;
def CMPHS  : FNM<0b0011, 0b0010, (outs), (ins GPR:$Rn, GPR:$Rm),
                 "cmp/hs\t$Rm, $Rn", [(set SR, (SHcmphs i32:$Rn, i32:$Rm))]>;
def CMPGE  : FNM<0b0011, 0b0011, (outs), (ins GPR:$Rn, GPR:$Rm),
                 "cmp/ge\t$Rm, $Rn", [(set SR, (SHcmpge i32:$Rn, i32:$Rm))]>;
def CMPHI  : FNM<0b0011, 0b0110, (outs), (ins GPR:$Rn, GPR:$Rm),
                 "cmp/hi\t$Rm, $Rn", [(set SR, (SHcmphi i32:$Rn, i32:$Rm))]>;
def CMPGT  : FNM<0b0011, 0b0111, (outs), (ins GPR:$Rn, GPR:$Rm),
                 "cmp/gt\t$Rm, $Rn", [(set SR, (SHcmpgt i32:$Rn, i32:$Rm))]>;
def CMPPZ  : FN<0b0100, 0b00010001, (outs), (ins GPR:$Rn),
                "cmp/pz\t$Rn", [(set SR, (SHcmppz i32:$Rn))]>;
def CMPPL  : FN<0b0100, 0b00010101, (outs), (ins GPR:$Rn),
                "cmp/pl\t$Rn", [(set SR, (SHcmppl i32:$Rn))]>;
def CMPEQI : FI8<0b10001000, (outs), (ins R0C:$src, simm8:$imm),
                 "cmp/eq\t$imm, $src",
                 [(set SR, (SHcmpeq R0C:$src, simm8:$imm))]> {
  bits<0> src;
}
let isCommutable = 1 in
def CMPSTR : FNM<0b0010, 0b1100, (outs), (ins GPR:$Rn, GPR:$Rm),
                 "cmp/str\t$Rm, $Rn">;
let isCommutable = 1 in
def TST    : FNM<0b0010, 0b1000, (outs), (ins GPR:$Rn, GPR:$Rm),
                 "tst\t$Rm, $Rn", [(set SR, (SHtst i32:$Rn, i32:$Rm))]>;
def TSTI   : FI8<0b11001000, (outs), (ins R0C:$src, uimm8:$imm),
                 "tst\t$imm, $src",
                 [(set SR, (SHtst R0C:$src, imm_uimm8:$imm))]> {
  bits<0> src;
}
}
def CLRT   : F0<0b0000000000001000, (outs), (ins), "clrt",
                [(set SR, (SHclrt))]>;
def SETT   : F0<0b0000000000011000, (outs), (ins), "sett",
                [(set SR, (SHsett))]>;
}

let Defs = [SR], mayLoad = 1, mayStore = 1, hasSideEffects = 1 in
def TAS : FN<0b0100, 0b00011011, (outs), (ins GPR:$Rn), "tas.b\t@$Rn">;

//===----------------------------------------------------------------------===//
// Logic
//===----------------------------------------------------------------------===//

let hasSideEffects = 0 in {
let Constraints = "$src = $Rn", isCommutable = 1 in {
def AND   : FNM<0b0010, 0b1001, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                "and\t$Rm, $Rn", [(set i32:$Rn, (and i32:$src, i32:$Rm))]>;
def OR    : FNM<0b0010, 0b1011, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                "or\t$Rm, $Rn", [(set i32:$Rn, (or i32:$src, i32:$Rm))]>;
def XOR   : FNM<0b0010, 0b1010, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                "xor\t$Rm, $Rn", [(set i32:$Rn, (xor i32:$src, i32:$Rm))]>;
}
let Constraints = "$src = $dst" in {
def ANDI  : FI8<0b11001001, (outs R0C:$dst), (ins R0C:$src, uimm8:$imm),
                "and\t$imm, $dst",
                []> {
  bits<0> dst;
  bits<0> src;
}
def ORI   : FI8<0b11001011, (outs R0C:$dst), (ins R0C:$src, uimm8:$imm),
                "or\t$imm, $dst",
                []> {
  bits<0> dst;
  bits<0> src;
}
def XORI  : FI8<0b11001010, (outs R0C:$dst), (ins R0C:$src, uimm8:$imm),
                "xor\t$imm, $dst",
                []> {
  bits<0> dst;
  bits<0> src;
}
}
def NOT   : FNM<0b0110, 0b0111, (outs GPR:$Rn), (ins GPR:$Rm),
                "not\t$Rm, $Rn", [(set i32:$Rn, (not i32:$Rm))]>;
}

let Uses = [R0, GBR], mayLoad = 1, mayStore = 1, hasSideEffects = 1 in {
def ANDM : FI8<0b11001101, (outs), (ins uimm8:$imm), "and.b\t$imm, @(r0, gbr)">;
def ORM  : FI8<0b11001111, (outs), (ins uimm8:$imm), "or.b\t$imm, @(r0, gbr)">;
def XORM : FI8<0b11001110, (outs), (ins uimm8:$imm), "xor.b\t$imm, @(r0, gbr)">;
let Defs = [SR] in
def TSTM : FI8<0b11001100, (outs), (ins uimm8:$imm), "tst.b\t$imm, @(r0, gbr)">;
}

//===----------------------------------------------------------------------===//
// Shifts
//===----------------------------------------------------------------------===//

let hasSideEffects = 0, Constraints = "$src = $Rn" in {
let Defs = [SR] in {
def ROTL   : FN<0b0100, 0b00000100, (outs GPR:$Rn), (ins GPR:$src), "rotl\t$Rn",
                [(set i32:$Rn, (rotl i32:$src, (i32 1)))]>;
def ROTR   : FN<0b0100, 0b00000101, (outs GPR:$Rn), (ins GPR:$src), "rotr\t$Rn",
                [(set i32:$Rn, (rotr i32:$src, (i32 1)))]>;
def SHAL   : FN<0b0100, 0b00100000, (outs GPR:$Rn), (ins GPR:$src), "shal\t$Rn">;
def SHAR   : FN<0b0100, 0b00100001, (outs GPR:$Rn), (ins GPR:$src), "shar\t$Rn",
                [(set i32:$Rn, (sra i32:$src, (i32 1)))]>;
def SHLL   : FN<0b0100, 0b00000000, (outs GPR:$Rn), (ins GPR:$src), "shll\t$Rn",
                [(set i32:$Rn, (shl i32:$src, (i32 1)))]>;
def SHLR   : FN<0b0100, 0b00000001, (outs GPR:$Rn), (ins GPR:$src), "shlr\t$Rn",
                [(set i32:$Rn, (srl i32:$src, (i32 1)))]>;
}
let Defs = [SR], Uses = [SR] in {
def ROTCL  : FN<0b0100, 0b00100100, (outs GPR:$Rn), (ins GPR:$src), "rotcl\t$Rn">;
def ROTCR  : FN<0b0100, 0b00100101, (outs GPR:$Rn), (ins GPR:$src), "rotcr\t$Rn">;
}
def SHLL2  : FN<0b0100, 0b00001000, (outs GPR:$Rn), (ins GPR:$src), "shll2\t$Rn",
                [(set i32:$Rn, (shl i32:$src, (i32 2)))]>;
def SHLL8  : FN<0b0100, 0b00011000, (outs GPR:$Rn), (ins GPR:$src), "shll8\t$Rn",
                [(set i32:$Rn, (shl i32:$src, (i32 8)))]>;
def SHLL16 : FN<0b0100, 0b00101000, (outs GPR:$Rn), (ins GPR:$src), "shll16\t$Rn",
                [(set i32:$Rn, (shl i32:$src, (i32 16)))]>;
def SHLR2  : FN<0b0100, 0b00001001, (outs GPR:$Rn), (ins GPR:$src), "shlr2\t$Rn",
                [(set i32:$Rn, (srl i32:$src, (i32 2)))]>;
def SHLR8  : FN<0b0100, 0b00011001, (outs GPR:$Rn), (ins GPR:$src), "shlr8\t$Rn",
                [(set i32:$Rn, (srl i32:$src, (i32 8)))]>;
def SHLR16 : FN<0b0100, 0b00101001, (outs GPR:$Rn), (ins GPR:$src), "shlr16\t$Rn",
                [(set i32:$Rn, (srl i32:$src, (i32 16)))]>;

// SH-3 dynamic shifts: a positive count shifts left, a negative one right.
let Predicates = [IsSH3] in {
def SHAD   : FNM<0b0100, 0b1100, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                 "shad\t$Rm, $Rn">;
def SHLD   : FNM<0b0100, 0b1101, (outs GPR:$Rn), (ins GPR:$src, GPR:$Rm),
                 "shld\t$Rm, $Rn">;
}
}

//===----------------------------------------------------------------------===//
// Branches, calls and returns
//===----------------------------------------------------------------------===//

let isBranch = 1, isTerminator = 1, isDelayIllegal = 1, hasSideEffects = 0,
    Uses = [SR] in {
def BT  : FD8<0b10001001, (outs), (ins brtarget8:$disp), "bt\t$disp">;
def BF  : FD8<0b10001011, (outs), (ins brtarget8:$disp), "bf\t$disp">;
let hasDelaySlot = 1 in {
def BTS : FD8<0b10001101, (outs), (ins brtarget8:$disp), "bt/s\t$disp">;
def BFS : FD8<0b10001111, (outs), (ins brtarget8:$disp), "bf/s\t$disp">;
}
}

let isBranch = 1, isTerminator = 1, isBarrier = 1, hasDelaySlot = 1,
    isDelayIllegal = 1, hasSideEffects = 0 in {
def BRA  : FD12<0b1010, (outs), (ins brtarget12:$disp), "bra\t$disp">;
let isIndirectBranch = 1 in {
def BRAF : FM<0b0000, 0b00100011, (outs), (ins GPR:$Rm), "braf\t$Rm">;
def JMP  : FM<0b0100, 0b00101011, (outs), (ins GPR:$Rm), "jmp\t@$Rm",
              [(brind i32:$Rm)]>;
}
}

let isCall = 1, hasDelaySlot = 1, isDelayIllegal = 1, Defs = [PR] in {
def JSR  : FM<0b0100, 0b00001011, (outs), (ins GPR:$Rm), "jsr\t@$Rm",
              [(SHcall i32:$Rm)]>;
def BSR  : FD12<0b1011, (outs), (ins brtarget12:$disp), "bsr\t$disp">;
def BSRF : FM<0b0000, 0b00000011, (outs), (ins GPR:$Rm), "bsrf\t$Rm">;
}

// rts reads PR, but PR is not modelled as live into the function: the
// prologue saves it when calls clobber it and the delay slot filler keeps
// PR writes out of rts's slot.
let isReturn = 1, isTerminator = 1, isBarrier = 1, hasDelaySlot = 1,
    isDelayIllegal = 1 in
def RTS  : F0<0b0000000000001011, (outs), (ins), "rts", [(SHret)]>;

let isReturn = 1, isTerminator = 1, isBarrier = 1, hasDelaySlot = 1,
    isDelayIllegal = 1, hasSideEffects = 1 in
def RTE  : F0<0b0000000000101011, (outs), (ins), "rte">;

//===----------------------------------------------------------------------===//
// System control
//===----------------------------------------------------------------------===//

let hasSideEffects = 0 in
def NOP : F0<0b0000000000001001, (outs), (ins), "nop">;

let hasSideEffects = 1 in {
def CLRS  : F0<0b0000000001001000, (outs), (ins), "clrs">;
def SETS  : F0<0b0000000001011000, (outs), (ins), "sets">;
let isDelayIllegal = 1 in
def SLEEP : F0<0b0000000000011011, (outs), (ins), "sleep">;
def LDTLB : F0<0b0000000000111000, (outs), (ins), "ldtlb">;
def PREF  : FN<0b0000, 0b10000011, (outs), (ins GPR:$Rn), "pref\t@$Rn">;
let isDelayIllegal = 1 in
def TRAPA : FI8<0b11000011, (outs), (ins uimm8:$imm), "trapa\t$imm">;
}

// Moves between general and control or system registers.
multiclass SHCtrlMoves<string name, Register reg, bits<8> ldcop,
                       bits<8> ldcmop, bits<8> stcop, bits<8> stcmop,
                       string ld, string st> {
  let Defs = [reg], hasSideEffects = 1 in {
  def LD # NAME  : FM<0b0100, ldcop, (outs), (ins GPR:$Rm),
                      ld # "\t$Rm, " # name>;
  let mayLoad = 1, Constraints = "$Rm = $Rm_wb" in
  def LD # NAME # P : FM<0b0100, ldcmop, (outs GPR:$Rm_wb), (ins GPR:$Rm),
                         ld # ".l\t@${Rm}+, " # name>;
  }
  let Uses = [reg], hasSideEffects = 1 in {
  def ST # NAME  : FN<0b0000, stcop, (outs GPR:$Rn), (ins),
                      st # "\t" # name # ", $Rn">;
  let mayStore = 1, Constraints = "$Rn = $Rn_wb" in
  def ST # NAME # P : FN<0b0100, stcmop, (outs GPR:$Rn_wb), (ins GPR:$Rn),
                         st # ".l\t" # name # ", @-$Rn">;
  }
}

let isDelayIllegal = 1 in {
defm SR   : SHCtrlMoves<"sr",   SR,    0b00001110, 0b00000111,
                        0b00000010, 0b00000011, "ldc", "stc">;
defm SSR  : SHCtrlMoves<"ssr",  SSR,   0b00111110, 0b00110111,
                        0b00110010, 0b00110011, "ldc", "stc">;
defm SPC  : SHCtrlMoves<"spc",  SPC,   0b01001110, 0b01000111,
                        0b01000010, 0b01000011, "ldc", "stc">;
defm VBR  : SHCtrlMoves<"vbr",  VBR,   0b00101110, 0b00100111,
                        0b00100010, 0b00100011, "ldc", "stc">;
}
defm GBR  : SHCtrlMoves<"gbr",  GBR,   0b00011110, 0b00010111,
                        0b00010010, 0b00010011, "ldc", "stc">;
defm MACH : SHCtrlMoves<"mach", MACHI, 0b00001010, 0b00000110,
                        0b00001010, 0b00000010, "lds", "sts">;
defm MACL : SHCtrlMoves<"macl", MACLO, 0b00011010, 0b00010110,
                        0b00011010, 0b00010010, "lds", "sts">;
defm PR   : SHCtrlMoves<"pr",   PR,    0b00101010, 0b00100110,
                        0b00101010, 0b00100010, "lds", "sts">;

//===----------------------------------------------------------------------===//
// Pseudo instructions
//===----------------------------------------------------------------------===//

let Defs = [R15], Uses = [R15] in {
def ADJCALLSTACKDOWN : SHPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                [(SHcallseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP   : SHPseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                                [(SHcallseq_end timm:$amt1, timm:$amt2)]>;
}

// A 32-bit constant or symbol address. The literal pool pass turns it into
// a PC-relative mov.l from a nearby pool.
let isReMaterializable = 1, hasSideEffects = 0, mayLoad = 0 in
def LOADCONST : SHPseudo<(outs GPR:$Rn), (ins literal32:$value)> {
  let Size = 2;
}

// The literal pool load the LOADCONST pseudo becomes, before the pool label
// is known.
let mayLoad = 0, hasSideEffects = 0, isDelayIllegal = 1 in
def LOADLITERAL : SHPseudo<(outs GPR:$Rn), (ins literal32:$value)> {
  let Size = 2;
}

// A literal pool entry: a label and a 32-bit value.
let isNotDuplicable = 1, hasSideEffects = 0 in
def LITERAL : SHPseudo<(outs), (ins i32imm:$label, literal32:$value)> {
  let Size = 4;
}

// Alignment padding before a literal pool.
let hasSideEffects = 0 in
def ALIGNPOOL : SHPseudo<(outs), (ins)> {
  let Size = 2;
}

// Frame accesses, expanded by eliminateFrameIndex.
let mayLoad = 1, hasSideEffects = 0 in {
def LDB_FI : SHPseudo<(outs GPR:$Rn), (ins frameaddr:$addr)>;
def LDW_FI : SHPseudo<(outs GPR:$Rn), (ins frameaddr:$addr)>;
def LDL_FI : SHPseudo<(outs GPR:$Rn), (ins frameaddr:$addr)>;
}
let mayStore = 1, hasSideEffects = 0 in {
def STB_FI : SHPseudo<(outs), (ins GPR:$Rm, frameaddr:$addr)>;
def STW_FI : SHPseudo<(outs), (ins GPR:$Rm, frameaddr:$addr)>;
def STL_FI : SHPseudo<(outs), (ins GPR:$Rm, frameaddr:$addr)>;
}
let hasSideEffects = 0, isReMaterializable = 1 in
def FRAMEADDR : SHPseudo<(outs GPR:$Rn), (ins frameaddr:$addr)>;

// Multiplications through MACL/MACH.
let hasSideEffects = 0, Defs = [MACLO] in {
let isCommutable = 1 in
def MUL32 : SHPseudo<(outs GPR:$Rd), (ins GPR:$Rn, GPR:$Rm),
                     [(set i32:$Rd, (mul i32:$Rn, i32:$Rm))]>;
}
let hasSideEffects = 0, Defs = [MACHI, MACLO], isCommutable = 1 in {
def MULHS32 : SHPseudo<(outs GPR:$Rd), (ins GPR:$Rn, GPR:$Rm),
                       [(set i32:$Rd, (mulhs i32:$Rn, i32:$Rm))]>;
def MULHU32 : SHPseudo<(outs GPR:$Rd), (ins GPR:$Rn, GPR:$Rm),
                       [(set i32:$Rd, (mulhu i32:$Rn, i32:$Rm))]>;
def SMULLOHI : SHPseudo<(outs GPR:$Lo, GPR:$Hi), (ins GPR:$Rn, GPR:$Rm),
                        [(set i32:$Lo, i32:$Hi,
                              (smullohi i32:$Rn, i32:$Rm))]>;
def UMULLOHI : SHPseudo<(outs GPR:$Lo, GPR:$Hi), (ins GPR:$Rn, GPR:$Rm),
                        [(set i32:$Lo, i32:$Hi,
                              (umullohi i32:$Rn, i32:$Rm))]>;
}

// Select on T. Expanded into a branch diamond by the custom inserter.
let usesCustomInserter = 1, hasSideEffects = 0, Uses = [SR] in
def SELECT : SHPseudo<(outs GPR:$dst), (ins GPR:$tval, GPR:$fval),
                      [(set i32:$dst, (SHselect i32:$tval, i32:$fval, SR))]>;

// Conditional branches with the delay slot filled later. Selected for
// SHISD::BT and SHISD::BF.
let isBranch = 1, isTerminator = 1, hasSideEffects = 0, Uses = [SR] in {
def PseudoBT : SHPseudo<(outs), (ins brtarget8:$dst), [(SHbt bb:$dst, SR)]> {
  let Size = 2;
}
def PseudoBF : SHPseudo<(outs), (ins brtarget8:$dst), [(SHbf bb:$dst, SR)]> {
  let Size = 2;
}
}

let isBranch = 1, isTerminator = 1, isBarrier = 1, hasSideEffects = 0 in
def PseudoBR : SHPseudo<(outs), (ins brtarget12:$dst), [(br bb:$dst)]> {
  let Size = 4;
}

// A branch beyond the reach of bra: saves a register, loads the distance
// and uses braf. Inserted by branch relaxation.
let isBranch = 1, isTerminator = 1, isBarrier = 1, hasSideEffects = 0 in
def PseudoLongBR : SHPseudo<(outs), (ins brtarget12:$dst)> {
  let Size = 12;
}

// Constant shift amounts that need a sequence of shift instructions.
let hasSideEffects = 0, Constraints = "$src = $Rn" in {
let Defs = [SR] in {
def SHLLI : SHPseudo<(outs GPR:$Rn), (ins GPR:$src, i32imm:$amount)>;
def SHLRI : SHPseudo<(outs GPR:$Rn), (ins GPR:$src, i32imm:$amount)>;
def SHARI : SHPseudo<(outs GPR:$Rn), (ins GPR:$src, i32imm:$amount)>;
}
}

//===----------------------------------------------------------------------===//
// Selection patterns
//===----------------------------------------------------------------------===//

// Constants.
def : Pat<(i32 imm_ext8:$v), (EXTUB (MOVI (NegImm8 imm:$v)))>;
def : Pat<(i32 imm_ext16:$v), (EXTUW (MOVI (NegImm16 imm:$v)))>;
def : Pat<(i32 imm:$v), (LOADCONST imm:$v)>;
def : Pat<(SHwrapper tglobaladdr:$a), (LOADCONST tglobaladdr:$a)>;
def : Pat<(SHwrapper texternalsym:$a), (LOADCONST texternalsym:$a)>;
def : Pat<(SHwrapper tblockaddress:$a), (LOADCONST tblockaddress:$a)>;
def : Pat<(SHwrapper tjumptable:$a), (LOADCONST tjumptable:$a)>;
def : Pat<(SHwrapper tconstpool:$a), (LOADCONST tconstpool:$a)>;

// Loads.
// Results that must be in R0 are copied out at once, so that the R0
// constraint doesn't spread to two-address instructions that use them.
multiclass SHLoadPats<PatFrag op, SHInst plain, SHInst disp, SHInst index,
                      ComplexPattern dispaddr, SHPseudo frame> {
  def : Pat<(i32 (op (AddrFrame GPR:$base, i32:$off))),
            (frame GPR:$base, i32:$off)>;
  def : Pat<(i32 (op (dispaddr GPR:$base, i32:$off))),
            (COPY_TO_REGCLASS (disp GPR:$base, i32:$off), GPR)>;
  def : Pat<(i32 (op (AddrIndex R0C:$idx, GPR:$base))),
            (index R0C:$idx, GPR:$base)>;
  def : Pat<(i32 (op GPR:$addr)), (plain GPR:$addr)>;
}

defm : SHLoadPats<load, LDL, LDLD, LDLX, AddrDisp4, LDL_FI>;
defm : SHLoadPats<sextloadi8, LDB, LDBD, LDBX, AddrDisp1, LDB_FI>;
defm : SHLoadPats<extloadi8, LDB, LDBD, LDBX, AddrDisp1, LDB_FI>;
defm : SHLoadPats<sextloadi16, LDW, LDWD, LDWX, AddrDisp2, LDW_FI>;
defm : SHLoadPats<extloadi16, LDW, LDWD, LDWX, AddrDisp2, LDW_FI>;

multiclass SHZeroLoadPats<PatFrag op, SHInst plain, SHInst disp,
                          SHInst index, ComplexPattern dispaddr,
                          SHPseudo frame, SHInst ext> {
  def : Pat<(i32 (op (AddrFrame GPR:$base, i32:$off))),
            (ext (frame GPR:$base, i32:$off))>;
  def : Pat<(i32 (op (dispaddr GPR:$base, i32:$off))),
            (ext (disp GPR:$base, i32:$off))>;
  def : Pat<(i32 (op (AddrIndex R0C:$idx, GPR:$base))),
            (ext (index R0C:$idx, GPR:$base))>;
  def : Pat<(i32 (op GPR:$addr)), (ext (plain GPR:$addr))>;
}

defm : SHZeroLoadPats<zextloadi8, LDB, LDBD, LDBX, AddrDisp1, LDB_FI, EXTUB>;
defm : SHZeroLoadPats<zextloadi16, LDW, LDWD, LDWX, AddrDisp2, LDW_FI, EXTUW>;

// Stores.
multiclass SHStorePats<PatFrag op, SHInst plain, SHInst disp, SHInst index,
                       ComplexPattern dispaddr, SHPseudo frame,
                       RegisterClass disprc> {
  def : Pat<(op i32:$src, (AddrFrame GPR:$base, i32:$off)),
            (frame GPR:$src, GPR:$base, i32:$off)>;
  def : Pat<(op disprc:$src, (dispaddr GPR:$base, i32:$off)),
            (disp disprc:$src, GPR:$base, i32:$off)>;
  def : Pat<(op i32:$src, (AddrIndex R0C:$idx, GPR:$base)),
            (index GPR:$src, R0C:$idx, GPR:$base)>;
  def : Pat<(op i32:$src, GPR:$addr), (plain GPR:$src, GPR:$addr)>;
}

defm : SHStorePats<store, STL, STLD, STLX, AddrDisp4, STL_FI, GPR>;
defm : SHStorePats<truncstorei8, STB, STBD, STBX, AddrDisp1, STB_FI, R0C>;
defm : SHStorePats<truncstorei16, STW, STWD, STWX, AddrDisp2, STW_FI, R0C>;

// Logic with an 8-bit immediate works on R0 only. extu.b is better for 255.
let AddedComplexity = -1 in {
def : Pat<(and i32:$src, imm_uimm8:$imm),
          (COPY_TO_REGCLASS (ANDI (COPY_TO_REGCLASS GPR:$src, R0C), imm:$imm),
                            GPR)>;
def : Pat<(or i32:$src, imm_uimm8:$imm),
          (COPY_TO_REGCLASS (ORI (COPY_TO_REGCLASS GPR:$src, R0C), imm:$imm),
                            GPR)>;
def : Pat<(xor i32:$src, imm_uimm8:$imm),
          (COPY_TO_REGCLASS (XORI (COPY_TO_REGCLASS GPR:$src, R0C), imm:$imm),
                            GPR)>;
}

// Extensions.
def : Pat<(i32 (sext_inreg i32:$src, i1)),
          (NEG (AND (MOVI 1), GPR:$src))>;

// Byte swap.
def : Pat<(i32 (bswap i32:$src)), (SWAPB (SWAPW (SWAPB GPR:$src)))>;
def : Pat<(i32 (rotr i32:$src, (i32 16))), (SWAPW GPR:$src)>;

// Shifts. Constants use the fixed shift instructions where that is short;
// everything else goes through the SH-3 dynamic shifts.
def shift_cheap_left : ImmLeaf<i32, [{
  return SuperHInstrInfo::getShiftCost(Imm, SuperHInstrInfo::ShiftLeft) <= 3;
}]>;
def shift_cheap_right : ImmLeaf<i32, [{
  return SuperHInstrInfo::getShiftCost(Imm, SuperHInstrInfo::ShiftRight) <= 3;
}]>;
def shift_one : ImmLeaf<i32, [{ return Imm == 1; }]>;
def NegShift : SDNodeXForm<imm, [{
  return CurDAG->getSignedTargetConstant(-N->getSExtValue(), SDLoc(N),
                                         MVT::i32);
}]>;

def : Pat<(shl i32:$src, (i32 shift_cheap_left:$amount)),
          (SHLLI GPR:$src, imm:$amount)>;
def : Pat<(srl i32:$src, (i32 shift_cheap_right:$amount)),
          (SHLRI GPR:$src, imm:$amount)>;
def : Pat<(sra i32:$src, (i32 shift_one:$amount)),
          (SHARI GPR:$src, imm:$amount)>;
def : Pat<(sra i32:$src, (i32 31)),
          (SHAD GPR:$src, (MOVI -32))>;
def : Pat<(shl i32:$src, (i32 imm:$amount)),
          (SHLD GPR:$src, (MOVI imm:$amount))>;
def : Pat<(srl i32:$src, (i32 imm:$amount)),
          (SHLD GPR:$src, (MOVI (NegShift imm:$amount)))>;
def : Pat<(sra i32:$src, (i32 imm:$amount)),
          (SHAD GPR:$src, (MOVI (NegShift imm:$amount)))>;
def : Pat<(shl i32:$src, i32:$amount), (SHLD GPR:$src, GPR:$amount)>;
def : Pat<(srl i32:$src, i32:$amount), (SHLD GPR:$src, (NEG GPR:$amount))>;
def : Pat<(sra i32:$src, i32:$amount), (SHAD GPR:$src, (NEG GPR:$amount))>;

// Comparisons with zero.
def : Pat<(SHcmpeq i32:$a, (i32 0)), (TST GPR:$a, GPR:$a)>;

// Microsoft's C runtime traps with trapa #1.
def : Pat<(trap), (TRAPA 1)>;
def : Pat<(debugtrap), (TRAPA 1)>;
