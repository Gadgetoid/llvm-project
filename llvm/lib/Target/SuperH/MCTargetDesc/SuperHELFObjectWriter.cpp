//===-- SuperHELFObjectWriter.cpp - SuperH ELF Writer ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/SuperHFixupKinds.h"
#include "MCTargetDesc/SuperHMCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCValue.h"

using namespace llvm;

namespace {

class SuperHELFObjectWriter : public MCELFObjectTargetWriter {
public:
  SuperHELFObjectWriter(uint8_t OSABI)
      : MCELFObjectTargetWriter(/*Is64Bit=*/false, OSABI, ELF::EM_SH,
                                /*HasRelocationAddend=*/true) {}

protected:
  unsigned getRelocType(const MCFixup &Fixup, const MCValue &Target,
                        bool IsPCRel) const override;
  bool needsRelocateWithSymbol(const MCValue &Value,
                               unsigned Type) const override {
    return false;
  }
};

} // end anonymous namespace

unsigned SuperHELFObjectWriter::getRelocType(const MCFixup &Fixup,
                                             const MCValue &Target,
                                             bool IsPCRel) const {
  unsigned Kind = Fixup.getKind();
  if (mc::isRelocation(Kind))
    return Kind - FirstLiteralRelocationKind;

  switch (Kind) {
  case FK_Data_4:
    return IsPCRel ? ELF::R_SH_REL32 : ELF::R_SH_DIR32;
  case SH::fixup_sh_branch8:
    return ELF::R_SH_DIR8WPN;
  case SH::fixup_sh_branch12:
    return ELF::R_SH_IND12W;
  case SH::fixup_sh_pcrel8x2:
    return ELF::R_SH_DIR8WPZ;
  case SH::fixup_sh_pcrel8x4:
    return ELF::R_SH_DIR8WPL;
  default:
    reportError(Fixup.getLoc(), "unsupported relocation type");
    return ELF::R_SH_NONE;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createSuperHELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<SuperHELFObjectWriter>(OSABI);
}
