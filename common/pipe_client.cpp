#include "pipe_client.h"

#include <windows.h>

#include <iostream>
#include <sstream>

PipeClient::PipeClient() {}

PipeClient::~PipeClient() {}

bool PipeClient::Read(const wstring &pipe_name) { return true; }

bool PipeClient::Write(const wstring &pipe_name, const wstring &msg) {
  DWORD num_rcv = 0;
  if (pipe_name.empty()) return false;
  std::ostringstream oss;
  // 打开指定命名管道
  HANDLE pipe = INVALID_HANDLE_VALUE;
  while (1) {
    pipe = ::CreateFileW(pipe_name.c_str(), GENERIC_WRITE, 0, nullptr,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (INVALID_HANDLE_VALUE != pipe) {
      OutputDebugStringA("pipe client create pipe successful.");
      break;
    }
    if (::GetLastError() != ERROR_PIPE_BUSY) {
      oss.clear();
      oss << "Failed to open the appointed named pipe!Error code: "
          << ::GetLastError() << "\n";
      OutputDebugStringA(oss.str().c_str());
      return false;
    }
    if (!WaitNamedPipeW(pipe_name.c_str(), 1000 * 2)) {
      printf("Could not open pipe: 2 second wait timed out.");
      return false;
    }
  }
  if (::WriteFile(pipe, msg.c_str(), 1024, &num_rcv, nullptr)) {
    oss.clear();
    oss << "Message sent successfully...\n";
    OutputDebugStringA(oss.str().c_str());
  } else {
    oss.clear();
    oss << "Failed to send message!Error code: " << ::GetLastError() << "\n";
    OutputDebugStringA(oss.str().c_str());
    OutputDebugStringW(msg.c_str());
    ::CloseHandle(pipe);
    return false;
  }
  ::CloseHandle(pipe);
  return true;
}
