//===-- SuperHCallingConv.td - Calling Conventions for SuperH -*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-------------------------------------------------------------------------===//
//
// Argument and return value assignment is done in C++ (SuperHISelLowering.cpp)
// because both conventions lay the argument list out as a memory image. This
// file only lists the callee-saved registers.
//
//===-------------------------------------------------------------------------===//

// R8-R14 are permanent registers in both the GNU/Renesas convention and the
// Windows CE SH-3 calling standard. PR is listed so that a function that
// makes calls saves its return address with the other registers.
def CSR_SH : CalleeSavedRegs<(add PR, R14, R13, R12, R11, R10, R9, R8)>;

def CSR_NoRegs : CalleeSavedRegs<(add)>;
