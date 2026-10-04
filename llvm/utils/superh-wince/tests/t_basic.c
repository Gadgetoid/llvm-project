#include "rt.h"

volatile i32 vi[8] = {1, -2, 3, -4, 100000, -100000, 0x7fffffff, (i32)0x80000000};
volatile u32 vu[4] = {1u, 0xffffffffu, 0x80000000u, 12345678u};
volatile unsigned char vb[4] = {0x80, 0x7f, 0xff, 1};
volatile signed char vsb[4] = {-128, 127, -1, 1};
volatile unsigned short vh[4] = {0x8000, 0x7fff, 0xffff, 1};
volatile short vsh[4] = {-32768, 32767, -1, 1};

static i32 fib(i32 n) { return n < 2 ? n : fib(n - 1) + fib(n - 2); }

static u32 collatz(u32 n) {
  u32 steps = 0;
  while (n != 1) {
    n = (n & 1) ? 3 * n + 1 : n / 2;
    steps++;
  }
  return steps;
}

__attribute__((noinline)) static i32 cmp_all(i32 a, i32 b, u32 c, u32 d) {
  i32 r = 0;
  r |= (a < b) << 0;
  r |= (a <= b) << 1;
  r |= (a > b) << 2;
  r |= (a >= b) << 3;
  r |= (a == b) << 4;
  r |= (a != b) << 5;
  r |= (c < d) << 6;
  r |= (c <= d) << 7;
  r |= (c > d) << 8;
  r |= (c >= d) << 9;
  r |= (a < 0) << 10;
  r |= (a > 0) << 11;
  r |= (a >= 0) << 12;
  r |= (a <= 0) << 13;
  r |= (a == 0) << 14;
  r |= ((a & b) == 0) << 15;
  r |= (a > 100) << 16;
  r |= (c > 1000u) << 17;
  return r;
}

int main(void) {
  for (int i = 0; i < 8; i++)
    for (int j = 0; j < 8; j++) {
      i32 a = vi[i], b = vi[j];
      print_kx("cmp", cmp_all(a, b, (u32)a, (u32)b));
      print_kx("add", (u32)(a + b));
      print_kx("sub", (u32)(a - b));
      print_kx("mul", (u32)(a * b));
      if (b != 0 && !(a == (i32)0x80000000 && b == -1)) {
        print_kv("div", a / b);
        print_kv("rem", a % b);
      }
      if ((u32)b != 0) {
        print_kx("udiv", (u32)a / (u32)b);
        print_kx("urem", (u32)a % (u32)b);
      }
      print_kx("and", (u32)(a & b));
      print_kx("or", (u32)(a | b));
      print_kx("xor", (u32)(a ^ b));
      u32 sh = (u32)b & 31;
      print_kx("shl", (u32)a << sh);
      print_kx("shr", (u32)a >> sh);
      print_kx("sar", (u32)(a >> sh));
    }
  for (int s = 0; s < 32; s++) {
    print_kx("cshl", ((u32)vu[3]) << s);
  }
  print_kx("c1", vu[3] << 1);
  print_kx("c2", vu[3] << 2);
  print_kx("c3", vu[3] << 3);
  print_kx("c5", vu[3] << 5);
  print_kx("c7", vu[3] << 7);
  print_kx("c8", vu[3] << 8);
  print_kx("c13", vu[3] << 13);
  print_kx("c16", vu[3] << 16);
  print_kx("c24", vu[3] << 24);
  print_kx("c31", vu[3] << 31);
  print_kx("r1", vu[1] >> 1);
  print_kx("r2", vu[1] >> 2);
  print_kx("r3", vu[1] >> 3);
  print_kx("r9", vu[1] >> 9);
  print_kx("r16", vu[1] >> 16);
  print_kx("r31", vu[1] >> 31);
  print_kx("a1", (u32)(vi[7] >> 1));
  print_kx("a7", (u32)(vi[7] >> 7));
  print_kx("a31", (u32)(vi[7] >> 31));
  for (int i = 0; i < 4; i++) {
    print_kv("b", vb[i]);
    print_kv("sb", vsb[i]);
    print_kv("h", vh[i]);
    print_kv("sh", vsh[i]);
    vb[i] = vb[i] + 1;
    vsh[i] = vsh[i] - 1;
  }
  for (int i = 0; i < 4; i++) {
    print_kv("b2", vb[i]);
    print_kv("sh2", vsh[i]);
  }
  print_kv("fib", fib(20));
  print_kv("collatz", collatz(27));
  print_kx("big", 0x12345678u + vu[0]);
  print_kx("neg", (u32)-vi[4]);
  print_kx("not", ~vu[3]);
  return 0;
}
