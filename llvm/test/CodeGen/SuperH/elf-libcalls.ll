; RUN: llc -mtriple=sh3el-unknown-none-elf -verify-machineinstrs < %s | FileCheck %s

; The GNU convention links against compiler-rt's generic builtins too, so
; division uses the standard names.

; CHECK-LABEL: sdiv32:
; CHECK: .long __divsi3
define i32 @sdiv32(i32 %a, i32 %b) {
  %r = sdiv i32 %a, %b
  ret i32 %r
}

; CHECK-LABEL: udiv32:
; CHECK: .long __udivsi3
define i32 @udiv32(i32 %a, i32 %b) {
  %r = udiv i32 %a, %b
  ret i32 %r
}

; CHECK-LABEL: srem32:
; CHECK: .long __modsi3
define i32 @srem32(i32 %a, i32 %b) {
  %r = srem i32 %a, %b
  ret i32 %r
}

; CHECK-LABEL: urem32:
; CHECK: .long __umodsi3
define i32 @urem32(i32 %a, i32 %b) {
  %r = urem i32 %a, %b
  ret i32 %r
}

; CHECK-LABEL: sdiv64:
; CHECK: .long __divdi3
define i64 @sdiv64(i64 %a, i64 %b) {
  %r = sdiv i64 %a, %b
  ret i64 %r
}
