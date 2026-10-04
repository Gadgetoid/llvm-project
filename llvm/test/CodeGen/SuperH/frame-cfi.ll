; RUN: llc -mtriple=sh3el-unknown-none-wince -verify-machineinstrs < %s | FileCheck %s

; Call frame information for debuggers: pushes, the frame allocation and the
; switch to r14 as the frame pointer.

declare void @use(ptr)

; CHECK-LABEL: frame:
; CHECK:      .cfi_startproc
; CHECK:      mov.l r14, @-r15
; CHECK-NEXT: .cfi_def_cfa_offset 4
; CHECK-NEXT: .cfi_offset r14, -4
; CHECK-NEXT: sts.l pr, @-r15
; CHECK-NEXT: .cfi_def_cfa_offset 8
; CHECK-NEXT: .cfi_offset pr, -8
; CHECK-NEXT: add #-56, r15
; CHECK-NEXT: .cfi_def_cfa_offset 64
; CHECK-NEXT: mov r15, r14
; CHECK-NEXT: .cfi_def_cfa_register r14
define void @frame() uwtable "frame-pointer"="all" {
  %buf = alloca [10 x i32]
  call void @use(ptr %buf)
  ret void
}
