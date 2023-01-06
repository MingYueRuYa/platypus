// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 WINASSISTANT_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// WINASSISTANT_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef WINASSISTANT_EXPORTS
#define WINASSISTANT_API __declspec(dllexport)
#else
#define WINASSISTANT_API __declspec(dllimport)
#endif

//1.注入函数
//2.hook
//3.unhook
class WINASSISTANT_API CGitPlugin {
public:
    CGitPlugin(void);
    ~CGitPlugin(void);
    bool Register(HWND targetWnd, DWORD dwThreadId);
    bool Unregister();
    void ReceiveShortcut(int vkcode);
};
