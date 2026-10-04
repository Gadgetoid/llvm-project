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
// SuperH ABI Implementation. Documented at
// https://llvm-gcc-renesas.com/manuals/SH-ABI-Specification.html
//===----------------------------------------------------------------------===//

namespace {
class SuperHABIInfo : public DefaultABIInfo {
public:
  SuperHABIInfo(CodeGenTypes &CGT) : DefaultABIInfo(CGT) {}

  ABIArgInfo classifyReturnType(QualType Ty, bool &LargeRet) const {

    // We have a total of 64 bits of return space in R0 and R1,
    // if we can fit a struct in there, do.
    if (isAggregateTypeForABI(Ty) && getContext().getTypeSize(Ty) <= 64)
      return ABIArgInfo::getDirect();

    // Otherwise, we store a pointer to the struct in R2 and return that.
    if (getContext().getTypeSize(Ty) > 64) {
      LargeRet = true;
      return getNaturalAlignIndirect(Ty, getDataLayout().getAllocaAddrSpace());
    }

    // Otherwise we follow the default way which is compatible.
    return DefaultABIInfo::classifyReturnType(Ty);
  }

  ABIArgInfo classifyArgumentType(QualType Ty, unsigned &NumRegs) const {
    unsigned TySize = getContext().getTypeSize(Ty);

    // Smaller types are required to be sign extended.
    if (TySize < 32 && NumRegs > 0 && Ty->isIntegralOrEnumerationType()) {
      NumRegs -= 1;
      return ABIArgInfo::getExtend(Ty);
    }

    // If there's enough space, pass in registers.
    if (TySize <= NumRegs * 32) {
      NumRegs -= TySize / 32;
      return ABIArgInfo::getDirect();
    }

    // A pointer will be created and passsed in R4 instead, as the arguments
    // from here on out doesn't fit in the remaining registers.
    NumRegs = 0;
    return ABIArgInfo::getDirect();
  }

  void computeInfo(CGFunctionInfo &FI) const override {

    // Decide the return type.
    bool LargeRet = false;
    if (!getCXXABI().classifyReturnType(FI))
      FI.getReturnInfo() = classifyReturnType(FI.getReturnType(), LargeRet);

    // Decide each argument type.
    unsigned NumRegs = 4;
    for (auto &I : FI.arguments())
      I.info = classifyArgumentType(I.type, NumRegs);
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
