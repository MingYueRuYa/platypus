﻿#ifndef hook_keyboard_h
#define hook_keyboard_h
#include <Windows.h>

#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <set>

namespace HookShortCut {
enum class Shortcut {
  Unknow = 0,
  TAB_CTRL = 1,    // tab向前滚动
  TAB_CTRL_SHIFT,  // tab向后滚动
  CTRL_SHIFT_A,    // 关闭tab
  CTRL_SHIFT_W,    // 新建tab
  ALT_F11,         // 放大窗口 or 恢复窗口
};
}

class MyHook {
  using NotifyCallBack = std::function<void(int)>;

 public:
  // single ton
  static MyHook &Instance() {
    static MyHook myHook;
    return myHook;
  }
  ~MyHook();
  void InstallHook();    // function to install our hook
  void UninstallHook();  // function to uninstall our hook
  void start();
  void stop();

  HHOOK KeyBoardHook() { return keyboardhook; }
  HHOOK MouseHook() { return mousehook; }
  void insert(DWORD vkcode, bool alt);
  void setNotifyCallBack(NotifyCallBack callBack) { callBack_ = callBack; }

 private:
  void CheckKeyBoard();
  HookShortCut::Shortcut sendShortcut(const std::set<DWORD> &keySet);
  HookShortCut::Shortcut containsShortcut(const std::set<DWORD> &keySet);

 private:
  HHOOK keyboardhook = NULL;
  HHOOK mousehook = NULL;

  std::list<DWORD> keyboard_;
  bool checkable_ = true;
  //TODO：使用原子变量
  bool receive_key_ = true;
  std::mutex mutex_;
  NotifyCallBack callBack_ = nullptr;
};

#endif  // hook_keyboard_h