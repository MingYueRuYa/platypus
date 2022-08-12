#ifndef common_h
#define common_h

#include <QtCore/QObject>
#include <Windows.h>

namespace Common {
QString GetInstallGitPath();
bool StartProcess(const QString &process_path, const QString &command_line,
                  int nShow, DWORD flags = STARTF_USESHOWWINDOW);
}  // namespace Common

#endif  // common_h