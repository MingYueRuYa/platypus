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
}  // namespace Common