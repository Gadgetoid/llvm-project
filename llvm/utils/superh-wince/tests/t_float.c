#include "rt.h"

volatile double vd[5] = {1.5, -2.25, 1e10, 3.14159265358979, 0.001};
volatile float vf[5] = {1.5f, -2.25f, 1e10f, 3.14159f, 0.001f};

__attribute__((noinline)) double poly(double x) {
  return ((x * 0.5 + 1.25) * x - 3.0) * x + 0.125;
}

__attribute__((noinline)) float polyf(float x) {
  return ((x * 0.5f + 1.25f) * x - 3.0f) * x + 0.125f;
}

int main(void) {
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      double a = vd[i], b = vd[j];
      print_double("dadd", a + b);
      print_double("dsub", a - b);
      print_double("dmul", a * b);
      print_double("ddiv", a / b);
      print_kv("dlt", a < b);
      print_kv("deq", a == b);
      float x = vf[i], y = vf[j];
      print_double("fadd", x + y);
      print_double("fmul", x * y);
      print_kv("fge", x >= y);
    }
    print_double("poly", poly(vd[i]));
    print_double("polyf", polyf(vf[i]));
    print_kv("d2i", (i32)vd[i]);
    print_kv("d2l", (i64)vd[i]);
    print_double("i2d", (double)(i32)(i * 12345 - 7));
    print_double("f2d", (double)vf[i]);
  }
  return 0;
}
