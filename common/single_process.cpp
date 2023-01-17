#include "single_process.h"

SingleProcess::SingleProcess(const wstring &key) {
  if (key.empty()) throw std::exception("the key is allowed empty");

  is_exits_ = false;
  handle_ = ::CreateMutexW(NULL, TRUE, key.c_str());
  if (::GetLastError() == ERROR_ALREADY_EXISTS) {
    ::CloseHandle(handle_);
    handle_ = NULL;
    is_exits_ = true;
  }
}

SingleProcess::~SingleProcess() { ::CloseHandle(handle_); }

bool SingleProcess::isExist() const { return is_exits_; }