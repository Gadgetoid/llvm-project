// RUN: %clang_cc1 -triple armv4-unknown-none-wince -emit-llvm -o - %s | FileCheck %s

// Windows CE ARM, as Microsoft's compiler implements it: structures have
// 8-byte aligned 64-bit members and Microsoft bitfields, arguments follow APCS
// with no even register pairs, and every structure returns through a hidden
// pointer.

struct S1 { char c; };
struct S4 { int a; };
struct S8 { int a, b; };
struct Mixed { int x; long long y; };
struct Bits { unsigned a : 3; unsigned b : 30; };
struct HasBits { char c; struct Bits b; };
struct MixedBits { char c; short s : 4; char d; };
enum E { E1 = 1 };

_Static_assert(sizeof(struct Mixed) == 16 && __builtin_offsetof(struct Mixed, y) == 8, "");
_Static_assert(sizeof(struct { int x; double d; }) == 16, "");
_Static_assert(sizeof(struct { char c; struct Mixed m; }) == 24, "");
_Static_assert(sizeof(struct Bits) == 8, "");
_Static_assert(sizeof(struct HasBits) == 12, "");
_Static_assert(sizeof(struct MixedBits) == 6, "");
_Static_assert(sizeof(enum E) == 4, "");
_Static_assert(sizeof(long double) == 8, "");
_Static_assert(sizeof(__WCHAR_TYPE__) == 2 && (__WCHAR_TYPE__)-1 > 0, "");
_Static_assert((char)-1 < 0, "");

// CHECK: target datalayout = "e-m:e-p:32:32-Fi8-i64:64-v64:32:64-v128:32:128-a:0:32-n32-S32"

// CHECK-LABEL: define{{.*}} void @rets1(ptr dead_on_unwind noalias writable sret(%struct.S1) align 1 %agg.result)
struct S1 rets1(void) { struct S1 r = { 1 }; return r; }

// CHECK-LABEL: define{{.*}} void @rets4(ptr dead_on_unwind noalias writable sret(%struct.S4) align 4 %agg.result)
struct S4 rets4(void) { struct S4 r = { 1 }; return r; }

// CHECK-LABEL: define{{.*}} void @rets8(ptr dead_on_unwind noalias writable sret(%struct.S8) align 4 %agg.result)
struct S8 rets8(void) { struct S8 r = { 1, 2 }; return r; }

// CHECK-LABEL: define{{.*}} i64 @ret64(i32 noundef %a, i64 noundef %b)
long long ret64(int a, long long b) { return b; }

// CHECK-LABEL: define{{.*}} double @retdouble(i32 noundef %a, double noundef %b)
double retdouble(int a, double b) { return b; }

// CHECK-LABEL: define{{.*}} i32 @takemixed(i32 noundef %a, [4 x i32] %m.coerce)
int takemixed(int a, struct Mixed m) { return m.x; }

// CHECK-LABEL: define{{.*}} signext i8 @plainchar(ptr noundef %p)
char plainchar(char *p) { return *p; }
