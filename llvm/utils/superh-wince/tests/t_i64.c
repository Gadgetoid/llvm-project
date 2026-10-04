#include "rt.h"

volatile i64 vl[6] = {1, -1, 0x123456789abcdefll, -0x7edcba9876543210ll,
                      0x100000000ll, 1000000007ll};

int main(void) {
  for (int i = 0; i < 6; i++)
    for (int j = 0; j < 6; j++) {
      i64 a = vl[i], b = vl[j];
      print_kv("add", a + b);
      print_kv("sub", a - b);
      print_kv("mul", a * b);
      print_kv("div", a / b);
      print_kv("rem", a % b);
      print_kv("udiv", (i64)((u64)a / (u64)b));
      print_kv("urem", (i64)((u64)a % (u64)b));
      print_kv("lt", a < b);
      print_kv("ult", (u64)a < (u64)b);
      print_kv("eq", a == b);
      int s = (int)(b & 63);
      print_kv("shl", (i64)((u64)a << s));
      print_kv("shr", (i64)((u64)a >> s));
      print_kv("sar", a >> s);
    }
  print_kv("c1", vl[2] << 1);
  print_kv("c32", vl[2] << 32);
  print_kv("c33", vl[2] >> 33);
  print_kv("mulw", (i64)(i32)vl[3] * (i32)vl[2]);
  print_kv("mulu", (i64)((u64)(u32)vl[3] * (u32)vl[2]));
  return 0;
}
