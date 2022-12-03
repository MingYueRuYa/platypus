#include "hook_keyboard.h"

#include <map>

using Shortcut = HookShortCut::Shortcut;

LRESULT WINAPI MyMouseCallback(int nCode, WPARAM wParam,
                               LPARAM lParam);  // callback declaration
LRESULT WINAPI MyKeyBoardCallback(int nCode, WPARAM wParam, LPARAM lParam);

namespace KeyBoard
{
  enum class KeyBoardValue
  {
    VK_Unknow = 0,
    VK_A = 0x41,
    VK_W = 0x57
  };
}

using KeyBoardVal = KeyBoard::KeyBoardValue;

MyHook::~MyHook() {
  stop();
  UninstallHook();  // if we close, let's uninstall our hook
}

void MyHook::InstallHook() {
  /*
  SetWindowHookEx(
  WM_MOUSE_LL = mouse low level hook type,
  MyMouseCallback = our callback function that will deal with system messages
  about mouse NULL, 0);

  c++ note: we can check the return SetWindowsHookEx like this because:
  If it return NULL, a NULL value is 0 and 0 is false.
  */
  //	if (!(hook = SetWindowsHookEx(WH_MOUSE_LL, MyMouseCallback, NULL, 0))){
  //		printf_s("Error: %x \n", GetLastError());
  //	}

  if (!(keyboardhook =
            SetWindowsHookEx(WH_KEYBOARD_LL, MyKeyBoardCallback, NULL, 0))) {
    printf_s("Error: %x \n", GetLastError());
  }
}

void MyHook::UninstallHook() {
  /*
  uninstall our hook using the hook handle
  */
  UnhookWindowsHookEx(keyboardhook);
}

void MyHook::stop() { checkable_ = false; }

void MyHook::insert(DWORD vkcode) { keyboard_.push_back(vkcode); }

void MyHook::start() {
  InstallHook();

  std::thread thr(&MyHook::CheckKeyBoard, this);
  thr.detach();
}

void MyHook::CheckKeyBoard() {
  const std::map<DWORD, DWORD> keyMap = {
      {VK_LCONTROL, VK_LCONTROL}, {VK_TAB, VK_TAB}, {VK_LSHIFT, VK_LSHIFT}, {(int)KeyBoardVal::VK_A, (int)KeyBoardVal::VK_A}, {(int)KeyBoardVal::VK_W, (int)KeyBoardVal::VK_W} };
  while (checkable_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(80));

    std::lock_guard<std::mutex> lock(mutex_);
    if (keyboard_.empty()) {
      continue;
    }

    printf("list size:%d\n", keyboard_.size());
    printf("------------------------\n");
    for (auto i = keyboard_.begin(); i != keyboard_.end(); i++) {
      printf("%d ", *i);
    }
    printf("\n");
    printf("------------------------\n");

    std::set<DWORD> keySet = {};
    while (!keyboard_.empty()) {
      keySet.clear();
      // 直接提取前2个或前3个键
      // 1.优先提取前3个
      bool pop_data = true;
      if (keyboard_.size() >= 3) {
        auto ibeg = keyboard_.begin();
        for (int i = 0; ibeg != keyboard_.end(); ++i, ++ibeg) {
          if (i == 3) break;
          auto keyfind = keyMap.find(*ibeg);
          if (keyfind != keyMap.end()) keySet.insert(keyfind->second);
        }

        Shortcut shortcut = containsShortcut(keySet);
        if (Shortcut::Unknow != shortcut) {
          printf("we get tab + shift + control keys\n");
          keyboard_.pop_front();
          keyboard_.pop_front();
          keyboard_.pop_front();
          pop_data = false;
        }
      }
      if (keyboard_.size() >= 2) {
        auto ibeg = keyboard_.begin();
        for (int i = 0; ibeg != keyboard_.end(); ++i, ++ibeg) {
          if (i == 2) break;
          auto keyfind = keyMap.find(*ibeg);
          if (keyfind != keyMap.end()) keySet.insert(keyfind->second);
        }

        Shortcut shortcut = containsShortcut(keySet);
        if (Shortcut::Unknow != shortcut) {
          printf("we get tab + control keys\n");
          keyboard_.pop_front();
          keyboard_.pop_front();
          pop_data = false;
        }
      }

      if (pop_data && !keyboard_.empty()) keyboard_.pop_front();
    } //while
    sendShortcut(keySet);
  }
}

void MyHook::sendShortcut(const std::set<DWORD>& keySet) {
  Shortcut shortcut = containsShortcut(keySet);
  if (Shortcut::Unknow != shortcut && nullptr != callBack_)
        callBack_((int)shortcut);
}

HookShortCut::Shortcut MyHook::containsShortcut(const std::set<DWORD>& keySet) {
  if (keySet.empty()) return Shortcut::Unknow;
  if (keySet.end() != keySet.find(VK_LCONTROL) &&
      keySet.end() != keySet.find(VK_TAB) &&
      keySet.end() != keySet.find(VK_LSHIFT)) {
    printf("Get TAB CTRL SHIFT key");
    return Shortcut::TAB_CTRL_SHIFT;
  }
  
  if (keySet.end() != keySet.find(VK_LCONTROL) &&
      keySet.end() != keySet.find((int)KeyBoardVal::VK_W) &&
      keySet.end() != keySet.find(VK_LSHIFT)) {
    printf("Get CTRL SHIFT W key");
    return Shortcut::CTRL_SHIFT_W;
  }

  if (keySet.end() != keySet.find(VK_LCONTROL) &&
      keySet.end() != keySet.find(VK_TAB)) {
    printf("Get TAB CTRL key");
    return Shortcut::TAB_CTRL;
  }

  // 0x41 -> A
  if (keySet.end() != keySet.find(VK_LCONTROL) &&
      keySet.end() != keySet.find((int)KeyBoardVal::VK_A) &&
      keySet.end() != keySet.find(VK_LSHIFT)) {
    printf("Get CTRL + SHIFT + A key");
    return Shortcut::CTRL_SHIFT_A;
  }

  return Shortcut::Unknow;
}

LRESULT WINAPI MyMouseCallback(int nCode, WPARAM wParam, LPARAM lParam) {
  MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;  // WH_MOUSE_LL struct
  /*
  nCode, this parameters will determine how to process a message
  This callback in this case only have information when it is 0 (HC_ACTION):
  wParam and lParam contain info

  wParam is about WINDOWS MESSAGE, in this case MOUSE messages.
  lParam is information contained in the structure MSLLHOOKSTRUCT
  */

  if (nCode ==
      0) {  // we have information in wParam/lParam ? If yes, let's check it:
    if (pMouseStruct != NULL) {  // Mouse struct contain information?
      // printf_s("Mouse Coordinates: x = %i | y = %i \n", pMouseStruct->pt.x,
      // pMouseStruct->pt.y);
    }

    switch (wParam) {
      case WM_LBUTTONUP: {
        printf_s("LEFT CLICK UP\n");
      } break;
      case WM_LBUTTONDOWN: {
        printf_s("LEFT CLICK DOWN\n");
      } break;
      case WM_RBUTTONUP: {
        printf_s("RIGHT CLICK UP\n");
      } break;
      case WM_RBUTTONDOWN: {
        printf_s("RIGHT CLICK DOWN\n");
      } break;
      case WM_CHAR: {
        printf_s("%c", (TCHAR)wParam);
      } break;
    }
  }

  /*
  Every time that the nCode is less than 0 we need to CallNextHookEx:
  -> Pass to the next hook
  MSDN: Calling CallNextHookEx is optional, but it is highly recommended;
  otherwise, other applications that have installed hooks will not receive hook
  notifications and may behave incorrectly as a result.
  */
  return CallNextHookEx(MyHook::Instance().MouseHook(), nCode, wParam, lParam);
}

LRESULT WINAPI MyKeyBoardCallback(int nCode, WPARAM wParam, LPARAM lParam) {
  KBDLLHOOKSTRUCT* pKeyStruct = (KBDLLHOOKSTRUCT*)lParam;

  if (nCode == 0 && nullptr != pKeyStruct) {
    DWORD vkcode = pKeyStruct->vkCode;
    MyHook::Instance().insert(vkcode);
  }
  return CallNextHookEx(MyHook::Instance().KeyBoardHook(), nCode, wParam,
                        lParam);
}