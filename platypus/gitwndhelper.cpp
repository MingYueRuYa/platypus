#include "gitwndhelper.h"

#include <WinUser.h>

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string>

#include "debughelper.h"

using std::find_if;
using std::wstring;

GitWndHelper::~GitWndHelper() { CloseAllWindows(); }

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
  mNotifyHelper.NotifyNewHandle(hwnd);
  return true;
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

void GitWndHelper::ConnectNotify(QObject *obj, const char *funName) {
  mNotifyHelper.Connect(obj, funName);
}

bool GitWndHelper::InitGitWidget(HWND git_wnd, QWidget *parent, QString &title,
                                 QWidget **widget) {
  GitWndWrap *git_wrap = nullptr;
  bool result = find(git_wnd, &git_wrap);
  if (!result) return false;
  git_wrap->InitWidget();
  git_wrap->SetParent(parent);
  title = git_wrap->GetTitle();
  *widget = git_wrap->GetSmartWidget();
  return true;
}

QWidget *GitWndHelper::GetWidget(HWND hwnd) {
  ConstGitWndIte ite = find_if(mGitWindowWrap.begin(), mGitWindowWrap.end(),
                               [hwnd](const GitWndWrap &wndWrap) {
                                 return wndWrap.GetGitWnd() == hwnd;
                               });
  if (ite != mGitWindowWrap.end()) {
    return ite->GetSmartWidget();
  } else {
    return nullptr;
  }
}

void GitWndHelper::init() {}

bool GitWndHelper::find(HWND git_hwnd, GitWndWrap **git_wrap) {
  if (git_hwnd == INVALID_HANDLE_VALUE || 0 == git_hwnd) return false;
  GitWndIte ite = find_if(mGitWindowWrap.begin(), mGitWindowWrap.end(),
                          [git_hwnd](const GitWndWrap &wndWrap) {
                            return wndWrap.GetGitWnd() == git_hwnd;
                          });
  if (ite == mGitWindowWrap.end()) return false;
  *git_wrap = &(*ite);
  return true;
}