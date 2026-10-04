//===- SuperH.cpp ---------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ABIInfoImpl.h"
#include "TargetInfo.h"
#include "clang/CodeGen/CGFunctionInfo.h"
#include "llvm/Support/MathExtras.h"

using namespace clang;
using namespace clang::CodeGen;

//===----------------------------------------------------------------------===//
// SuperH GNU ABI Implementation, as GCC implements it without -mrenesas.
//
// Arguments are a sequence of 4-byte words, the first four in R4-R7 and the
// rest on the stack; an argument may be split between the two. Structures are
// copied into the sequence. A structure is returned in R0 (and R1) when GCC
// gives it an integer mode, and otherwise in memory at an address passed in
// R2.
//===----------------------------------------------------------------------===//

namespace {
class SuperHABIInfo : public DefaultABIInfo {
public:
  SuperHABIInfo(CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}

  // GCC gives a structure an integer mode when its size is 1, 2, 4 or 8 bytes
  // and it is at least as aligned as that mode, whose alignment is at most
  // 4 bytes.
  ABIArgInfo classifyReturnType(QualType Ty) const {
    if (Ty->isVoidType())
      return ABIArgInfo::getIgnore();
    if (!isAggregateTypeForABI(Ty))
      return DefaultABIInfo::classifyReturnType(Ty);
    if (getRecordArgABI(Ty, getCXXABI()) == CGCXXABI::RAA_Default) {
      if (isEmptyRecord(getContext(), Ty, true))
        return ABIArgInfo::getIgnore();
      uint64_t Size = getContext().getTypeSize(Ty);
      uint64_t Align = getContext().getTypeAlign(Ty);
      if ((Size == 8 || Size == 16 || Size == 32 || Size == 64) &&
          Align >= std::min<uint64_t>(Size, 32))
        return ABIArgInfo::getDirect(
            llvm::IntegerType::get(getVMContext(), Size));
    }
    return getNaturalAlignIndirect(Ty, getDataLayout().getAllocaAddrSpace());
  }

  ABIArgInfo classifyArgumentType(QualType Ty) const {
    if (!isAggregateTypeForABI(Ty))
      return DefaultABIInfo::classifyArgumentType(Ty);
    if (CGCXXABI::RecordArgABI RAA = getRecordArgABI(Ty, getCXXABI()))
      return getNaturalAlignIndirect(Ty, getDataLayout().getAllocaAddrSpace(),
                                     RAA == CGCXXABI::RAA_DirectInMemory);
    if (isEmptyRecord(getContext(), Ty, true))
      return ABIArgInfo::getIgnore();
    llvm::Type *Word = llvm::IntegerType::get(getVMContext(), 32);
    uint64_t Words = llvm::alignTo(getContext().getTypeSize(Ty), 32) / 32;
    if (Words == 1)
      return ABIArgInfo::getDirect(Word);
    return ABIArgInfo::getDirect(llvm::ArrayType::get(Word, Words));
  }

  void computeInfo(CGFunctionInfo &FI) const override {
    if (!getCXXABI().classifyReturnType(FI))
      FI.getReturnInfo() = classifyReturnType(FI.getReturnType());
    for (auto &Arg : FI.arguments())
      Arg.info = classifyArgumentType(Arg.type);
  }

  // The callee stores the unnamed argument registers next to the stack
  // arguments, so va_list is a pointer that walks 4-byte words.
  RValue EmitVAArg(CodeGenFunction &CGF, Address VAListAddr, QualType Ty,
                   AggValueSlot Slot) const override {
    return emitVoidPtrVAArg(CGF, VAListAddr, Ty, /*IsIndirect=*/false,
                            getContext().getTypeInfoInChars(Ty),
                            CharUnits::fromQuantity(4),
                            /*AllowHigherAlign=*/false, Slot);
  }
};

//===----------------------------------------------------------------------===//
// Windows CE SH-3 ABI Implementation. Documented in Microsoft's "SH-3 Calling
// Standard" rev 1.4 (Windows CE Toolkit for Visual C++ 5.0).
//
// The argument list is laid out like a structure of 4-byte aligned words whose
// first four words are passed in R4-R7; structures are copied into it. Values
// of up to 4 bytes are returned in R0 and anything larger, including 64-bit
// scalars, through a hidden pointer passed in R4.
//===----------------------------------------------------------------------===//

class SuperHWinCEABIInfo : public DefaultABIInfo {
public:
  SuperHWinCEABIInfo(CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}

  ABIArgInfo classifyReturnType(QualType Ty) const {
    if (Ty->isVoidType())
      return ABIArgInfo::getIgnore();
    uint64_t Size = getContext().getTypeSize(Ty);
    if (Size > 32 || (isAggregateTypeForABI(Ty) &&
                      getRecordArgABI(Ty, getCXXABI()) != CGCXXABI::RAA_Default))
      return getNaturalAlignIndirect(Ty, getDataLayout().getAllocaAddrSpace());
    if (isAggregateTypeForABI(Ty)) {
      if (isEmptyRecord(getContext(), Ty, true))
        return ABIArgInfo::getIgnore();
      return ABIArgInfo::getDirect(llvm::IntegerType::get(getVMContext(), 32));
    }
    return DefaultABIInfo::classifyReturnType(Ty);
  }

  ABIArgInfo classifyArgumentType(QualType Ty) const {
    if (isAggregateTypeForABI(Ty)) {
      if (CGCXXABI::RecordArgABI RAA = getRecordArgABI(Ty, getCXXABI()))
        return getNaturalAlignIndirect(Ty,
                                       getDataLayout().getAllocaAddrSpace(),
                                       RAA == CGCXXABI::RAA_DirectInMemory);
      if (isEmptyRecord(getContext(), Ty, true))
        return ABIArgInfo::getIgnore();
      if (getContext().getTypeSize(Ty) <= 32)
        return ABIArgInfo::getDirect(
            llvm::IntegerType::get(getVMContext(), 32));
      // The copy in the argument list is only 4-byte aligned.
      return ABIArgInfo::getIndirect(
          CharUnits::fromQuantity(4), getDataLayout().getAllocaAddrSpace(),
          /*ByVal=*/true,
          /*Realign=*/getContext().getTypeAlign(Ty) > 32);
    }
    return DefaultABIInfo::classifyArgumentType(Ty);
  }

  void computeInfo(CGFunctionInfo &FI) const override {
    if (!getCXXABI().classifyReturnType(FI))
      FI.getReturnInfo() = classifyReturnType(FI.getReturnType());
    for (auto &Arg : FI.arguments())
      Arg.info = classifyArgumentType(Arg.type);
  }

  RValue EmitVAArg(CodeGenFunction &CGF, Address VAListAddr, QualType Ty,
                   AggValueSlot Slot) const override {
    return emitVoidPtrVAArg(CGF, VAListAddr, Ty, /*IsIndirect=*/false,
                            getContext().getTypeInfoInChars(Ty),
                            CharUnits::fromQuantity(4),
                            /*AllowHigherAlign=*/false, Slot);
  }
};

class SuperHTargetCodeGenInfo : public TargetCodeGenInfo {
public:
  SuperHTargetCodeGenInfo(CodeGenTypes &CGT, bool WinCE)
      : TargetCodeGenInfo(
            WinCE ? std::unique_ptr<ABIInfo>(
                        std::make_unique<SuperHWinCEABIInfo>(CGT))
                  : std::unique_ptr<ABIInfo>(
                        std::make_unique<SuperHABIInfo>(CGT))) {}
};
} // namespace

std::unique_ptr<TargetCodeGenInfo>
CodeGen::createSuperHTargetCodeGenInfo(CodeGenModule &CGM) {
  return std::make_unique<SuperHTargetCodeGenInfo>(
      CGM.getTypes(), CGM.getTriple().isWinCE());
}
