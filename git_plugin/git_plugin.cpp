// clang-format off
#include "stdafx.h"
// clang-format on
#include "git_plugin.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "../3rdparty/json/json.hpp"
#include "../include/const.h"
#include "../include/head.h"
#include "Client.h"
#include "hook_keyboard.h"
#include "string_utils.hpp"

using std::string;
using std::wstring;
using json = nlohmann::json;

#define HWND_TO_WSTR(wnd) (std::to_wstring((long long)wnd))
#define HWND_TO_PWCHAR(wnd) (std::to_wstring((long long)wnd).c_str())

#define PROCESS_ID_TO_WSTR(id) (std::to_wstring((unsigned long)id))
#define PROCESS_ID_TO_PWCHAR(id) (std::to_wstring((unsigned long)id).c_str())

HHOOK g_hHook = NULL;
extern HINSTANCE g_hInstDll;
wstring title_cache;

void Test(PBYTE pPayload, UINT64 size) {}

void Send(const wchar_t *title, HWND hwnd) {
  wstring temp_title = wstring(title);
  if (temp_title.empty()) {
    OutputDebugStringA("empty title or Default IME title, not need to update");
    return;
  }

  // TODO:How to solve this situation
  // maybe we should restart process
  std::vector<wstring> vc_filter_title = {L"Default IME", L"MSCTFIME UI"};
  auto find_itr = std::find_if(vc_filter_title.begin(), vc_filter_title.end(),
                               [title](const wstring &temp_title) -> bool {
                                 return wstring::npos != temp_title.find(title);
                               });
  if (find_itr != vc_filter_title.end()) {
    OutputDebugStringA("find title: Default IME, MSCTFIME UI");
    return;
  }

  string str_title = to_utf8_string(temp_title);
  json title_json = {
      {"title", str_title}, {"HWND", (long long)hwnd}, {"action", "update"}};
  wstring wstr_title = to_wide_string(title_json.dump());
  if (title_cache != wstr_title) {
    title_cache = wstr_title;
    Client client;
    client.init(dll_shm_name, MAX_SHM_SIZE, dll_evt_name);
    client.send(function_name, (wchar_t *)wstr_title.c_str(),
                wstr_title.size() * sizeof(wchar_t), Test);
  }
}

void Quit(DWORD process_id, HWND hwnd) {
  json quit_json = {{"process_id", (unsigned long)process_id},
                    {"HWND", (long long)hwnd},
                    {"action", "exit"}};
  wstring buffer = to_wide_string(quit_json.dump());
  OutputDebugStringW(buffer.c_str());
  Client client;
  client.init(dll_shm_name, MAX_SHM_SIZE, dll_evt_name);
  client.send(wnd_exit_name, (PVOID)buffer.c_str(),
              buffer.size() * sizeof(wchar_t), Test);
}

static volatile HWND g_wndHwnd = 0;
void ReceiveShortcut(int vkcode) {
  // ::PostMessageA(g_wndHwnd, WM_KEYDOWN, VK_BACK, 0x000E0001);
  // ::PostMessageA(g_wndHwnd, WM_KEYUP, VK_BACK, 0xC00E0001);
}

LRESULT WINAPI CallWndProc(int nCode, WPARAM wParam, LPARAM lParam) {
  PCWPSTRUCT msg = (PCWPSTRUCT)lParam;

  static long first = 1;
  if (InterlockedExchange(&first, 0)) {
    wchar_t title[MAX_PATH] = {0};
    GetWindowTextW(msg->hwnd, title, MAX_PATH);
    OutputDebugStringW(title);
    g_wndHwnd = msg->hwnd;
    Send(title, (msg->hwnd));
    OutputDebugStringA("first time");
    if (MyHook::Instance().start(false)) {
      MyHook::Instance().insertBlockKey(HookShortCut::Shortcut::ALT_F11);
      MyHook::Instance().setNotifyCallBack(ReceiveShortcut);
      OutputDebugStringA("install keyboard successful.");
    } else {
      OutputDebugStringA("install keyboard error.");
    }
  }

  if (WM_SETTEXT == msg->message) {
    const wchar_t *title = (const wchar_t *)msg->lParam;
    if (nullptr != title) {
      if (0 == wcslen(title)) {
        OutputDebugStringA("empty title");
        if (g_wndHwnd == msg->hwnd) {
          Quit(GetCurrentProcessId(), msg->hwnd);
        } else {
          OutputDebugStringA("not target window hwnd.");
        }
      } else {
        if (g_wndHwnd == msg->hwnd) {
          Send(title, (msg->hwnd));
        }
      }
    }
  } else if (WM_CLOSE == msg->message || WM_QUIT == msg->message) {
    OutputDebugStringA("quit wnd");
    Quit(GetCurrentProcessId(), msg->hwnd);
  }
  return (CallNextHookEx(g_hHook, nCode, wParam, lParam));
}

CGitPlugin::CGitPlugin() {}

CGitPlugin::~CGitPlugin() { Unregister(); }

bool CGitPlugin::Register(HWND targetWnd, DWORD dwThreadId) {
  bool bOk = FALSE;
  if (dwThreadId != 0) {
    gitHwnd_ = targetWnd;
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

bool CGitPlugin::Unregister() {
  if (NULL == g_hHook) return true;
  bool ok = UnhookWindowsHookEx(g_hHook);
  g_hHook = NULL;
  OutputDebugStringA("unregister hook.");
  return ok;
}