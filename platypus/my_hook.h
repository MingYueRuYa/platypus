
#include <Windows.h>

#include <iostream>
#include <list>
#include <mutex>
#include <functional>

class MyHook {

using NotifyCallBack = std::function<void(int)>; 

 public:
  // single ton
  static MyHook& Instance() {
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

 private:
  HHOOK keyboardhook = NULL;
  HHOOK mousehook = NULL;

  std::list<DWORD> keyboard_;
  bool checkable_ = true;
  std::mutex mutex_;
  NotifyCallBack callBack_ = nullptr;
};