#include "stdafx.h"
#include "Client.h"
#include "../include/head.h"

Client::Client() : m_hMap(NULL), m_mapSize(0) {}

void Client::init(const std::string& mapName, size_t size,
                  const std::string& eventName) {
  // ������ͼ��һ���������ڴ�ӳ���ļ�����
  m_hMap = ::OpenFileMappingA(
      FILE_MAP_ALL_ACCESS,
      0,  //������������صľ�����ɵ�ǰ�����������½��̼̳У����������ΪTRUE��
      mapName.c_str());
  if (NULL == m_hMap)  // ��ʧ��
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

  //��������ͷ
  strncpy_s(header.type, eventType.length() + 1, eventType.c_str(), 20);
  header.size = dataSize;

  //�����ݸ��Ƶ������ڴ�(����ͷ+��Ч����)
  memmove((PBYTE)pBuffer, &header, sizeof(header));

  PBYTE pPayload = (PBYTE)pBuffer + sizeof(header);
  memmove(pPayload, pdata, dataSize);

  std::string sndEventName = m_EventName + "ctos";
  HANDLE hSendEvent = OpenEventA(EVENT_ALL_ACCESS, NULL, sndEventName.c_str());
  if (hSendEvent == nullptr) {
    //������
    return;
  }
  SetEvent(hSendEvent);

  //��ʼ�������ݷ����¼�
  std::string rcvEventName = m_EventName + "stoc";
  HANDLE hRcvEvent = CreateEventA(NULL, FALSE, FALSE, rcvEventName.c_str());
  if (hRcvEvent == nullptr) {
    //������
    return;
  }
  std::cout << "�ȴ����ݷ�����..." << std::endl;
  WaitForSingleObject(hRcvEvent, INFINITE);  //�յ��źţ��Զ�����

  //���ݷ����Ժ�
  //���
  memset(&header, 0, sizeof(header));
  memmove(&header, (PBYTE)pBuffer, sizeof(header));
  pPayload = (PBYTE)pBuffer + sizeof(header);

  callback(pPayload, header.size);
  CloseHandle(hRcvEvent);
}
