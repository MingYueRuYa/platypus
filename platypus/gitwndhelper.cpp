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

bool GitWndHelper::Add(HWND hwnd, const QString &title) {
  GitWndMapItr itr = git_wnds_maps_.find(hwnd);
  // exist same window
  if (itr != git_wnds_maps_.end()) {
    return false;
  }
  git_wnds_maps_[hwnd] = new GitWndWrap(hwnd, title);
  mNotifyHelper.NotifyNewHandle(hwnd);
  return true;
}

void GitWndHelper::Clear() { git_wnds_maps_.clear(); }

void GitWndHelper::CloseAllWindows() {
  std::for_each(git_wnds_maps_.begin(), git_wnds_maps_.end(),
                [](GitWndPair &wnd_pair) { wnd_pair.second->Close(); });

  Clear();
}

void GitWndHelper::SetFocus(QWidget *widget) {
  GitWndMapItr itr = this->find(widget);
  if (itr == git_wnds_maps_.end()) {
    return;
  }

  itr->second->SetFocus();
}

void GitWndHelper::Delete(QWidget *widget) {
  GitWndMapItr itr = this->find(widget);
  if (itr == git_wnds_maps_.end()) {
    return;
  }
  itr->second->Close();
  delete itr->second;
  git_wnds_maps_.erase(itr);
}

void GitWndHelper::ShowWindow(QWidget *widget) {
  if (nullptr == widget) {
    return;
  }
  GitWndMapItr itr = this->find(widget);
  if (itr == git_wnds_maps_.end()) {
    return;
  }
  itr->second->ShowWindow(true);
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
  GitWndMapItr itr = git_wnds_maps_.find(hwnd);
  if (itr != git_wnds_maps_.end()) {
    return itr->second->GetSmartWidget();
  } else {
    return nullptr;
  }
}

void GitWndHelper::init() {}

bool GitWndHelper::find(HWND git_hwnd, GitWndWrap **git_wrap) {
  if (git_hwnd == INVALID_HANDLE_VALUE || 0 == git_hwnd) return false;
  GitWndMapItr ite =
      std::find_if(git_wnds_maps_.begin(), git_wnds_maps_.end(),
                   [git_hwnd](GitWndPair &git_pair) {
                     return git_pair.second->GetGitWnd() == git_hwnd;
                   });
  if (ite == git_wnds_maps_.end()) return false;
  *git_wrap = (ite->second);
  return true;
}

GitWndMapItr GitWndHelper::find(QWidget *widget) {
  if (nullptr == widget) return git_wnds_maps_.end();
  GitWndMapItr itr =
      std::find_if(git_wnds_maps_.begin(), git_wnds_maps_.end(),
                   [widget](GitWndPair &git_pair) {
                     return git_pair.second->GetSmartWidget() == widget;
                   });
  return itr;
}