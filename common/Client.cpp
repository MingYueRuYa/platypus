#include "Client.h"

#include <format>

#include "../include/head.h"

Client::Client() : m_hMap(NULL), m_mapSize(0) {}

void Client::init(const std::string& mapName, size_t size,
                  const std::string& eventName) {
  // 首先试图打开一个命名的内存映射文件对象
  m_hMap = ::OpenFileMappingA(
      FILE_MAP_ALL_ACCESS,
      0,  // 如这个函数返回的句柄能由当前进程启动的新进程继承，则这个参数为TRUE。
      mapName.c_str());
  if (NULL == m_hMap)  // 打开失败
  {
    throw;
  }
  m_mapSize = size;

  m_EventName = eventName;
}

void Client::send(const std::string& eventType, LPVOID pdata, size_t dataSize,
                  const _EVENT& callback) {
  Header header;

  LPVOID pBuffer = ::MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  memset(pBuffer, 0, m_mapSize);

  // set data head
  strncpy_s(header.type, eventType.length() + 1, eventType.c_str(), 20);
  header.size = dataSize;

  // copy data to share memory (data head + valid data)
  memmove((PBYTE)pBuffer, &header, sizeof(header));

  PBYTE pPayload = (PBYTE)pBuffer + sizeof(header);
  memmove(pPayload, pdata, dataSize);

  std::string sndEventName = m_EventName + "ctos";
  HANDLE hSendEvent = OpenEventA(EVENT_ALL_ACCESS, NULL, sndEventName.c_str());
  if (hSendEvent == nullptr) {
    std::wstring error_info =
        std::format(L"Open event error code: {}", ::GetLastError());
    OutputDebugStringW(error_info.c_str());
    return;
  }
  SetEvent(hSendEvent);

  std::string rcvEventName = m_EventName + "stoc";
  HANDLE hRcvEvent = CreateEventA(NULL, FALSE, FALSE, rcvEventName.c_str());
  if (hRcvEvent == nullptr) {
    std::wstring error_info =
        std::format(L"Create event error code: {}", ::GetLastError());
    OutputDebugStringW(error_info.c_str());
    return;
  }
  DWORD result = 0;
  if (WAIT_OBJECT_0 == (result = WaitForSingleObject(hRcvEvent, 5 * 1000))) {
    memset(&header, 0, sizeof(header));
    memmove(&header, (PBYTE)pBuffer, sizeof(header));
    pPayload = (PBYTE)pBuffer + sizeof(header);

    callback(pPayload, header.size);
    CloseHandle(hRcvEvent);
  } else {
    std::string error_info =
        std::format(" WaitForSingleObjecterror code: {}", ::GetLastError());
    OutputDebugStringA(error_info.c_str());
  }
}
