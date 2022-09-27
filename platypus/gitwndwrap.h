#ifndef gitwindget_h
#define gitwindget_h

#include <Windows.h>

#include <QtCore/QObject>
#include <QtWidgets/QWidget>
#include <memory>

using std::shared_ptr;
using std::wstring;

class GitWndWrap : public QObject{
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(GitWndWrap)

 public:
  explicit GitWndWrap();
  explicit GitWndWrap(HWND gitHwnd);
  explicit GitWndWrap(HWND gitHwnd, const QString &title);
  virtual ~GitWndWrap();

  void ShowWindow(bool isShow);
  void SetFocus();
  void Close();
  HWND GetGitWnd() const;
  QWidget *GetSmartWidget() const;
  void InitWidget();
  const QString &GetTitle() const;

 private:
  void setStyle();

 private:
  HWND git_wnd_ = 0;
  QString title_ = "";
  QWidget *widget_ = nullptr;
};

#endif  // gitwindget_h