; RUN: llc -mtriple=sh3el-unknown-none-wince -verify-machineinstrs < %s | FileCheck %s

; Values wider than 32 bits are returned through a hidden pointer in r4,
; which the callee also returns in r0, as Microsoft's llabs does.

; CHECK-LABEL: ret64:
; CHECK: mov r4, r0
; CHECK-NEXT: mov.l r6, @(4, r0)
; CHECK-NEXT: rts
; CHECK-NEXT: mov.l r5, @r0
define i64 @ret64(i64 %x) {
  ret i64 %x
}

%struct.pair = type { i32, i32 }

; CHECK-LABEL: retsret:
; CHECK: mov r4, r0
; CHECK-NEXT: rts
; CHECK-NEXT: mov.l r5, @r0
define void @retsret(ptr sret(%struct.pair) %p, i32 %a) {
  store i32 %a, ptr %p
  ret void
}

declare i64 @get64()

; CHECK-LABEL: call64:
; CHECK: mov r15, r4
; CHECK: jsr @r1
; CHECK-NEXT: add #16, r4
; CHECK-NEXT: mov.l @(16, r15), r0
define i32 @call64() {
  %v = call i64 @get64()
  %t = trunc i64 %v to i32
  ret i32 %t
}
