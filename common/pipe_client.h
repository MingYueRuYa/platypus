#ifndef pipe_client_h
#define pipe_client_h

#include <string>

using std::wstring;

class PipeClient
{
public:
    PipeClient();
    ~PipeClient();
    bool Read(const wstring &pipe_name);
    bool Write(const wstring &pipe_name, const wstring &msg);
};

#endif // pipe_client_h