#include "pipe_server.h"

#include <iostream>

#define BUF_SIZE 1024

PipeServer::PipeServer() {}

PipeServer::~PipeServer() {
  if (INVALID_HANDLE_VALUE != pipe_) ::CloseHandle(pipe_);
}

bool PipeServer::Start(const wstring &pipe_name) {
  pipe_name_ = wstring(L"\\\\.\\pipe\\") + pipe_name;
  //创建命名管道,命名为MyPipe,消息只能从客户端流向服务器,读写数据采用阻塞模式,字节流形式,超时值置为0表示采用默认的50毫秒
  pipe_ = ::CreateNamedPipeW(
      pipe_name_.c_str(), PIPE_ACCESS_INBOUND, PIPE_READMODE_BYTE | PIPE_WAIT,
      PIPE_UNLIMITED_INSTANCES, BUF_SIZE, BUF_SIZE, 50 * 1000, nullptr);
  if (pipe_ == INVALID_HANDLE_VALUE) {
    return false;
  } else {
    std::cout << "Named pipe created successfully...\n";
    return true;
  }
}

void PipeServer::Stop() {
  wchar_t buf_msg[] = L"exit";
  DWORD num_rcv = 0;  //实际接收到的字节数
  stop_ = true;
  HANDLE pipe = ::CreateFileW(pipe_name_.c_str(), GENERIC_WRITE, 0, nullptr,
                              OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (pipe == INVALID_HANDLE_VALUE) {
    // TODO something
    return;
  } else {
    ::WriteFile(pipe, buf_msg, BUF_SIZE, &num_rcv, nullptr);
  }
  ::CloseHandle(pipe);
}

bool PipeServer::Run() {
  wchar_t buf_msg[BUF_SIZE];
  DWORD num_rcv;  //实际接收到的字节数
  while (1) {
    //等待命名管道客户端连接
    if (::ConnectNamedPipe(pipe_, nullptr)) {
      std::cout << "A client connected...\n";
      memset(buf_msg, 0, BUF_SIZE);
      //读取数据
      if (::ReadFile(pipe_, buf_msg, BUF_SIZE, &num_rcv, nullptr)) {
        OutputDebugStringW((wchar_t *)buf_msg);
        if (wstring((wchar_t *)buf_msg) == L"exit") {
          OutputDebugStringW(L"pipe ready to exit");
          break;
        }
      } else {
        ::CloseHandle(pipe_);
        return true;
      }
    }
  }
  return false;
}
