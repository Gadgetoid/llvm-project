#include <windows.h>

static WCHAR line[256];
static int wndproc_calls;
static UINT seen_msg;
static WPARAM seen_wparam;
static LPARAM seen_lparam;

static void say(const WCHAR *text) {
  OutputDebugStringW(text);
  OutputDebugStringW(L"\r\n");
}

static LRESULT CALLBACK TestProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (msg == WM_USER + 7) {
    wndproc_calls++;
    seen_msg = msg;
    seen_wparam = wparam;
    seen_lparam = lparam;
    return 0x12345678;
  }
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

struct pair { int a, b; };

static __attribute__((noinline)) struct pair swap_pair(struct pair p) {
  struct pair r = {p.b, p.a};
  return r;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous, LPWSTR command, int show) {
  wsprintfW(line, L"abi wsprintf %d %s %c %u %x %d %d %d", 1, L"two", L'3', 4u, 0x55, -6, 7, 8);
  say(line);

  __int64 big = _wtoll(L"-12345678901");
  wsprintfW(line, L"abi wtoll %d %d", (int)(big / 1000000), (int)(big % 1000000));
  say(line);

  RECT rect;
  SetRect(&rect, 10, 20, 110, 220);
  POINT inside = {50, 60}, outside = {5, 60};
  wsprintfW(line, L"abi ptinrect %d %d", PtInRect(&rect, inside), PtInRect(&rect, outside));
  say(line);

  struct pair p = swap_pair((struct pair){3, 4});
  double d = (double)big / 3.0;
  wsprintfW(line, L"abi local %d %d %d", p.a, p.b, (int)(d / 1000.0));
  say(line);

  WNDCLASSW wc = {0};
  wc.lpfnWndProc = TestProc;
  wc.hInstance = instance;
  wc.lpszClassName = L"AbiTest";
  RegisterClassW(&wc);
  HWND hwnd = CreateWindowExW(0, L"AbiTest", L"abi", 0, 1, 2, 30, 40, NULL, NULL, instance, NULL);
  LRESULT r = SendMessageW(hwnd, WM_USER + 7, 0xABCD, -42);
  wsprintfW(line, L"abi wndproc %d %x %x %d %x", wndproc_calls, seen_msg, seen_wparam, (int)seen_lparam, (unsigned)r);
  say(line);
  DestroyWindow(hwnd);
  say(L"abi done");
  return 0;
}
