#ifndef SH_TEST_RT_H
#define SH_TEST_RT_H

typedef unsigned int u32;
typedef int i32;
typedef unsigned long long u64;
typedef long long i64;

void rt_putc(int c);
void rt_exit(int status);
void *memcpy(void *dst, const void *src, unsigned int n);
void *memset(void *dst, int c, unsigned int n);
void *memmove(void *dst, const void *src, unsigned int n);
int memcmp(const void *a, const void *b, unsigned int n);

static void print_str(const char *s) {
  while (*s)
    rt_putc(*s++);
}

static void print_u64(u64 v) {
  char buf[24];
  int i = 0;
  do {
    buf[i++] = '0' + (int)(v % 10);
    v /= 10;
  } while (v);
  while (i)
    rt_putc(buf[--i]);
}

static void print_i64(i64 v) {
  if (v < 0) {
    rt_putc('-');
    print_u64(0 - (u64)v);
  } else {
    print_u64((u64)v);
  }
}

static void print_hex(u32 v) {
  static const char digits[] = "0123456789abcdef";
  for (int shift = 28; shift >= 0; shift -= 4)
    rt_putc(digits[(v >> shift) & 15]);
}

static void print_kv(const char *k, i64 v) {
  print_str(k);
  rt_putc('=');
  print_i64(v);
  rt_putc('\n');
}

static void print_kx(const char *k, u32 v) {
  print_str(k);
  rt_putc('=');
  print_hex(v);
  rt_putc('\n');
}

static void print_double(const char *k, double d) {
  print_str(k);
  rt_putc('=');
  if (d < 0) {
    rt_putc('-');
    d = -d;
  }
  u64 whole = (u64)d;
  print_u64(whole);
  rt_putc('.');
  double frac = d - (double)whole;
  for (int i = 0; i < 6; i++) {
    frac *= 10.0;
    int digit = (int)frac;
    rt_putc('0' + digit);
    frac -= digit;
  }
  rt_putc('\n');
}

#endif
