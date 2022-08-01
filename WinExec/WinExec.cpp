#include "stdafx.h"
#include "WinExec.h"
#include "Client.h"

#include <TlHelp32.h>
#include <Windows.h>

#include <algorithm>
#include <map>
#include <string>
#include <thread>

#include "../include/const.h"
#include "../include/head.h"
#include "Server.h"
#include "WinAssistant.h"

#define MAX_LOADSTRING 100

#ifdef X64
#pragma comment(lib, "WinAssistant_x64.lib")
#else
#pragma comment(lib, "WinAssistant.lib")
#endif

using std::wstring;

#define WM_SEND_PROCESS_ID WM_USER + 1000 + 1

#define THREAD_ID DWORD
#define PROCESS_ID DWORD

// 全局变量:
HINSTANCE hInst;                      // 当前实例
TCHAR szTitle[MAX_LOADSTRING];        // 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];  // 主窗口类名
std::map<PROCESS_ID, CWinAssistant *> g_MapWndAssistant;
using MapAssistPair = std::pair<PROCESS_ID, CWinAssistant *>;
Server *g_Server = nullptr;
std::thread g_ServerThread;
HWND g_HWND = 0x0;

bool bStopEnum = false;
std::thread g_EnumProcessThread;

std::map<PROCESS_ID, HWND> g_MapProcessIDHWND;
using MapPIDHWNDPair = std::pair<PROCESS_ID, HWND>;

// 此代码模块中包含的函数的前向声明:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void RegisterDLL(HWND wnd, PROCESS_ID process_id);
void UnregisterDLL(PROCESS_ID process_id);
HWND GetWndByProcessID(DWORD dwProcessID);
void StartServer();
void EnumProcess(const wstring &exeName);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine,
                       _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  // TODO:  在此放置代码。
  MSG msg;
  HACCEL hAccelTable;

  // 初始化全局字符串
  LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadString(hInstance, IDC_WINEXEC, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // 执行应用程序初始化:
  if (!InitInstance(hInstance, nCmdShow)) {
    return FALSE;
  }

  g_ServerThread = std::thread(StartServer);
  g_EnumProcessThread = std::thread(EnumProcess, L"mintty.exe");

  hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINEXEC));

  // 主消息循环:
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  //   std::for_each(std::begin(g_MapWndAssistant), std::end(g_MapWndAssistant),
  //                 [](const MapAssistPair &pair) {
  //                   pair.second->Unregister();
  //                   delete pair.second;
  //                 });

  g_Server->stop();
  g_ServerThread.join();
  delete g_Server;

  bStopEnum = true;
  g_EnumProcessThread.join();

  return (int)msg.wParam;
}

//
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance) {
  WNDCLASSEX wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINEXEC));
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCE(IDC_WINEXEC);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  return RegisterClassEx(&wcex);
}

//
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
  HWND hWnd;

  hInst = hInstance;  // 将实例句柄存储在全局变量中

  hWnd =
      CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                   0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

  if (!hWnd) {
    return FALSE;
  }

  g_HWND = hWnd;
  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

//
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {
  int wmId, wmEvent;
  PAINTSTRUCT ps;
  HDC hdc;

  switch (message) {
    case WM_COMMAND:
      wmId = LOWORD(wParam);
      wmEvent = HIWORD(wParam);
      // 分析菜单选择:
      switch (wmId) {
        case IDM_ABOUT: {
          DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
        } break;
        case IDM_EXIT:
          DestroyWindow(hWnd);
          break;
        default:
          return DefWindowProc(hWnd, message, wParam, lParam);
      }
      break;
    case WM_PAINT:
      hdc = BeginPaint(hWnd, &ps);
      // TODO:  在此添加任意绘图代码...
      EndPaint(hWnd, &ps);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_NULL: {
      //   if (1 == lParam)
      //     RegisterDLL(wParam);
      //   else if (0 == lParam)
      //     UnregisterDLL(wParam);
    } break;
    case WM_SEND_PROCESS_ID: {
      PROCESS_ID process_id = (PROCESS_ID)(wParam);
      if (g_MapProcessIDHWND.find(process_id) == g_MapProcessIDHWND.end()) {
        HWND wnd = GetWndByProcessID(process_id);
        if (wnd == NULL) {
          OutputDebugStringA("Get Wnd error.");
        } else {
          g_MapProcessIDHWND[process_id] = wnd;
          RegisterDLL(wnd, process_id);
        }
      }

    } break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
    case WM_INITDIALOG:
      return (INT_PTR)TRUE;

    case WM_COMMAND:
      if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
      }
      break;
  }
  return (INT_PTR)FALSE;
}

void RegisterDLL(HWND targetWnd, PROCESS_ID process_id) {
  if (0 == targetWnd) return;
  THREAD_ID thread_id = GetWindowThreadProcessId(targetWnd, NULL);

  CWinAssistant *assist = new CWinAssistant();
  assist->Register(thread_id);
  g_MapWndAssistant[process_id] = assist;
}

void UnregisterDLL(PROCESS_ID process_id) {
  if (0 == process_id) return;

  CWinAssistant *assist = g_MapWndAssistant[process_id];
  delete assist;
  g_MapWndAssistant.erase(process_id);
  g_MapProcessIDHWND.erase(process_id);
}

typedef struct ST_WNDINFO {
  HWND hWnd;
  DWORD dwProcessId;
} WNDINFO, *LPWNDINFO;

HWND GetWndByProcessID(DWORD dwProcessID) {
  WNDINFO wndInfo;
  wndInfo.hWnd = nullptr;
  wndInfo.dwProcessId = dwProcessID;
  ::EnumWindows(
      [](HWND hWnd, LPARAM lParam) -> BOOL {
        DWORD dwProcessId = 0;
        ::GetWindowThreadProcessId(hWnd, &dwProcessId);
        WNDINFO *pInfo = reinterpret_cast<LPWNDINFO>(lParam);
        if (dwProcessId == pInfo->dwProcessId) {
          pInfo->hWnd = hWnd;
          return FALSE;
        }
        return TRUE;
      },
      reinterpret_cast<LPARAM>(&wndInfo));

  return wndInfo.hWnd;
}

void Test(PBYTE pPayload, UINT64 size) {}

bool FindWndTitle(PBYTE pBuffer, UINT64 &size) {
  if (0 != size) {
    OutputDebugStringW((wchar_t *)(pBuffer));
    // Client client;
    // client.init(platypus_shm_name, MAX_SHM_SIZE, platypus_evt_name);
    // client.send(function_name, (PVOID)pBuffer, size, Test);
  }
  return true;
}

bool WndExit(PBYTE pBuffer, UINT64 &size) {
  if (size != 0) {
    wstring log_msg = wstring(L"process exited ,id:") + (wchar_t *)(pBuffer);
    OutputDebugStringW(log_msg.c_str());
    unsigned long process_id = std::stoul((wchar_t *)pBuffer);
    g_MapProcessIDHWND.erase(process_id);
    UnregisterDLL(process_id);
  }
  return true;
}

bool Stop(PBYTE pBuffer, UINT64 &size) {
  return false;
}

void StartServer() {
  g_Server = new Server();
  g_Server->createShm(dll_shm_name, MAX_SHM_SIZE);
  g_Server->appendRcvEvent(function_name, FindWndTitle);
  g_Server->appendRcvEvent(wnd_exit_name, WndExit);
  g_Server->appendRcvEvent(exit_name, Stop);
  g_Server->eventLoop(dll_evt_name);
}

void EnumProcess(const wstring &exeName) {
  while (!bStopEnum) {
    PROCESSENTRY32 processEntry = {0};
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
      return;
    }
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    BOOL bRet = Process32First(hProcessSnap, &processEntry);
    while (bRet) {
      wstring exe_file = processEntry.szExeFile;
      if (exe_file == exeName) {
        ::PostMessageW(g_HWND, WM_SEND_PROCESS_ID, processEntry.th32ProcessID,
                       0);
      }
      bRet = Process32Next(hProcessSnap, &processEntry);
    }
    CloseHandle(hProcessSnap);
    // sleep 1s
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
  }
}
