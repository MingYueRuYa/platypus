#ifndef PLAYPUS_CONST_H
#define PLAYPUS_CONST_H

#define MAX_SHM_SIZE 40000

// WinExec与dll通信
const char* dll_shm_name = "platypus@shm_name";
const char* dll_evt_name = "platypus@event_name";

// WinExec与platypus通信
const char* platypus_shm_name = "platypus@shm_name";
const char* platypus_evt_name = "platypus@event_name";

const char* function_name = "find_wnd_title";
const char* wnd_exit_name = "wnd_exit";
const char* exit_name = "exit";

#endif  // PLAYPUS_CONST_H
