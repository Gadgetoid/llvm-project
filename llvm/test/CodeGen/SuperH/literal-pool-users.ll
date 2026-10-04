; RUN: llc -mtriple=sh3el-unknown-none-elf -verify-machineinstrs < %s | FileCheck %s

; Four literal loads in a row, about 1 KB before any barrier. The pool that
; splits the code must start early enough for all four literals: placed for
; the first load alone, the last literal didn't fit, and pools for the first
; and the last load kept displacing each other.

; CHECK: mov.l .Llit0, r1
; CHECK-NEXT: mov.l .Llit1, r2
; CHECK-NEXT: mov.l .Llit2, r3
; CHECK-NEXT: mov.l .Llit3, r6
; CHECK: bra .LBB0_2
; CHECK: .Llit0:
; CHECK-NEXT: .long 1000001
; CHECK-NEXT: .Llit1:
; CHECK-NEXT: .long 1000002
; CHECK-NEXT: .Llit2:
; CHECK-NEXT: .long 1000003
; CHECK-NEXT: .Llit3:
; CHECK-NEXT: .long 1000004
; CHECK-NEXT: .LBB0_2:
define void @users(ptr %p, i32 %x) {
  call void asm sideeffect "", "r,r,r,r"(i32 1000001, i32 1000002, i32 1000003, i32 1000004)
  %i = insertelement <250 x i32> poison, i32 %x, i32 0
  %v = shufflevector <250 x i32> %i, <250 x i32> poison, <250 x i32> zeroinitializer
  store volatile <250 x i32> %v, ptr %p
  ret void
}
