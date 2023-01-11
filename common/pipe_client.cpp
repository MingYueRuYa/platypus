#include "pipe_client.h"

#include <windows.h>

#include <format>
#include <iostream>
#include <sstream>
#include <string>

using string = std::string;

PipeClient::PipeClient() {}

PipeClient::~PipeClient() {}

bool PipeClient::Read(const wstring &pipe_name) { return true; }

bool PipeClient::Write(const wstring &pipe_name, const wstring &msg) {
  DWORD num_rcv = 0;
  if (pipe_name.empty()) return false;
  HANDLE pipe = INVALID_HANDLE_VALUE;
  while (1) {
    pipe = ::CreateFileW(pipe_name.c_str(), GENERIC_WRITE, 0, nullptr,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (INVALID_HANDLE_VALUE != pipe) {
      OutputDebugStringA("pipe client create pipe successful.");
      break;
    }
    DWORD error_code = ::GetLastError();
    if (error_code == ERROR_PIPE_BUSY || error_code == ERROR_FILE_NOT_FOUND) {
      string error_str = std::format(
          "Pipe file is busy now or not found, error code:{}", error_code);
      OutputDebugStringA(error_str.c_str());
      if (!WaitNamedPipeW(pipe_name.c_str(), 1000 * 2)) {
        OutputDebugStringA("Could not open pipe: 2 second wait timed out.");
        return false;
      }
    } else {
      string error_str = std::format(
          "Failed to open the appointed named pipe!Error code:{}", error_code);
      OutputDebugStringA(error_str.c_str());
      return false;
    }
  }
  if (::WriteFile(pipe, msg.c_str(),
                  static_cast<DWORD>(wcslen(msg.c_str()) * sizeof(wchar_t)),
                  &num_rcv, nullptr)) {
    OutputDebugStringA("Message sent successfully...\n");
  } else {
    string error_str =
        std::format("Failed to send message!Error code:{}", ::GetLastError());
    OutputDebugStringA(error_str.c_str());
    OutputDebugStringW(msg.c_str());
    ::CloseHandle(pipe);
    return false;
  }
  ::CloseHandle(pipe);
  return true;
}
