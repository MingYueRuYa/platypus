#include "stdafx.h"
#include "WinAssistant.h"

#include <cmath>
#include <string>

#include "../include/const.h"
#include "../include/head.h"
#include "Client.h"

using std::string;
using std::wstring;

HHOOK g_hHook = NULL;
extern HINSTANCE g_hInstDll;
wstring title_cache;

void Test(PBYTE pPayload, UINT64 size) {}

void Send(const wchar_t *title) {
  size_t size = wcslen(title) * sizeof(wchar_t);
  wstring temp_str = title;
  if (title_cache != temp_str) {
    title_cache = temp_str;
    Client client;
    client.init(dll_shm_name, MAX_SHM_SIZE, dll_evt_name);
    client.send(function_name, (PVOID)title, size, Test);
  }
}

void Quit(const wchar_t *process_id) {
  size_t size = wcslen(process_id) * sizeof(wchar_t);
  Client client;
  client.init(dll_shm_name, MAX_SHM_SIZE, dll_evt_name);
  client.send(wnd_exit_name, (PVOID)process_id, size, Test);
}

LRESULT WINAPI CallWndProc(int nCode, WPARAM wParam, LPARAM lParam) {
  PCWPSTRUCT msg = (PCWPSTRUCT)lParam;

  static long bFirst = 1;
  if (bFirst) {
    InterlockedExchange(&bFirst, 0);
    wchar_t title[255] = {0};
    GetWindowTextW(msg->hwnd, title, 255);
    Send(title);
  }

  if (WM_SETTEXT == msg->message) {
    const wchar_t *title = (const wchar_t *)msg->lParam;
    if (nullptr != title) {
      if (0 == wcslen(title)) {
        OutputDebugStringA("empty title");
        Quit(std::to_wstring(GetCurrentProcessId()).c_str());
      } else {
        Send(title);
      }
    }
  } else if (WM_CLOSE == msg->message || WM_QUIT == msg->message) {
    OutputDebugStringA("quit wnd");
    Quit(std::to_wstring(GetCurrentProcessId()).c_str());
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
