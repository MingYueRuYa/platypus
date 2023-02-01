#ifndef PLAYPUS_CONST_H
#define PLAYPUS_CONST_H

#define MAX_SHM_SIZE 40000

// git_register_exec与git_plugin dll通信
const char* dll_shm_name = "register@plugin@shm_name";
const char* dll_evt_name = "register@plugin@event_name";

// git_register_exec与platypus通信
const char* platypus_shm_name = "register@platypus@shm_name";
const char* platypus_evt_name = "register@platypus@event_name";

const char* function_name = "find_wnd_title";
const char* wnd_exit_name = "wnd_exit";
const char* exit_name = "exit";
const char* data_transfor_name = "data_transfor";

// pipe info
const wchar_t *pipe_name = L"\\\\.\\pipe\\platypus@!#pipe";
const int kBUFF_SIZE = 1024;

#endif  // PLAYPUS_CONST_H
