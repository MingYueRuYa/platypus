#include "gitwndhelper.h"

#include <WinUser.h>

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string>

#include "debughelper.h"

using std::find_if;
using std::wstring;

wchar_t buff[1024] = {0};

int GetWindowTextSafe(HWND hWnd, LPTSTR lpString, int nMaxCount) {
  if (NULL == hWnd || FALSE == IsWindow(hWnd) || NULL == lpString ||
      0 == nMaxCount) {
    return -1;
  }
  DWORD dwHwndProcessID = 0;
  DWORD dwHwndThreadID = 0;
  dwHwndThreadID = GetWindowThreadProcessId(
      hWnd, &dwHwndProcessID);  //获取窗口所属的进程和线程ID

  if (dwHwndProcessID !=
      GetCurrentProcessId())  //窗口进程不是当前调用进程时，返回原本调用
  {
    return GetWindowText(hWnd, lpString, nMaxCount);
  }

  //窗口进程是当前进程时：
  if (dwHwndThreadID ==
      GetCurrentThreadId())  //窗口线程就是当前调用线程，返回原本调用
  {
    return GetWindowText(hWnd, lpString, nMaxCount);
  }

#ifndef _UNICODE
  WCHAR *lpStringUnicode = new WCHAR[nMaxCount];
  InternalGetWindowText(hWnd, lpStringUnicode, nMaxCount);
  int size =
      WideCharToMultiByte(CP_ACP, 0, lpStringUnicode, -1, NULL, 0, NULL, NULL);
  if (size <= nMaxCount) {
    size = WideCharToMultiByte(CP_ACP, 0, lpStringUnicode, -1, lpString, size,
                               NULL, NULL);
    if (NULL != lpStringUnicode) {
      delete[] lpStringUnicode;
      lpStringUnicode = NULL;
    }
    return size;
  }
  if (NULL != lpStringUnicode) {
    delete[] lpStringUnicode;
    lpStringUnicode = NULL;
  }
  return 0;

#else
  return InternalGetWindowText(hWnd, lpString, nMaxCount);
#endif
}

BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM lParam) {
  // https://blog.csdn.net/shang_cm/article/details/88819472
  // 这里直接调用GetWindowText会发生卡死线程的问题
  // GetWindowText(hwnd, buff, 1024);
  GetWindowTextSafe(hwnd, buff, 1024);
  wstring window_name = buff;
  if (window_name.empty()) {
    return true;
  }

  if (wstring::npos == window_name.find(L"MSYS") &&
      wstring::npos == window_name.find(L"MINGW"))
    return true;

  GitWndHelperInstance.Put(hwnd, "");

  return true;
}

GitWndHelper::~GitWndHelper() {
  mStopThread = true;

  CloseAllWindows();

  if (mPollingWndThread.joinable()) {
    mPollingWndThread.join();
  }
}

void GitWndHelper::EnumWindows() {
  while (!mStopThread) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ::EnumWindows(EnumWindowsProc, 0);
  }
}

bool GitWndHelper::Put(HWND hwnd, const QString &title) {
  GitWndIte ite = find_if(mGitWindowWrap.begin(), mGitWindowWrap.end(),
                          [hwnd](const GitWndWrap &wndWrap) {
                            return wndWrap.GetGitWnd() == hwnd;
                          });
  // 已经存在同样的窗口
  if (ite != mGitWindowWrap.end()) {
    return false;
  }
  mGitWindowWrap.emplace_back(hwnd, title);
  mNotifyHelper.NotifyNewHandle();
  return true;
}

const GitWndWrap &GitWndHelper::Get(HWND hwnd) const {
  ConstGitWndIte ite = find_if(mGitWindowWrap.begin(), mGitWindowWrap.end(),
                          [hwnd](const GitWndWrap &wndWrap) {
                            return wndWrap.GetGitWnd() == hwnd;
                          });
  const static GitWndWrap wrap(0);
  // 已经存在同样的窗口
  if (ite != mGitWindowWrap.end()) {
    return *ite;
  } else {
    return wrap;
  }
}

void GitWndHelper::Clear() { mGitWindowWrap.clear(); }

void GitWndHelper::CloseAllWindows() {
  std::for_each(mGitWindowWrap.begin(), mGitWindowWrap.end(),
                [this](GitWndWrap &wndWrap) { wndWrap.Close(); });

  Clear();
}

void GitWndHelper::SetFocus(QWidget *widget) {
  GitWndIte ite = find_if(mGitWindowWrap.begin(), mGitWindowWrap.end(),
                          [widget](const GitWndWrap &wndWrap) {
                            return wndWrap.GetSmartWidget() == widget;
                          });

  if (ite == mGitWindowWrap.end()) {
    return;
  }

  ite->SetFocus();
}

void GitWndHelper::Close(QWidget *widget) {
  GitWndIte ite = find_if(mGitWindowWrap.begin(), mGitWindowWrap.end(),
                          [widget](const GitWndWrap &wndWrap) {
                            return wndWrap.GetSmartWidget() == widget;
                          });

  if (ite == mGitWindowWrap.end()) {
    return;
  }
  ite->Close();
  mGitWindowWrap.erase(ite);
}

void GitWndHelper::ShowWindow(QWidget *widget) {
  if (nullptr == widget) {
    return;
  }
  std::for_each(mGitWindowWrap.begin(), mGitWindowWrap.end(),
                [](GitWndWrap &wndWrap) { wndWrap.ShowWindow(false); });

  GitWndIte ite = find_if(mGitWindowWrap.begin(), mGitWindowWrap.end(),
                          [widget](const GitWndWrap &wndWrap) {
                            return wndWrap.GetSmartWidget() == widget;
                          });
  if (ite == mGitWindowWrap.end()) {
    return;
  }
  ite->ShowWindow(true);
}

void GitWndHelper::ConnectNotify(QWidget *widget, const char *funName) {
  mNotifyHelper.Connect(widget, funName);
}

GitWindowsWrap &GitWndHelper::GetWindowsWrap() { return mGitWindowWrap; }

void GitWndHelper::init() {
  mStopThread = false;
  // mPollingWndThread = std::thread(&GitWndHelper::EnumWindows, this);
}
