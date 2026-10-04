; RUN: llc -mtriple=sh3el-unknown-none-wince -verify-machineinstrs < %s | FileCheck %s

; The Windows CE SH-3 calling standard: the first four words in r4-r7, the
; rest after a 16-byte home area that the caller always reserves.

declare i32 @callee6(i32, i32, i32, i32, i32, i32)

; CHECK-LABEL: stack_args:
; CHECK: mov.l @(16, r15), r1
; CHECK: mov.l @(20, r15), r0
define i32 @stack_args(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f) {
  %1 = add i32 %a, %b
  %2 = add i32 %1, %c
  %3 = add i32 %2, %d
  %4 = add i32 %3, %e
  %5 = add i32 %4, %f
  ret i32 %5
}

; CHECK-LABEL: call6:
; CHECK: sts.l pr, @-r15
; CHECK-NEXT: add #-24, r15
; CHECK-DAG: mov.l r1, @(20, r15)
; CHECK-DAG: mov.l r1, @(16, r15)
; CHECK-DAG: mov #1, r4
; CHECK-DAG: mov #2, r5
; CHECK-DAG: mov #3, r6
; CHECK: jsr @r1
; CHECK-NEXT: mov #4, r7
; CHECK-NEXT: add #24, r15
; CHECK-NEXT: lds.l @r15+, pr
; CHECK: .long 1000
; CHECK-NEXT: .Llit1:
; CHECK-NEXT: .long callee6
define i32 @call6() {
  %r = call i32 @callee6(i32 1, i32 2, i32 3, i32 4, i32 5, i32 1000)
  ret i32 %r
}

declare void @callee0()

; A non-leaf function always reserves the home area.
; CHECK-LABEL: call0:
; CHECK: sts.l pr, @-r15
; CHECK-NEXT: add #-16, r15
define void @call0() {
  call void @callee0()
  ret void
}

; A 64-bit argument is 4-byte aligned and may be split between r7 and the
; stack.
; CHECK-LABEL: split64:
; CHECK: mov.l @(16, r15), r0
; CHECK-NEXT: rts
; CHECK-NEXT: add r7, r0
define i32 @split64(i32 %a, i32 %b, i32 %c, i64 %d) {
  %lo = trunc i64 %d to i32
  %s = lshr i64 %d, 32
  %hi = trunc i64 %s to i32
  %1 = add i32 %lo, %hi
  ret i32 %1
}
