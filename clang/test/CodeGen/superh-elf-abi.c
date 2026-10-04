// RUN: %clang_cc1 -triple sh3el-unknown-none-elf -emit-llvm -o - %s | FileCheck %s

// GCC's SuperH convention: arguments are 4-byte words, so structures are
// passed as arrays of words. A structure with the size and alignment of an
// integer mode returns in registers, others through a hidden pointer. va_list
// is a pointer that walks the words.

#include <stdarg.h>

struct C1 { char a; };
struct C2 { char a, b; };
struct C3 { char a, b, c; };
struct S4 { short a, b; };
struct I2 { int a, b; };
struct I3 { int a, b, c; };
struct D1 { double d; };
struct F2 { float a, b; };
struct SA4 { short a[4]; };

// CHECK-LABEL: define{{.*}} i8 @retc1()
struct C1 retc1(void) { struct C1 r = {0}; return r; }

// CHECK-LABEL: define{{.*}} void @retc2(ptr {{.*}}sret(%struct.C2) align 1 %agg.result)
struct C2 retc2(void) { struct C2 r = {0}; return r; }

// CHECK-LABEL: define{{.*}} void @rets4(ptr {{.*}}sret(%struct.S4) align 2 %agg.result)
struct S4 rets4(void) { struct S4 r = {0}; return r; }

// CHECK-LABEL: define{{.*}} i64 @reti2()
struct I2 reti2(void) { struct I2 r = {0}; return r; }

// CHECK-LABEL: define{{.*}} i64 @retd1()
struct D1 retd1(void) { struct D1 r = {0}; return r; }

// CHECK-LABEL: define{{.*}} i64 @retf2()
struct F2 retf2(void) { struct F2 r = {0}; return r; }

// CHECK-LABEL: define{{.*}} void @retsa4(ptr {{.*}}sret(%struct.SA4) align 2 %agg.result)
struct SA4 retsa4(void) { struct SA4 r = {{0}}; return r; }

// CHECK-LABEL: define{{.*}} void @reti3(ptr {{.*}}sret(%struct.I3) align 4 %agg.result, i32 noundef %x)
struct I3 reti3(int x) { struct I3 r = {x, x, x}; return r; }

// CHECK-LABEL: define{{.*}} i32 @argc3(i32 %a.coerce, [3 x i32] %b.coerce, i8 noundef signext %c)
int argc3(struct C3 a, struct I3 b, char c) { return a.a + b.c + c; }

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
