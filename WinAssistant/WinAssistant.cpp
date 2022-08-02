#include "stdafx.h"
#include "WinAssistant.h"

#include <cmath>
#include <string>

#include "../include/const.h"
#include "../include/head.h"
#include "Client.h"

using std::string;
using std::wstring;

#define HWND_TO_WSTR(wnd) (std::to_wstring((int)wnd))
#define HWND_TO_PWCHAR(wnd) (std::to_wstring((int)wnd).c_str())

#define PROCESS_ID_TO_WSTR(id) (std::to_wstring((unsigned long)id))
#define PROCESS_ID_TO_PWCHAR(id) (std::to_wstring((unsigned long)id).c_str())

HHOOK g_hHook = NULL;
extern HINSTANCE g_hInstDll;
wstring title_cache;

void Test(PBYTE pPayload, UINT64 size) {}

void Send(const wchar_t *title, const wchar_t *hwnd) {
  wchar_t buff[MAX_PATH] = {0};
  _snwprintf_s(buff, MAX_PATH, L"{\"title\":\"%s\", \"HWND\":\"%s\"}", title, hwnd);
  size_t size = wcslen(buff) * sizeof(wchar_t);
  wstring temp_str = title;
  if (title_cache != temp_str) {
    title_cache = temp_str;
    Client client;
    client.init(dll_shm_name, MAX_SHM_SIZE, dll_evt_name);
    client.send(function_name, buff, size, Test);
  }
}

void Quit(const wchar_t *process_id, const wchar_t *hwnd) {
  wchar_t buff[MAX_PATH] = {0};
  _snwprintf_s(buff, MAX_PATH, L"{\"process_id\":\"%s\", \"HWND\":\"%s\"}", process_id, hwnd);
  size_t size = wcslen(buff) * sizeof(wchar_t);
  Client client;
  client.init(dll_shm_name, MAX_SHM_SIZE, dll_evt_name);
  client.send(wnd_exit_name, (PVOID)buff, size, Test);
}

LRESULT WINAPI CallWndProc(int nCode, WPARAM wParam, LPARAM lParam) {
  PCWPSTRUCT msg = (PCWPSTRUCT)lParam;

  static long bFirst = 1;
  if (bFirst) {
    InterlockedExchange(&bFirst, 0);
    wchar_t title[MAX_PATH] = {0};
    GetWindowTextW(msg->hwnd, title, MAX_PATH);
    Send(title, HWND_TO_PWCHAR(msg->hwnd));
  }

  if (WM_SETTEXT == msg->message) {
    const wchar_t *title = (const wchar_t *)msg->lParam;
    if (nullptr != title) {
      if (0 == wcslen(title)) {
        OutputDebugStringA("empty title");
        Quit(PROCESS_ID_TO_PWCHAR(GetCurrentProcessId()), HWND_TO_PWCHAR(msg->hwnd));
      } else {
        Send(title, HWND_TO_PWCHAR(msg->hwnd));
      }
    }
  } else if (WM_CLOSE == msg->message || WM_QUIT == msg->message) {
    OutputDebugStringA("quit wnd");
    Quit(PROCESS_ID_TO_PWCHAR(GetCurrentProcessId()), HWND_TO_PWCHAR(msg->hwnd));
  }
  return (CallNextHookEx(g_hHook, nCode, wParam, lParam));
}

CWinAssistant::CWinAssistant() {}

CWinAssistant::~CWinAssistant() { Unregister(); }

bool CWinAssistant::Register(DWORD dwThreadId) {
  bool bOk = FALSE;
  if (dwThreadId != 0) {
    g_hHook =
        SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, g_hInstDll, dwThreadId);
    bOk = (g_hHook != NULL);
    string thread_id = std::to_string(dwThreadId);
    string successful = string("register successuful thread id:") + thread_id;
    string error = string("register error thread id:") + thread_id;
    OutputDebugStringA((bOk ? successful : error).c_str());
  } else {
    // Make sure that a hook has been installed.
    return Unregister();
  }

  return (bOk);
}

bool CWinAssistant::Unregister() {
  if (NULL == g_hHook) return true;
  bool ok = UnhookWindowsHookEx(g_hHook);
  g_hHook = NULL;
  OutputDebugStringA("unregister hook.");
  return ok;
}
