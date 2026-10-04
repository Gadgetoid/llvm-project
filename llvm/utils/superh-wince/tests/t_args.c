#include "rt.h"
#include <stdarg.h>

struct small {
  char c;
};
struct pair {
  i32 a, b;
};
struct triple {
  i32 a, b, c;
};
struct mixed {
  char c;
  short s;
  i32 i;
  i64 l;
};
struct big {
  i32 v[9];
};

__attribute__((noinline)) i32 many(i32 a, i32 b, i32 c, i32 d, i32 e, i32 f,
                                   i32 g, i32 h) {
  return a + 2 * b + 3 * c + 4 * d + 5 * e + 6 * f + 7 * g + 8 * h;
}

__attribute__((noinline)) i64 mix64(i32 a, i64 b, i32 c, i64 d, i64 e) {
  return a + b * 3 + c * 5 + d * 7 + e * 11;
}

__attribute__((noinline)) i64 split64(i32 a, i32 b, i32 c, i64 d, i32 e) {
  return a + b + c + d * 2 + e;
}

__attribute__((noinline)) double dargs(i32 a, double b, float c, double d,
                                       float e) {
  return a + b * 2 + c * 3 + d * 4 + e * 5;
}

__attribute__((noinline)) struct pair mkpair(i32 a, i32 b) {
  struct pair p = {a, b};
  return p;
}

__attribute__((noinline)) struct triple mktriple(i32 a) {
  struct triple t = {a, a * 2, a * 3};
  return t;
}

__attribute__((noinline)) struct small mksmall(char c) {
  struct small s = {c};
  return s;
}

__attribute__((noinline)) i32 takepair(struct pair p, i32 x) {
  return p.a * 10 + p.b + x;
}

__attribute__((noinline)) i32 taketriple(i32 x, i32 y, struct triple t,
                                         i32 z) {
  return x + y * 2 + t.a * 3 + t.b * 4 + t.c * 5 + z * 6;
}

__attribute__((noinline)) i64 takemixed(struct mixed m, struct mixed n) {
  return m.c + m.s + m.i + m.l + 2 * (n.c + n.s + n.i + n.l);
}

__attribute__((noinline)) i32 takebig(i32 a, struct big b, i32 c) {
  i32 s = a * 1000 + c;
  for (int i = 0; i < 9; i++)
    s += b.v[i] * (i + 1);
  return s;
}

__attribute__((noinline)) struct big mkbig(i32 seed) {
  struct big b;
  for (int i = 0; i < 9; i++)
    b.v[i] = seed + i;
  return b;
}

__attribute__((noinline)) i64 sum_va(i32 n, ...) {
  va_list ap;
  va_start(ap, n);
  i64 s = 0;
  for (i32 i = 0; i < n; i++) {
    i32 kind = va_arg(ap, i32);
    if (kind == 0)
      s += va_arg(ap, i32);
    else if (kind == 1)
      s += va_arg(ap, i64);
    else if (kind == 2)
      s += (i64)(va_arg(ap, double) * 100);
    else {
      struct triple t = va_arg(ap, struct triple);
      s += t.a + t.b + t.c;
    }
  }
  va_end(ap);
  return s;
}

__attribute__((noinline)) i32 vsum(i32 n, va_list ap) {
  i32 s = 0;
  while (n--)
    s += va_arg(ap, i32);
  return s;
}

__attribute__((noinline)) i32 fwd_va(i32 n, ...) {
  va_list ap;
  va_start(ap, n);
  i32 s = vsum(n, ap);
  va_end(ap);
  return s;
}

typedef i32 (*binop)(i32, i32);
static i32 op_add(i32 a, i32 b) { return a + b; }
static i32 op_sub(i32 a, i32 b) { return a - b; }
volatile binop ops[2] = {op_add, op_sub};

int main(void) {
  print_kv("many", many(1, 2, 3, 4, 5, 6, 7, 8));
  print_kv("mix64", mix64(1, 0x100000001ll, 3, -5, 0x7fffffffffll));
  print_kv("split64", split64(1, 2, 3, 0x123456789abcll, 5));
  print_double("dargs", dargs(1, 2.5, 3.25f, -4.125, 5.5f));
  struct pair p = mkpair(3, 4);
  print_kv("pair", p.a * 100 + p.b);
  struct triple t = mktriple(7);
  print_kv("triple", t.a + t.b * 100 + t.c * 10000);
  print_kv("small", mksmall('x').c);
  print_kv("takepair", takepair(p, 5));
  print_kv("taketriple", taketriple(1, 2, t, 3));
  struct mixed m = {1, 2, 3, 0x100000000ll};
  struct mixed n = {-1, -2, -3, -4};
  print_kv("takemixed", takemixed(m, n));
  struct big b = mkbig(5);
  print_kv("takebig", takebig(2, b, 3));
  print_kv("bigret", b.v[0] + b.v[8] * 100);
  print_kv("sum_va", sum_va(5, 0, 10, 1, 0x200000000ll, 2, 1.5, 3, t, 0, -4));
  print_kv("fwd_va", fwd_va(6, 1, 2, 3, 4, 5, 6));
  print_kv("fnptr", ops[0](9, 4) * 10 + ops[1](9, 4));
  return 0;
}
