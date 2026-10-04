//===-- SuperHSubtarget.h - Define Subtarget for SuperH ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the SuperH specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "SuperHSubtarget.h"
#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

#define DEBUG_TYPE "sh-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SuperHGenSubtargetInfo.inc"

SuperHSubtarget::SuperHSubtarget(const StringRef &CPU, const StringRef &FS,
                                 const TargetMachine &TM)
    : SuperHGenSubtargetInfo(TM.getTargetTriple(), CPU, CPU, FS), TM(TM),
      InstrInfo(*this), TLInfo(TM, initializeSubtargetDependencies(CPU, FS)),
      TSInfo(), FrameLowering(*this) {

  // Parse features string.
  ParseSubtargetFeatures(CPU, CPU, FS);

#define GET_SUBTARGETINFO_MACRO(ATTRIBUTE, DEFAULT, GETTER)                    \
  LLVM_DEBUG(dbgs() << #ATTRIBUTE "=" << GETTER() << "\n");
#include "SuperHGenSubtargetInfo.inc"
  LLVM_DEBUG(dbgs() << "\n");
}

SuperHSubtarget::~SuperHSubtarget() = default;

const SuperHSubtarget &
SuperHSubtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS) {
  ParseSubtargetFeatures(CPU, CPU, FS);
  return *this;
}
