; RUN: llc -mtriple=sh3el-unknown-none-wince -verify-machineinstrs < %s | FileCheck %s

; Structures are copied into the argument list: the words that fall in the
; first four go in registers, as Microsoft's code passes POINT to PtInRect.

%struct.T = type { i32, i32, i32 }

; CHECK-LABEL: take:
; CHECK-DAG: mov.l r7, @(12, r15)
; CHECK-DAG: mov.l r6, @(8, r15)
; CHECK-DAG: mov.l r5, @(4, r15)
; CHECK: mov.l @(16, r15), r1
define i32 @take(i32 %x, ptr byval(%struct.T) align 4 %t, i32 %z) {
  %q = getelementptr i8, ptr %t, i32 8
  %c = load volatile i32, ptr %q
  %s = add i32 %c, %z
  ret i32 %s
}

; CHECK-LABEL: give:
; CHECK-DAG: mov.l r1, @(16, r15)
; CHECK-DAG: mov.l @r4, r5
; CHECK-DAG: mov.l @(4, r4), r6
; CHECK-DAG: mov.l @(8, r4), r7
; CHECK: jsr @r1
; CHECK-NEXT: mov #1, r4
define i32 @give(ptr %t) {
  %r = call i32 @take(i32 1, ptr byval(%struct.T) align 4 %t, i32 3)
  ret i32 %r
}
