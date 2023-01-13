#ifndef single_process_h
#define single_process_h

#include <Windows.h>
#include <string>

using std::wstring;

class SingleProcess {
 public:
  explicit SingleProcess(const wstring &key);
  ~SingleProcess();
  bool isExist() const;

private:
  HANDLE handle_;
  bool is_exits_;
};

#endif  // single_process_h