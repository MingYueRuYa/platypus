#ifndef process_utils_h
#define process_utils_h

#include <windows.h>

class ProcessUtils {
 public:
  enum class ProcessType { Unknown = 0, Process_32, Process_64 };

 public:
  ProcessUtils() {}
  ~ProcessUtils() {}

  BOOL GetOSVerIs64Bit() {
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

  // 判断进程是32位还是64位
  ProcessType GetProcessIsWOW64(HANDLE hProcess) {
    int nRet = -1;
    ProcessType process_type = ProcessType::Unknown;

    typedef BOOL(WINAPI * LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    BOOL bIsWow64 = FALSE;
    BOOL bRet;
    DWORD nError;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
        GetModuleHandle(_T("kernel32"), _T("IsWow64Process"));
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
};

#endif  // process_utils_h