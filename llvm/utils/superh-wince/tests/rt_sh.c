#include "rt.h"

__attribute__((aligned(4))) static const unsigned short putc_code[] = {
    0xC321, 0x000B, 0x0009, 0x0009};
__attribute__((aligned(4))) static const unsigned short exit_code[] = {
    0xC320, 0x000B, 0x0009, 0x0009};

void rt_putc(int c) { ((void (*)(int))putc_code)(c); }
void rt_exit(int status) { ((void (*)(int))exit_code)(status); }

void *memcpy(void *dst, const void *src, unsigned long n) {
  unsigned char *d = dst;
  const unsigned char *s = src;
  while (n--)
    *d++ = *s++;
  return dst;
}

void *memmove(void *dst, const void *src, unsigned long n) {
  unsigned char *d = dst;
  const unsigned char *s = src;
  if (d < s) {
    while (n--)
      *d++ = *s++;
  } else {
    d += n;
    s += n;
    while (n--)
      *--d = *--s;
  }
  return dst;
}

void *memset(void *dst, int c, unsigned long n) {
  unsigned char *d = dst;
  while (n--)
    *d++ = (unsigned char)c;
  return dst;
}

int memcmp(const void *a, const void *b, unsigned long n) {
  const unsigned char *x = a, *y = b;
  for (; n; n--, x++, y++)
    if (*x != *y)
      return *x - *y;
  return 0;
}
