#ifndef gitwindget_h
#define gitwindget_h

#include <Windows.h>

#include <QtCore/QObject>
#include <QtWidgets/QWidget>
#include <memory>

#include "noncopyable.h"

using std::shared_ptr;
using std::wstring;

class GitWndWrap : public QObject, public XIBAO::NonCopyable {
  Q_OBJECT

 public:
  explicit GitWndWrap();
  explicit GitWndWrap(HWND gitHwnd);
  explicit GitWndWrap(HWND gitHwnd, const QString &title);
  ~GitWndWrap();
  // 右值拷贝
  GitWndWrap(GitWndWrap &&rhs);
  GitWndWrap &operator=(GitWndWrap &&rhs);

  void ShowWindow(bool isShow);
  void SetFocus();
  void Close();
  void SetParent(QWidget *parent);
  HWND GetGitWnd() const;
  QWidget *GetSmartWidget() const;
  void InitWidget();
  const QString &GetTitle() const;

 private:
  void copyValue(const GitWndWrap &rhs);
  void setStyle();

 private:
  HWND git_wnd_ = 0;
  QString title_ = "";
  QWidget *widget_ = nullptr;
};

#endif  // gitwindget_h