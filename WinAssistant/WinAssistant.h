// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� WINASSISTANT_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// WINASSISTANT_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef WINASSISTANT_EXPORTS
#define WINASSISTANT_API __declspec(dllexport)
#else
#define WINASSISTANT_API __declspec(dllimport)
#endif

//1.ע�뺯��
//2.hook
//3.unhook
class WINASSISTANT_API CWinAssistant {
public:
    CWinAssistant(void);
    ~CWinAssistant(void);
    bool Register(DWORD dwThreadId);
    bool Unregister();
};
