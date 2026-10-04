//===-- SuperHFixupKinds.h - SuperH specific fixup entries ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SUPERH_MCTARGETDESC_SUPERHFIXUPKINDS_H
#define LLVM_LIB_TARGET_SUPERH_MCTARGETDESC_SUPERHFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace SH {

enum Fixups {
  /// bt, bf, bt/s, bf/s: 8-bit displacement in halfwords from PC + 4.
  fixup_sh_branch8 = FirstTargetFixupKind,
  /// bra, bsr: 12-bit displacement in halfwords from PC + 4.
  fixup_sh_branch12,
  /// mov.w @(disp,PC): 8-bit unsigned displacement in halfwords from PC + 4.
  fixup_sh_pcrel8x2,
  /// mov.l @(disp,PC), mova: 8-bit unsigned displacement in words from
  /// (PC & ~3) + 4.
  fixup_sh_pcrel8x4,

  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};

} // namespace SH
} // namespace llvm

#endif // LLVM_LIB_TARGET_SUPERH_MCTARGETDESC_SUPERHFIXUPKINDS_H
