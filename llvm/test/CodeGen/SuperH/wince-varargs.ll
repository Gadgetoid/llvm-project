; RUN: llc -mtriple=sh3el-unknown-none-wince -verify-machineinstrs < %s | FileCheck %s

; A variadic function stores r4-r7 to the caller's home area so that the
; whole argument list is contiguous; va_list is a plain pointer.

; CHECK-LABEL: second:
; CHECK: add #-4, r15
; CHECK-DAG: mov.l r7, @(16, r15)
; CHECK-DAG: mov.l r0, @(12, r15)
; CHECK-DAG: mov.l r5, @(8, r15)
; CHECK-DAG: mov.l r4, @(4, r15)
; CHECK: mov r15, r1
; CHECK-NEXT: add #8, r1
; CHECK-NEXT: mov.l r1, @(0, r15)
define i32 @second(i32 %n, ...) {
  %ap = alloca ptr
  call void @llvm.va_start(ptr %ap)
  %p = load ptr, ptr %ap
  %q = getelementptr i8, ptr %p, i32 4
  %v = load i32, ptr %q
  call void @llvm.va_end(ptr %ap)
  ret i32 %v
}

declare void @llvm.va_start(ptr)
declare void @llvm.va_end(ptr)
