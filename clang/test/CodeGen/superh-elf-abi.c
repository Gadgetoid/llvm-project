// RUN: %clang_cc1 -triple sh3el-unknown-none-elf -emit-llvm -o - %s | FileCheck %s

// GCC's SuperH convention: va_list is a pointer that walks 4-byte words.

#include <stdarg.h>

// CHECK-LABEL: define{{.*}} i64 @vaarg64(
// CHECK: %argp.next = getelementptr inbounds i8, ptr %argp.cur, i32 8
// CHECK: load i64, ptr %argp.cur, align 4
long long vaarg64(int n, ...) {
  va_list ap;
  va_start(ap, n);
  long long v = va_arg(ap, long long);
  va_end(ap);
  return v;
}

// CHECK-LABEL: define{{.*}} i32 @vaargchar(
// CHECK: %argp.next = getelementptr inbounds i8, ptr %argp.cur, i32 4
int vaargchar(int n, ...) {
  va_list ap;
  va_start(ap, n);
  int v = va_arg(ap, int);
  va_end(ap);
  return v;
}
