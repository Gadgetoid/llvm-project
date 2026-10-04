#include "rt.h"

struct node {
  struct node *next;
  i32 value;
  unsigned flag : 1;
  unsigned kind : 3;
  signed delta : 5;
};

static struct node nodes[10];
i32 counter;
static const char *const names[] = {"zero", "one", "two", "three"};
char message[] = "hello, world";

__attribute__((noinline)) i32 classify(i32 x) {
  switch (x) {
  case 0: return 11;
  case 1: return 23;
  case 2: return 37;
  case 3: return 41;
  case 4: return 59;
  case 5: return 61;
  case 6: return 73;
  case 7: return 89;
  case 9: return 97;
  case 100: return 5;
  default: return -1;
  }
}

__attribute__((noinline)) i32 sparse(i32 x) {
  switch (x) {
  case -1000: return 1;
  case 7: return 2;
  case 5000: return 3;
  case 123456: return 4;
  default: return 0;
  }
}

__attribute__((noinline)) i32 use_alloca(i32 n) {
  i32 *p = __builtin_alloca(n * sizeof(i32));
  for (i32 i = 0; i < n; i++)
    p[i] = i * i;
  i32 s = 0;
  for (i32 i = 0; i < n; i++)
    s += p[i];
  return s;
}

__attribute__((noinline)) i32 big_frame(i32 k) {
  volatile i32 arr[300];
  for (int i = 0; i < 300; i++)
    arr[i] = i ^ k;
  i32 s = 0;
  for (int i = 0; i < 300; i += 7)
    s += arr[i];
  return s;
}

__attribute__((noinline)) i32 pressure(i32 *p) {
  i32 a = p[0], b = p[1], c = p[2], d = p[3], e = p[4], f = p[5], g = p[6],
      h = p[7], i = p[8], j = p[9], k = p[10], l = p[11], m = p[12],
      n = p[13];
  for (int r = 0; r < 3; r++) {
    a += b * c;
    b ^= d + e;
    c -= f * g;
    d += h ^ i;
    e += j - k;
    f ^= l + m;
    g += n * a;
    h -= a + b;
    i ^= c + d;
    j += e * f;
    k -= g ^ h;
    l += i + j;
    m ^= k * l;
    n += m - a;
  }
  return a + b + c + d + e + f + g + h + i + j + k + l + m + n;
}

int main(void) {
  for (int i = 0; i < 10; i++) {
    nodes[i].next = i < 9 ? &nodes[i + 1] : 0;
    nodes[i].value = i * 3;
    nodes[i].flag = i & 1;
    nodes[i].kind = i;
    nodes[i].delta = -i;
  }
  i32 s = 0;
  for (struct node *n = nodes; n; n = n->next)
    s += n->value * 1000 + n->flag * 100 + n->kind * 10 + n->delta;
  print_kv("list", s);
  for (int i = -2; i < 12; i++)
    print_kv("classify", classify(i));
  print_kv("classify100", classify(100));
  print_kv("sparse", sparse(-1000) + sparse(7) * 10 + sparse(5000) * 100 +
                         sparse(123456) * 1000 + sparse(8));
  for (int i = 0; i < 4; i++)
    print_str(names[i]);
  rt_putc('\n');
  print_str(message);
  rt_putc('\n');
  counter += 5;
  counter *= 3;
  print_kv("counter", counter);
  print_kv("alloca", use_alloca(17));
  print_kv("bigframe", big_frame(0x55));
  i32 vals[14];
  for (int i = 0; i < 14; i++)
    vals[i] = i * 7 + 3;
  print_kv("pressure", pressure(vals));
  char buf[32];
  memset(buf, 'a', sizeof buf);
  memcpy(buf + 3, message, 5);
  buf[20] = 0;
  print_str(buf);
  rt_putc('\n');
  return 0;
}
