#ifndef PLAYPUS_HEAD_H
#define PLAYPUS_HEAD_H

//每次传递数据的包头
struct Header
{
	char type[20];  //调用的功能类型
	UINT64  size;   //数据大小
};

//#define MAX_SHM_SIZE 40000 
//
//const char* shm_name = "platypus@shm_name";
//const char* evt_name = "platypus@event_name";
//
//const char* function_name = "find_wnd_title";

#endif // PLAYPUS_HEAD_H
