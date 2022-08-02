#ifndef pipe_server_h
#define pipe_server_h

#include <string>

#include <windows.h>

using std::wstring;

class PipeServer
{
public:
    PipeServer();
    ~PipeServer();
    bool Start(const wstring &pipe_name);
    void Stop();
    bool Run();

protected:
    void Close();

private:
    bool stop_ = false;
    HANDLE pipe_ = INVALID_HANDLE_VALUE;
    wstring pipe_name_;

};

#endif // pipe_server_h