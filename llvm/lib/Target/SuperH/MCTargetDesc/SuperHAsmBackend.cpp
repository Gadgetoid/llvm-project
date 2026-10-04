//===-- SuperHAsmBackend.cpp - SuperH Assembler Backend -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SuperHAsmBackend.h"
#include "SuperHFixupKinds.h"
#include "SuperHMCTargetDesc.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/EndianStream.h"

using namespace llvm;

SuperHAsmBackend::SuperHAsmBackend(const MCSubtargetInfo &STI, uint8_t OSABI)
    : MCAsmBackend(STI.getTargetTriple().isLittleEndian()
                       ? llvm::endianness::little
                       : llvm::endianness::big),
      OSABI(OSABI) {}

std::optional<MCFixupKind>
SuperHAsmBackend::getFixupKind(StringRef Name) const {
  unsigned Type = StringSwitch<unsigned>(Name)
#define ELF_RELOC(NAME, ID) .Case(#NAME, ID)
#include "llvm/BinaryFormat/ELFRelocs/SuperH.def"
#undef ELF_RELOC
                      .Default(-1u);
  if (Type != -1u)
    return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
  return std::nullopt;
}

MCFixupKindInfo SuperHAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  static const MCFixupKindInfo Infos[SH::NumTargetFixupKinds] = {
      // Name                  Offset  Bits  Flags
      {"fixup_sh_branch8",     0,      8,    0},
      {"fixup_sh_branch12",    0,      12,   0},
      {"fixup_sh_pcrel8x2",    0,      8,    0},
      {"fixup_sh_pcrel8x4",    0,      8,    0},
  };
  if (mc::isRelocation(Kind))
    return {};
  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);
  return Infos[Kind - FirstTargetFixupKind];
}

bool SuperHAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                    const MCSubtargetInfo *STI) const {
  if (Count % 2) {
    OS.write_zeros(1);
    --Count;
  }
  for (; Count; Count -= 2)
    support::endian::write<uint16_t>(OS, 0x0009, Endian);
  return true;
}

void SuperHAsmBackend::applyFixup(const MCFragment &F, const MCFixup &Fixup,
                                  const MCValue &Target, uint8_t *Data,
                                  uint64_t Value, bool IsResolved) {
  unsigned Kind = Fixup.getKind();
  auto WriteData = [&](uint64_t Bits) {
    unsigned Size = MCAsmBackend::getFixupKindInfo(Kind).TargetSize / 8;
    for (unsigned I = 0; I != Size; ++I) {
      unsigned Index = Endian == llvm::endianness::little ? I : Size - 1 - I;
      Data[Index] |= uint8_t(Bits >> (I * 8));
    }
  };

  if (!IsResolved) {
    auto &Writer = static_cast<ELFObjectWriter &>(Asm->getWriter());
    auto &Relocations =
        Writer.Relocations[static_cast<const MCSectionELF *>(F.getParent())];
    size_t Count = Relocations.size();
    maybeAddReloc(F, Fixup, Target, Value, IsResolved);
    // GNU as also leaves the addend of a data relocation in the section
    // contents, and GNU ld takes it from there rather than from r_addend.
    if (Relocations.size() > Count && Kind < FirstTargetFixupKind &&
        !mc::isRelocation(Kind))
      WriteData(Relocations.back().Addend);
    return;
  }

  MCContext &Ctx = getContext();
  if (mc::isRelocation(Kind))
    return;

  if (Kind < FirstTargetFixupKind) {
    WriteData(Value);
    return;
  }

  int64_t Displacement = int64_t(Value);
  uint64_t Address = Asm->getFragmentOffset(F) + Fixup.getOffset();
  uint16_t Field;
  switch (Kind) {
  case SH::fixup_sh_branch8:
    Displacement -= 4;
    if (Displacement % 2 || !isInt<9>(Displacement))
      Ctx.reportError(Fixup.getLoc(), "branch target out of range");
    Field = (Displacement / 2) & 0xFF;
    break;
  case SH::fixup_sh_branch12:
    Displacement -= 4;
    if (Displacement % 2 || !isInt<13>(Displacement))
      Ctx.reportError(Fixup.getLoc(), "branch target out of range");
    Field = (Displacement / 2) & 0xFFF;
    break;
  case SH::fixup_sh_pcrel8x2:
    Displacement -= 4;
    if (Displacement % 2 || !isUInt<9>(Displacement))
      Ctx.reportError(Fixup.getLoc(), "PC-relative load out of range");
    Field = (Displacement / 2) & 0xFF;
    break;
  case SH::fixup_sh_pcrel8x4:
    Displacement += (Address & 3) - 4;
    if (Displacement % 4 || !isUInt<10>(Displacement))
      Ctx.reportError(Fixup.getLoc(),
                      "PC-relative load out of range or misaligned");
    Field = (Displacement / 4) & 0xFF;
    break;
  default:
    llvm_unreachable("unknown SuperH fixup");
  }

  unsigned Low = Endian == llvm::endianness::little ? 0 : 1;
  Data[Low] |= uint8_t(Field);
  Data[1 - Low] |= uint8_t(Field >> 8);
}

std::unique_ptr<MCObjectTargetWriter>
SuperHAsmBackend::createObjectTargetWriter() const {
  return createSuperHELFObjectWriter(OSABI);
}

MCAsmBackend *llvm::createSuperHAsmBackend(const Target &T,
                                           const MCSubtargetInfo &STI,
                                           const MCRegisterInfo &MRI,
                                           const MCTargetOptions &Options) {
  uint8_t OSABI =
      MCELFObjectTargetWriter::getOSABI(STI.getTargetTriple().getOS());
  return new SuperHAsmBackend(STI, OSABI);
}
