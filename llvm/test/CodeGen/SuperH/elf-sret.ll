; RUN: llc -mtriple=sh3el-unknown-none-elf -verify-machineinstrs < %s | FileCheck %s

; GCC passes the address for a returned structure in r2, so the arguments
; still start in r4.

%struct.big = type { [3 x i32] }

; CHECK-LABEL: fill:
; CHECK: mov r2, r0
; CHECK-DAG: mov.l r5, @(4, r0)
; CHECK-DAG: mov.l r4, @r0
define void @fill(ptr sret(%struct.big) %out, i32 %a, i32 %b) {
  store i32 %a, ptr %out
  %p = getelementptr i32, ptr %out, i32 1
  store i32 %b, ptr %p
  ret void
}

declare void @make(ptr sret(%struct.big), i32)

; CHECK-LABEL: call_make:
; CHECK: mov r15, r2
; CHECK: jsr
; CHECK-NEXT: mov #7, r4
define i32 @call_make() {
  %s = alloca %struct.big
  call void @make(ptr sret(%struct.big) %s, i32 7)
  %v = load i32, ptr %s
  ret i32 %v
}
