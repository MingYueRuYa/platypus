#ifndef process_utils_h
#define process_utils_h

#include <Tlhelp32.h>
#include <windows.h>

class ProcessUtils {
 public:
  enum class ProcessType { Unknown = 0, Process_32, Process_64 };

 public:
  ProcessUtils() {}
  ~ProcessUtils() {}

  static BOOL GetOSVerIs64Bit() {
    BOOL bRet = FALSE;
    SYSTEM_INFO si;
    typedef VOID(__stdcall * GETNATIVESYSTEMINFO)(LPSYSTEM_INFO lpSystemInfo);

    GETNATIVESYSTEMINFO fnGetNativeSystemInfo;
    fnGetNativeSystemInfo = (GETNATIVESYSTEMINFO)GetProcAddress(
        GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
    if (fnGetNativeSystemInfo != NULL) {
      fnGetNativeSystemInfo(&si);
      if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
          si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
        bRet = TRUE;
      }
    }

    return bRet;
  }

  /// @brief Determine whether the process is 32-bit or 64-bit
  /// @param hProcess Process handle
  /// @return ProcessType:Process_32 or Process_64
  static ProcessType GetProcessIsWOW64(HANDLE hProcess) {
    int nRet = -1;
    ProcessType process_type = ProcessType::Unknown;

    typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    BOOL bIsWow64 = FALSE;
    BOOL bRet;
    DWORD nError;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
        GetModuleHandle(L"kernel32"), "IsWow64Process");
    if (NULL != fnIsWow64Process) {
      bRet = fnIsWow64Process(hProcess, &bIsWow64);
      if (bRet == 0) {
        nError = GetLastError();
      } else {
        if (GetOSVerIs64Bit()) {
          // system is AMD64 or IA64
          if (bIsWow64) {
            nRet = 1;
            process_type = ProcessType::Process_32;
          } else {
            nRet = 0;
            process_type = ProcessType::Process_64;
          }
        } else {
          // system is 32bit
          process_type = ProcessType::Process_32;
        }
      }
    }

    return process_type;
  }

  /// @brief Kill process by application name
  /// @param appName App name like: mspaint.exe
  /// @param all Whether to kill all process
  static void KillProcess(const wchar_t *appName, bool all) {
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32W pEntry;
    pEntry.dwSize = sizeof(pEntry);
    BOOL hRes = Process32FirstW(hSnapShot, &pEntry);
    while (hRes) {
      if (wcscmp(pEntry.szExeFile, appName) == 0) {
        HANDLE hProcess =
            OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
        if (hProcess != NULL) {
          TerminateProcess(hProcess, 0x09);
          CloseHandle(hProcess);
          if (!all) break;
        }
      }
      hRes = Process32NextW(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
  }

  /// @brief Find process by App name
  /// @param appName App name like: mspaint.exe
  /// @return true or false
  static bool FindProcess(const wchar_t *appName) {
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32W pEntry;
    pEntry.dwSize = sizeof(pEntry);
    BOOL hRes = Process32FirstW(hSnapShot, &pEntry);
    bool find = false;
    while (hRes) {
      if (wcscmp(pEntry.szExeFile, appName) == 0) {
        HANDLE hProcess =
            OpenProcess(PROCESS_TERMINATE, 0, (DWORD)pEntry.th32ProcessID);
        if (hProcess != NULL) {
          find = true;
          break;
        }
      }
      hRes = Process32NextW(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
    return find;
  }
};

#endif  // process_utils_h