//===-- SuperHInstrFormats.td - SuperH Instruction Formats -*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// All SH-1 to SH-3 instructions are 16 bits wide. The field names below
// follow the Renesas manuals: n is the destination register in bits 11-8,
// m the source register in bits 7-4 (or 11-8 for single-register forms that
// read their operand), d a displacement and i an immediate.
//
//===----------------------------------------------------------------------===//

class SHInst<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction {
  let Namespace = "SH";
  let OutOperandList = outs;
  let InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
  let Size = 2;

  field bits<16> Inst;
  field bits<16> SoftFail = 0;

  // The instruction may not be placed in a delay slot: it is a branch, a
  // PC-relative access or otherwise changes control flow.
  bit isDelayIllegal = 0;
  let TSFlags{0} = isDelayIllegal;
}

// Operand-less: oooo oooo oooo oooo
class F0<bits<16> op, dag outs, dag ins, string asmstr,
         list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  let Inst = op;
}

// One register in bits 11-8: oooo nnnn oooo oooo
class FN<bits<4> op1, bits<8> op2, dag outs, dag ins, string asmstr,
         list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  bits<4> Rn;
  let Inst{15-12} = op1;
  let Inst{11-8} = Rn;
  let Inst{7-0} = op2;
}

// Same layout, the register is a source: oooo mmmm oooo oooo
class FM<bits<4> op1, bits<8> op2, dag outs, dag ins, string asmstr,
         list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  bits<4> Rm;
  let Inst{15-12} = op1;
  let Inst{11-8} = Rm;
  let Inst{7-0} = op2;
}

// Two registers: oooo nnnn mmmm oooo
class FNM<bits<4> op1, bits<4> op2, dag outs, dag ins, string asmstr,
          list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  bits<4> Rn;
  bits<4> Rm;
  let Inst{15-12} = op1;
  let Inst{11-8} = Rn;
  let Inst{7-4} = Rm;
  let Inst{3-0} = op2;
}

// Two registers and a 4-bit displacement: oooo nnnn mmmm dddd
class FNMD<bits<4> op1, dag outs, dag ins, string asmstr,
           list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  bits<4> Rn;
  bits<4> Rm;
  bits<4> disp;
  let Inst{15-12} = op1;
  let Inst{11-8} = Rn;
  let Inst{7-4} = Rm;
  let Inst{3-0} = disp;
}

// Base register (store) in bits 7-4 and a 4-bit displacement:
// oooo oooo nnnn dddd
class FND4<bits<8> op1, dag outs, dag ins, string asmstr,
           list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  bits<4> Rn;
  bits<4> disp;
  let Inst{15-8} = op1;
  let Inst{7-4} = Rn;
  let Inst{3-0} = disp;
}

// Base register (load) in bits 7-4 and a 4-bit displacement:
// oooo oooo mmmm dddd
class FMD4<bits<8> op1, dag outs, dag ins, string asmstr,
           list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  bits<4> Rm;
  bits<4> disp;
  let Inst{15-8} = op1;
  let Inst{7-4} = Rm;
  let Inst{3-0} = disp;
}

// 8-bit displacement: oooo oooo dddd dddd
class FD8<bits<8> op1, dag outs, dag ins, string asmstr,
          list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  bits<8> disp;
  let Inst{15-8} = op1;
  let Inst{7-0} = disp;
}

// 8-bit immediate: oooo oooo iiii iiii
class FI8<bits<8> op1, dag outs, dag ins, string asmstr,
          list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  bits<8> imm;
  let Inst{15-8} = op1;
  let Inst{7-0} = imm;
}

// 12-bit displacement: oooo dddd dddd dddd
class FD12<bits<4> op1, dag outs, dag ins, string asmstr,
           list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  bits<12> disp;
  let Inst{15-12} = op1;
  let Inst{11-0} = disp;
}

// Register and 8-bit displacement: oooo nnnn dddd dddd
class FND8<bits<4> op1, dag outs, dag ins, string asmstr,
           list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  bits<4> Rn;
  bits<8> disp;
  let Inst{15-12} = op1;
  let Inst{11-8} = Rn;
  let Inst{7-0} = disp;
}

// Register and 8-bit immediate: oooo nnnn iiii iiii
class FNI8<bits<4> op1, dag outs, dag ins, string asmstr,
           list<dag> pattern = []>
    : SHInst<outs, ins, asmstr, pattern> {
  bits<4> Rn;
  bits<8> imm;
  let Inst{15-12} = op1;
  let Inst{11-8} = Rn;
  let Inst{7-0} = imm;
}

class SHPseudo<dag outs, dag ins, list<dag> pattern = []>
    : SHInst<outs, ins, "", pattern> {
  let isPseudo = 1;
  let isCodeGenOnly = 1;
  let Size = 0;
}
