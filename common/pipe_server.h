#ifndef pipe_server_h
#define pipe_server_h

#include <windows.h>

#include <functional>
#include <string>

using std::wstring;

class PipeServer {
 public:
  typedef std::function<void(const wchar_t*)> _FUNC;  //可以注册的事件类型
 public:
  PipeServer();
  ~PipeServer();
  bool Start(const wstring& pipe_name);
  void Stop();
  bool Run();
  void BindFunc(_FUNC func);

 protected:
  void Close();

 private:
  bool stop_ = false;
  HANDLE pipe_ = INVALID_HANDLE_VALUE;
  wstring pipe_name_;
  _FUNC func_;
};

#endif  // pipe_server_h