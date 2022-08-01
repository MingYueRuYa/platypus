#include "pipe_client.h"

#include <windows.h>

#include <iostream>

PipeClient::PipeClient() {}

PipeClient::~PipeClient() {}

bool PipeClient::Read(const wstring &pipe_name) { return true; }

bool PipeClient::Write(const wstring &pipe_name, const wstring &msg) {
  DWORD num_rcv = 0;
  if (pipe_name.empty()) return false;
  //打开指定命名管道
  HANDLE pipe = ::CreateFileW(pipe_name.c_str(), GENERIC_WRITE, 0, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (pipe == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to open the appointed named pipe!Error code: "
              << ::GetLastError() << "\n";
    return false;
  } else {
    if (::WriteFile(pipe, msg.c_str(), 1024, &num_rcv, nullptr)) {
      std::cout << "Message sent successfully...\n";
    } else {
      std::cerr << "Failed to send message!Error code: " << ::GetLastError() << "\n";
      ::CloseHandle(pipe);
      return true;
    }
  }
  ::CloseHandle(pipe);
  return true;
}
