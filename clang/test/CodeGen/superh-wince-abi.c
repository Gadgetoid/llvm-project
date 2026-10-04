// RUN: %clang_cc1 -triple sh3el-unknown-none-wince -emit-llvm -o - %s | FileCheck %s

// Windows CE SH-3: values up to 32 bits return in r0; anything larger,
// including 64-bit scalars, through a hidden pointer. Structures are passed
// as copies in the argument list.

struct S1 { char c; };
struct S2 { short a, b; };
struct S3 { int a, b, c; };
struct SD { int a; double d; };

_Static_assert(sizeof(short) == 2 && _Alignof(short) == 2, "");
_Static_assert(sizeof(double) == 8 && _Alignof(double) == 8, "");
_Static_assert(sizeof(long double) == 8, "");
_Static_assert(sizeof(long long) == 8 && _Alignof(long long) == 8, "");
_Static_assert(sizeof(struct SD) == 16, "");
_Static_assert(sizeof(__WCHAR_TYPE__) == 2, "");

// CHECK-LABEL: define{{.*}} void @ret64(ptr dead_on_unwind noalias writable sret(i64) align 8 %agg.result, i64 noundef %x)
long long ret64(long long x) { return x; }

// CHECK-LABEL: define{{.*}} void @retdouble(ptr dead_on_unwind noalias writable sret(double) align 8 %agg.result, double noundef %x)
double retdouble(double x) { return x; }

// CHECK-LABEL: define{{.*}} float @retfloat(float noundef %x)
float retfloat(float x) { return x; }

// CHECK-LABEL: define{{.*}} i32 @rets2(i32 %s.coerce)
struct S2 rets2(struct S2 s) { return s; }

// CHECK-LABEL: define{{.*}} i32 @rets1(i32 %s.coerce)
struct S1 rets1(struct S1 s) { return s; }

// CHECK-LABEL: define{{.*}} void @rets3(ptr dead_on_unwind noalias writable sret(%struct.S3) align 4 %agg.result, ptr noundef byval(%struct.S3) align 4 %s)
struct S3 rets3(struct S3 s) { return s; }

// CHECK-LABEL: define{{.*}} i32 @takesd(ptr noundef byval(%struct.SD) align 4 %0)
int takesd(struct SD s) { return s.a; }

// CHECK-LABEL: define{{.*}} signext i16 @shorts(i16 noundef signext %a, i8 noundef zeroext %b)
short shorts(short a, unsigned char b) { return a + b; }
