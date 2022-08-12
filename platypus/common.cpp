#include "common.h"

#include <Windows.h>

#include <QtCore/QSettings>

#include "registerhelper.h"

namespace Common {
QString GetInstallGitPath() {
  wstring reg_sub = TEXT(R"(SOFTWARE\GitForWindows)");
  XIBAO::RegisterHelper reg_helper(HKEY_LOCAL_MACHINE, reg_sub,
                                   KEY_QUERY_VALUE | KEY_WOW64_64KEY);
  wstring value = L"";
  reg_helper.GetSZ(L"InstallPath", value);

  // 在尝试在32位注册表下读取
  if (value.empty()) {
    reg_helper =
        XIBAO::RegisterHelper(HKEY_LOCAL_MACHINE, reg_sub, KEY_QUERY_VALUE);
    reg_helper.GetSZ(L"InstallPath", value);
  }
  return QString::fromStdWString(value);
}

bool StartProcess(const QString &process_path, const QString &command_line,
                  int nShow, DWORD flags) {
  STARTUPINFO StartInfo;
  ZeroMemory(&StartInfo, sizeof(StartInfo));
  StartInfo.wShowWindow = SW_HIDE;
  StartInfo.dwFlags = flags;
  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));
  wchar_t command[1024] = {0};
  lstrcpyW(command, command_line.toStdWString().c_str());
  BOOL result = CreateProcessW(process_path.toStdWString().c_str(),
                               command, NULL, NULL,
                               false, 0, NULL, NULL, &StartInfo, &pi);
  if (!result) {
    return false;
  }
  ::CloseHandle(pi.hProcess);
  ::CloseHandle(pi.hThread);
  return true;
}

}  // namespace Common