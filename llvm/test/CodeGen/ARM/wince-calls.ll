; RUN: llc -mtriple=armv4-unknown-none-wince -mcpu=strongarm < %s | FileCheck %s

; Windows CE ARM: 64-bit arguments take the next registers with no even
; register pair, and may be split between r3 and the stack. A function that
; returns a structure in memory returns its address in r0.

%struct.Big = type { i32, i32, i32 }

declare void @take64(i32, i64)
declare void @split64(i32, i32, i32, i64)
declare void @make(ptr sret(%struct.Big), i32)

; CHECK-LABEL: call64:
; CHECK-DAG: mov r0, #1
; CHECK-DAG: mov r1, #3
; CHECK-DAG: mov r2, #2
; CHECK: b take64
define void @call64() {
  tail call void @take64(i32 1, i64 8589934595)
  ret void
}

; CHECK-LABEL: callsplit:
; CHECK-DAG: mov [[HIGH:r[0-9]+]], #5
; CHECK-DAG: str [[HIGH]], [sp]
; CHECK-DAG: mov r3, #4
; CHECK: bl split64
define void @callsplit() {
  call void @split64(i32 1, i32 2, i32 3, i64 21474836484)
  ret void
}

; CHECK-LABEL: wrap:
; CHECK: mov [[SAVED:r[0-9]+]], r0
; CHECK: bl make
; CHECK: mov r0, [[SAVED]]
; CHECK: mov pc, lr
define void @wrap(ptr noalias sret(%struct.Big) %result, i32 %x) {
  call void @make(ptr sret(%struct.Big) %result, i32 %x)
  %field = load i32, ptr %result
  %next = add i32 %field, 1
  store i32 %next, ptr %result
  ret void
}
