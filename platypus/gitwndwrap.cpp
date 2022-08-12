#include "gitwndwrap.h"

#include <QtGui/QWindow>

#include "debughelper.h"

GitWndWrap::GitWndWrap() {}

GitWndWrap::GitWndWrap(HWND gitHwnd) : git_wnd_(gitHwnd) {
  setStyle();

  // notice: 这里不允许在非GUI线程中创建widget
  // QWindow *window = QWindow::fromWinId((WId)git_wnd_);
  // widget_ = QWidget::createWindowContainer(window, nullptr);
}

GitWndWrap::GitWndWrap(HWND gitHwnd, const QString &title)
    : git_wnd_(gitHwnd), title_(title) {
  setStyle();
}

GitWndWrap::~GitWndWrap() { Close(); }

void GitWndWrap::ShowWindow(bool isShow) {
  ::ShowWindow(git_wnd_, isShow ? SW_SHOW : SW_HIDE);
}

void GitWndWrap::SetFocus() { ::SetFocus(git_wnd_); }

void GitWndWrap::Close() {
  if (0 != git_wnd_) {
    ::PostMessage(git_wnd_, WM_CLOSE, 0, 0);
  }
  if (nullptr != widget_) widget_->deleteLater();
}

void GitWndWrap::SetParent(QWidget *parent) {
  ::SetParent(git_wnd_, (HWND)parent->winId());
}

HWND GitWndWrap::GetGitWnd() const { return git_wnd_; }

QWidget *GitWndWrap::GetSmartWidget() const { return widget_; }

void GitWndWrap::InitWidget() {
  QWindow *window = QWindow::fromWinId((WId)git_wnd_);
  if (nullptr == window) {
    OutDebug("error.init widget error window is nullptr.");
    return;
  }
  widget_ = QWidget::createWindowContainer(window, nullptr);
  static int i = 0;
  widget_->setObjectName(QString::number(i++));
}

const QString &GitWndWrap::GetTitle() const { return title_; }

void GitWndWrap::setStyle() {
  LONG styleValue = ::GetWindowLong(git_wnd_, GWL_STYLE);
  styleValue &= ~WS_CAPTION;
  styleValue &= ~WS_VSCROLL;
  styleValue &= ~WS_THICKFRAME;
  styleValue &= ~WS_POPUP;
  styleValue &= WS_CHILD;
  ::SetWindowLong(git_wnd_, GWL_STYLE, styleValue);
  ::SetWindowLong(git_wnd_, GWL_EXSTYLE,
                  ::GetWindowLong(git_wnd_, GWL_STYLE) | ~WS_EX_TOOLWINDOW);
}
