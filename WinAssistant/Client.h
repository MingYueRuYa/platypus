#pragma once
#include <windows.h>  
#include <vector>
#include <functional>
#include <iostream>
#include <tuple>

// #include "../include/head.h"

class Client
{
	typedef std::function<void(PBYTE pPayload, UINT64 size)> _EVENT;  //可以注册的事件类型

public:
	Client();
	Client(const Client&) = delete;
	~Client() {}

	void init(const std::string& mapName, size_t size, const std::string& eventName);

	void send(const std::string& eventType, LPVOID pdata, size_t dataSize, const _EVENT& callback);

private:

	HANDLE m_hMap;
	size_t m_mapSize;
	std::string m_EventName;
	//HANDLE m_hSendEvent;
	
};