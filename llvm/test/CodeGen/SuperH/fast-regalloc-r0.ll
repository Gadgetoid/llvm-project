; RUN: llc -mtriple=sh3el-unknown-none-wince -O0 -verify-machineinstrs < %s | FileCheck %s

; At -O0 the fast register allocator used to give r0 to the stored value,
; which comes back from a call in r0, and then had no r0 left for the index
; of mov.w Rm,@(r0,Rn).

declare i16 @get()

; CHECK-LABEL: store_call_result:
; CHECK: jsr
; CHECK: mov r0, [[VALUE:r[0-9]+]]
; CHECK: mov.w [[VALUE]], @(r0, r{{[0-9]+}})
define void @store_call_result(ptr %p, i32 %i) {
  %v = call i16 @get()
  %a = getelementptr i16, ptr %p, i32 %i
  store i16 %v, ptr %a
  ret void
}
