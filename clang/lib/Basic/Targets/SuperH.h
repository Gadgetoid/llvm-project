//===--- SuperH.h - Declare SuperH target feature support -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares SuperH TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_SUPERH_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_SUPERH_H

#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "llvm/Support/Compiler.h"
#include "llvm/TargetParser/Triple.h"

namespace clang {
namespace targets {

// SuperH Target
class LLVM_LIBRARY_VISIBILITY SuperHTargetInfo : public TargetInfo {
public:
  SuperHTargetInfo(const llvm::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    bool WinCE = Triple.isWinCE();
    TLSSupported = false;
    PointerWidth = PointerAlign = 32;
    ShortWidth = ShortAlign = 16;
    IntWidth = IntAlign = 32;
    LongWidth = LongAlign = 32;
    LongLongWidth = 64;
    HalfWidth = HalfAlign = 16;
    FloatWidth = FloatAlign = 32;
    DoubleWidth = 64;
    LongDoubleWidth = 64;
    DoubleFormat = &llvm::APFloat::IEEEdouble();
    LongDoubleFormat = &llvm::APFloat::IEEEdouble();
    // Microsoft's SH compilers align 64-bit types naturally, GCC to 4 bytes.
    LongLongAlign = DoubleAlign = LongDoubleAlign = WinCE ? 64 : 32;
    SuitableAlign = WinCE ? 64 : 32;
    DefaultAlignForAttributeAligned = WinCE ? 64 : 32;
    SizeType = UnsignedInt;
    PtrDiffType = SignedInt;
    IntPtrType = SignedInt;
    Char16Type = UnsignedShort;
    Char32Type = UnsignedInt;
    Int16Type = SignedShort;
    SigAtomicType = SignedInt;
    if (WinCE) {
      WCharType = UnsignedShort;
      WIntType = UnsignedShort;
    } else {
      WIntType = SignedInt;
    }
    resetDataLayout();
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  llvm::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

  bool allowsLargerPreferedTypeAlignment() const override { return false; }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  std::string_view getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override {
    static const char *const GCCRegNames[] = {
        "r0", "r1", "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};
    return llvm::ArrayRef(GCCRegNames);
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    static const TargetInfo::GCCRegAlias Aliases[] = {{{"sp"}, "r15"}};
    return llvm::ArrayRef(Aliases);
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    switch (*Name) {
    case 'r':
    case 'z':
      Info.setAllowsRegister();
      return true;
    default:
      return false;
    }
  }

  bool isValidCPUName(StringRef Name) const override;
  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;
  bool setCPU(StringRef Name) override;
  std::optional<std::string> handleAsmEscapedChar(char EscChar) const override;
  StringRef getABI() const override { return ABI; }

  std::pair<unsigned, unsigned> hardwareInterferenceSizes() const override {
    return std::make_pair(32, 32);
  }

protected:
  std::string CPU;
  StringRef ABI;
};

} // namespace targets
} // namespace clang

#endif