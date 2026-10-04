# REQUIRES: superh
# RUN: llvm-mc -filetype=obj -triple=sh3el-unknown-none-wince %s -o %t.o
# RUN: llvm-mc -filetype=obj -triple=sh3el-unknown-none-wince %S/Inputs/superh-target.s -o %t2.o
# RUN: ld.lld --emit-relocs -Ttext=0x10000 %t.o %t2.o -o %t
# RUN: llvm-objdump -d --no-show-raw-insn %t | FileCheck %s
# RUN: llvm-readobj -r %t | FileCheck --check-prefix=RELOCS %s

# CHECK-LABEL: <_start>:
# CHECK-NEXT: 10000: mov.l @(8,pc) ! 0x1000c, r1
# CHECK-NEXT: 10002: bsr 0x10010
# CHECK-NEXT: 10004: nop
# CHECK-NEXT: 10006: bt 0x10010
# CHECK-NEXT: 10008: mova @(0,pc) ! 0x1000c, r0
# CHECK: <target>:
# CHECK-NEXT: 10010: rts

# RELOCS:      0x10002 R_SH_IND12W target 0x0
# RELOCS-NEXT: 0x10006 R_SH_DIR8WPN target 0x0
# RELOCS-NEXT: 0x1000C R_SH_DIR32 target 0x0

.text
.globl _start
_start:
  mov.l .Lpool, r1
  bsr target
  nop
  bt target
  mova .Lpool, r0
  nop
  .p2align 2
.Lpool:
  .long target
