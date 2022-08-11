#include "gitwndwrap.h"

#include <QtGui/QWindow>

#include "debughelper.h"

GitWndWrap::GitWndWrap(HWND gitHwnd) : mGitWnd(gitHwnd) {
  setStyle();

  // notice: 这里不允许在非GUI线程中创建widget
  // QWindow *window = QWindow::fromWinId((WId)mGitWnd);
  // mWidget = QWidget::createWindowContainer(window, nullptr);
}

GitWndWrap::GitWndWrap(HWND gitHwnd, const QString &title)
    : mGitWnd(gitHwnd), _title(title) {
  setStyle();
}

GitWndWrap::~GitWndWrap() { Close(); }

GitWndWrap::GitWndWrap(GitWndWrap &&rhs) {
  copyValue(rhs);

  rhs.mGitWnd = 0;
  rhs.mWidget = nullptr;
}

GitWndWrap &GitWndWrap::operator=(GitWndWrap &&rhs) {
  if (this == &rhs) {
    return *this;
  }

  copyValue(rhs);

  rhs.mGitWnd = 0;
  rhs.mWidget = nullptr;

  return *this;
}

void GitWndWrap::ShowWindow(bool isShow) {
  ::ShowWindow(mGitWnd, isShow ? SW_SHOW : SW_HIDE);
}

void GitWndWrap::SetFocus() { ::SetFocus(mGitWnd); }

void GitWndWrap::Close() {
  if (0 != mGitWnd) {
    ::PostMessage(mGitWnd, WM_CLOSE, 0, 0);
  }
}

void GitWndWrap::SetParent(QWidget *parent) {
  ::SetParent(mGitWnd, (HWND)parent->winId());
}

HWND GitWndWrap::GetGitWnd() const { return mGitWnd; }

QWidget *GitWndWrap::GetSmartWidget() const { return mWidget; }

wstring GitWndWrap::GetWndText() const {
  wchar_t buff[1024] = {0};
  GetWindowText(mGitWnd, buff, 1024);

  return wstring(buff);
}

void GitWndWrap::InitWidget() {
  QWindow *window = QWindow::fromWinId((WId)mGitWnd);
  mWidget = QWidget::createWindowContainer(window, nullptr);
  static int i = 0;
  mWidget->setObjectName(QString::number(i++));
}

const QString &GitWndWrap::GetTitle() const
{
    return _title;
}

void GitWndWrap::copyValue(const GitWndWrap &rhs) {
  this->mGitWnd = rhs.mGitWnd;
  this->mWidget = rhs.mWidget;
}

void GitWndWrap::setStyle() {
  LONG styleValue = ::GetWindowLong(mGitWnd, GWL_STYLE);
  styleValue &= ~WS_CAPTION;
  styleValue &= ~WS_VSCROLL;
  styleValue &= ~WS_THICKFRAME;
  styleValue &= ~WS_POPUP;
  styleValue &= WS_CHILD;
  ::SetWindowLong(mGitWnd, GWL_STYLE, styleValue);
  ::SetWindowLong(mGitWnd, GWL_EXSTYLE,
                  ::GetWindowLong(mGitWnd, GWL_STYLE) | ~WS_EX_TOOLWINDOW);
}
