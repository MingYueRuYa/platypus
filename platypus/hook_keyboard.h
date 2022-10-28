#ifndef hook_keyboard_h
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
  TAB_CTRL = 1,
  TAB_CTRL_SHIFT,
  CTRL_A,
  CTRL_SHIFT_W
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
  void insert(DWORD vkcode);
  void setNotifyCallBack(NotifyCallBack callBack) { callBack_ = callBack; }

 private:
  void CheckKeyBoard();
  void sendShortcut(const std::set<DWORD> &keySet);
  HookShortCut::Shortcut containsShortcut(const std::set<DWORD> &keySet);

 private:
  HHOOK keyboardhook = NULL;
  HHOOK mousehook = NULL;

  std::list<DWORD> keyboard_;
  bool checkable_ = true;
  std::mutex mutex_;
  NotifyCallBack callBack_ = nullptr;
};

#endif  // hook_keyboard_h