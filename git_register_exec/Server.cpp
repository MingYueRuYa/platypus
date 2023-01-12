#include "stdafx.h"
#include "Server.h"

#include "../include/head.h"

Server::Server() : m_hMap(NULL), m_mapSize(0) {}

void Server::createShm(const std::string& mapName, size_t size) {
  //���� CreateFileMapping ����һ���ڴ��ļ�ӳ�����
  m_hMap =
      ::CreateFileMappingA(INVALID_HANDLE_VALUE,  // handle to file to map
                           NULL,            // optional security attributes
                           PAGE_READWRITE,  // protection for mapping object
                           0,               // high-order 32 bits of object size
                           size,            // low-order 32 bits of object size
                           mapName.c_str());  // name of file-mapping object

  if (m_hMap == nullptr)  //����ʧ��
  {
    throw;
  }

  m_mapSize = size;
}

void Server::stop() {
  const std::string& eventType = "exit";
  PVOID pdata = "exit";
  UINT64 dataSize = strlen((char*)pdata) + 1;
  Header header;
  LPVOID pBuffer = ::MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  memset(pBuffer, 0, m_mapSize);

  //��������ͷ
  strncpy_s(header.type, eventType.length() + 1, eventType.c_str(), 20);
  header.size = dataSize;

  //�����ݸ��Ƶ������ڴ�(����ͷ+��Ч����)
  memmove((PBYTE)pBuffer, &header, sizeof(header));

  PBYTE pPayload = (PBYTE)pBuffer + sizeof(header);
  memmove(pPayload, pdata, dataSize);

  HANDLE hSendEvent =
      OpenEventA(EVENT_ALL_ACCESS, NULL, m_rcvEventName.c_str());
  if (hSendEvent == nullptr) {
    return;
  }
  SetEvent(hSendEvent);
}

void Server::appendRcvEvent(const std::string& eventType, const _FUNC& func) {
  m_handlers.emplace(eventType, func);
}

void Server::eventLoop(const std::string& eventName) {
  m_rcvEventName = eventName + "ctos";

  HANDLE hRcvEvent = CreateEventA(NULL, FALSE, FALSE, m_rcvEventName.c_str());
  if (hRcvEvent == nullptr) {
    return;
  }

  while (true) {
    WaitForSingleObject(hRcvEvent, INFINITE);  //�յ��źţ��Զ�����
    LPVOID pBuffer = ::MapViewOfFile(m_hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    Header header;
    memmove_s(&header, sizeof(header), pBuffer, sizeof(header));

    //��������
    _FUNC func = m_handlers.at(header.type);
    bool is_continue = func((PBYTE)pBuffer + sizeof(header), header.size);

    //���·��
    memset(pBuffer, 0, sizeof(header));
    memmove((PBYTE)pBuffer, &header, sizeof(header));

    std::string sndEventName = eventName + "stoc";
    HANDLE hsndEvent = OpenEventA(EVENT_ALL_ACCESS, NULL, sndEventName.c_str());
    if (hsndEvent == nullptr) {
      break;
    }
    SetEvent(hsndEvent);

    if (!is_continue) {
      return;
    }
  }
}