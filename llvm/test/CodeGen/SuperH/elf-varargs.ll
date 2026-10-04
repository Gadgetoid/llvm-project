; RUN: llc -mtriple=sh3el-unknown-none-elf -verify-machineinstrs < %s | FileCheck %s

; GCC's convention: a variadic function stores the argument registers that
; hold no named argument just below the incoming stack arguments, above the
; pushed registers, and va_list is a pointer that walks them.

; CHECK-LABEL: one_named:
; CHECK: add #-12, r15
; CHECK-NEXT: sts.l pr, @-r15
; CHECK-DAG: mov.l r7, @(16, r15)
; CHECK-DAG: mov.l r6, @(12, r15)
; CHECK-DAG: mov.l r5, @(8, r15)
; CHECK: add #8, r[[AP:[0-9]+]]
; CHECK: mov.l r[[AP]], @(0, r15)
; CHECK: lds.l @r15+, pr
; CHECK: add #12, r15
define i32 @one_named(i32 %n, ...) {
  %ap = alloca ptr
  call void @llvm.va_start(ptr %ap)
  call void @use(ptr %ap)
  %p = load ptr, ptr %ap
  %v = load i32, ptr %p
  call void @llvm.va_end(ptr %ap)
  ret i32 %v
}

; With all four registers named, the unnamed arguments start at the incoming
; stack pointer and nothing is stored.

; CHECK-LABEL: four_named:
; CHECK-NOT: mov.l r7
; CHECK: mov r15, r[[AP:[0-9]+]]
; CHECK-NEXT: add #4, r[[AP]]
; CHECK-NEXT: mov.l r[[AP]], @(0, r15)
define i32 @four_named(i32 %a, i32 %b, i32 %c, i32 %d, ...) {
  %ap = alloca ptr
  call void @llvm.va_start(ptr %ap)
  %p = load volatile ptr, ptr %ap
  %v = load i32, ptr %p
  call void @llvm.va_end(ptr %ap)
  ret i32 %v
}

declare void @use(ptr)
declare void @llvm.va_start(ptr)
declare void @llvm.va_end(ptr)
