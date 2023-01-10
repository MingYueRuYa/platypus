#include "pipe_server.h"

#include <iostream>
#include <sstream>

#define BUF_SIZE 1024

PipeServer::PipeServer() {}

PipeServer::~PipeServer() {
  if (INVALID_HANDLE_VALUE != pipe_) ::CloseHandle(pipe_);
}

bool PipeServer::Start(const wstring &pipe_name) {
  pipe_name_ = pipe_name;
  // 创建命名管道,命名为MyPipe,消息只能从客户端流向服务器,读写数据采用阻塞模式,字节流形式,超时值置为0表示采用默认的50毫秒
  pipe_ = ::CreateNamedPipeW(
      pipe_name_.c_str(), PIPE_ACCESS_INBOUND, PIPE_READMODE_BYTE | PIPE_WAIT,
      PIPE_UNLIMITED_INSTANCES, BUF_SIZE, BUF_SIZE, 50 * 1000, nullptr);
  if (pipe_ == INVALID_HANDLE_VALUE) {
    OutputDebugStringA("create pipe server error.");
    return false;
  } else {
    OutputDebugStringA("create pipe server successful.");
    return true;
  }
}

void PipeServer::Stop() {
  wchar_t buf_msg[] = L"exit";
  DWORD num_rcv = 0;  // 实际接收到的字节数
  stop_ = true;
  HANDLE pipe = INVALID_HANDLE_VALUE;
  while (1) {
    pipe = ::CreateFileW(pipe_name_.c_str(), GENERIC_WRITE, 0, nullptr,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (pipe != INVALID_HANDLE_VALUE) {
      break;
    }
    DWORD error_code = ::GetLastError();
    std::ostringstream oss;
    if (ERROR_PIPE_BUSY == error_code || ERROR_FILE_NOT_FOUND == error_code) {
      oss.clear();
      oss << "Pipe file is busy now or not found, error code:" << error_code
          << "\n";
      OutputDebugStringA(oss.str().c_str());
      if (!WaitNamedPipeW(pipe_name_.c_str(), 1000 * 2)) {
        OutputDebugStringA("Could not open pipe: 2 second wait timed out.");
        return;
      }
    } else {
      oss.clear();
      oss << "pipe server ready to stop, create pipe error, last error code:"
          << GetLastError();
      OutputDebugStringA(oss.str().c_str());
      return;
    }
  }
  std::ostringstream oss;
  if (::WriteFile(pipe, buf_msg,
                  static_cast<DWORD>(wcslen(buf_msg) * sizeof(wchar_t)),
                  &num_rcv, nullptr)) {
    oss.clear();
    oss << "Message sent successfully...\n";
    OutputDebugStringA(oss.str().c_str());
  } else {
    oss.clear();
    oss << "Failed to send message!Error code: " << ::GetLastError() << "\n";
    OutputDebugStringA(oss.str().c_str());
    OutputDebugStringW(buf_msg);
    ::CloseHandle(pipe);
  }

  ::CloseHandle(pipe);
}

bool PipeServer::Run() {
  wchar_t buf_msg[BUF_SIZE];
  DWORD num_rcv;  // 实际接收到的字节数
  while (1) {
    // 等待命名管道客户端连接
    if (::ConnectNamedPipe(pipe_, nullptr)) {
      memset(buf_msg, 0, BUF_SIZE);
      // 读取数据
      if (::ReadFile(pipe_, buf_msg, BUF_SIZE, &num_rcv, nullptr)) {
        OutputDebugStringW((wchar_t *)buf_msg);
        DisconnectNamedPipe(pipe_);
        if (wstring((wchar_t *)buf_msg) == L"exit") {
          OutputDebugStringW(L"pipe ready to exit");
          break;
        } else {
          func_(buf_msg);
        }
      } else {
        ::CloseHandle(pipe_);
        return true;
      }
    }
  }
  return false;
}

void PipeServer::BindFunc(_FUNC func) { func_ = func; }