#include "platypus.h"

#include <QPushButton>
#include <QtCore/QEvent>
#include <QtCore/QProcess>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtGui/QWindow>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QWidget>
#include <algorithm>
#include <functional>
#include <vector>

#include "NcFrameLessHelper.h"
#include "common.h"
#include "debughelper.h"
#include "draw_helper.h"
#include "gitwndhelper.h"
#include "json.hpp"
#include "round_shadow_helper.h"
#include "string_utils.hpp"
#include "tab_bar_draw_helper.h"
#include "ui_platypus.h"
#include "weak_call_back.hpp"

const int kLAYOUT_ITEM_WIDTH = 30;

using nlohmann::json;
using std::vector;
using XIBAO::DebugHelper;

Platypus::Platypus(QWidget *parent)
    : QWidget(parent), ui(new Ui::platypusClass) {
  ui->setupUi(this);

  setupUI();

  // 打开的时候新建一个git window
  // QTimer::singleShot(200, this, SLOT(OnAddNewTab()));
}

Platypus::~Platypus() {
  delete ui;
  delete frame_less_helper_;
}

bool Platypus::FindWndTitle(unsigned char *pPayload, unsigned long long &size) {
  OutputDebugStringW((wchar_t *)(pPayload));
  return true;
}
bool Platypus::WndExit(unsigned char *pPayload, unsigned long long &size) {
  return true;
}

bool Platypus::Stop(unsigned char *pPayload, unsigned long long &size) {
  return true;
}

void Platypus::ReceiveMsg(const wchar_t *json_str) {
  string json_msg = to_utf8_string(json_str);
  json json_obj = json::parse(json_msg);
  string action = json_obj.value("action", "");
  HWND git_hwnd = (HWND)json_obj.value("HWND", 0);
  OutDebug(action);
  if (action == "exit") {
    // 3. git wnd exit
  } else if (action == "update") {
    // 1. add git wnd
    // 2. update title
  }
}

void Platypus::mouseReleaseEvent(QMouseEvent *e) {
  if (e->button() == Qt::LeftButton) {
    setGitFocus();
  }

  return QWidget::mouseReleaseEvent(e);
}

void Platypus::resizeEvent(QResizeEvent *event) { QWidget::resizeEvent(event); }

void Platypus::showEvent(QShowEvent *event) { setGitFocus(); }

bool Platypus::nativeEvent(const QByteArray &eventType, void *message,
                           long *result) {
  MSG *msg = (MSG *)message;
  switch (msg->message) {
    case WM_SETFOCUS: {
      ReleaseCapture();
      setGitFocus();
      return false;
    }
    default:
      break;
  }

  return QWidget::nativeEvent(eventType, message, result);
}

void Platypus::setupUI() {
  this->setWindowFlags(Qt::FramelessWindowHint |
                       Qt::X11BypassWindowManagerHint);
  frame_less_helper_ = new NcFramelessHelper();
  frame_less_helper_->activateOn(this);
  frame_less_helper_->setFilterEventCallBack(
      std::bind(&TabWidgetProxy::filterTabBarEvent, ui->tabWidgetProxy,
                std::placeholders::_1, std::placeholders::_2));
  frame_less_helper_->activeOnWithChildWidget(this,
                                              ui->tabWidgetProxy->tabBar());

  connect(&(ui->tabWidgetProxy->getSignal()), SIGNAL(tabInserted(int)), this,
          SLOT(OnTabInserted(int)));
  connect(&(ui->tabWidgetProxy->getSignal()), SIGNAL(addBtnClicked()), this,
          SLOT(OnAddBtnClicked()));
  connect(&(ui->tabWidgetProxy->getSignal()), SIGNAL(tabClosed(int)), this,
          SLOT(OnCloseTab(int)));
  connect(&(ui->tabWidgetProxy->getSignal()), SIGNAL(closeWnd()), this,
          SLOT(OnCloseWnd()));
  connect(&(ui->tabWidgetProxy->getSignal()), SIGNAL(minWnd()), this,
          SLOT(OnMinWnd()));
  connect(&(ui->tabWidgetProxy->getSignal()), SIGNAL(maxWnd()), this,
          SLOT(OnMaxWnd()));
  connect(&(ui->tabWidgetProxy->getSignal()), SIGNAL(restoreWnd()), this,
          SLOT(OnRestoreWnd()));
  connect(&(ui->tabWidgetProxy->getSignal()), SIGNAL(tabBarDoubleClicked()),
          this, SLOT(OnMaxOrRestore()));

  ui->tabWidgetProxy->updateDrawHelp(new TabBarDrawHelper());
}

void Platypus::OnAddWnd() {
  GitWindowsWrap &git_wnd_wraps = GitWndHelperInstance.GetWindowsWrap();

  do {
    if (git_wnd_wraps.empty()) {
      break;
    }

    auto git_wraps = git_wnd_wraps.rbegin();
    // Qt的控件必须在GUI线程中初始化
    git_wraps->InitWidget();
    ui->tabWidgetProxy->addTab2(git_wraps->GetSmartWidget(),
                                tr("this is first tab"));
    git_wraps->SetParent(this);
  } while (0);
}

void Platypus::OnAddNewTab() { startGitWnd(); }

// void Platypus::setStyle() {
//   QFile file(SystemScaleInstance.IsScale() ? k2KSTYLE.arg("style.qss")
//                                            : k1KSTYLE.arg("style.qss"));
//   if (file.open(QFile::ReadOnly)) {
//     QString styleSheet = QLatin1String(file.readAll());
//     // 1.4为圆角的最小值
//     styleSheet = styleSheet.arg(4 * SystemScaleInstance.GetScalePrecision());
//     qApp->setStyleSheet(styleSheet);
//   }
//   file.close();
// }

void Platypus::startGitWnd() {
  // 1.获取git的安装目录
  static QString git_dir = Common::GetInstallGitPath();

  if (git_dir.isEmpty()) {
    QMessageBox::critical(this, tr("Error"), tr("Not find git install path"),
                          QMessageBox::Close);
    return;
  }

  QString mintty_full_path = git_dir + R"(\usr\bin\mintty.exe)";
  wstring git_path = QString(R"(%1\git-bash.exe)").arg(git_dir).toStdWString();
  wchar_t args[255 * 4] = {0};
  swprintf_s(args, 255 * 2,
             L"--nodaemon -o AppID=GitForWindows.Bash "
             L"-o AppLaunchCmd=\"%s\" "
             L"-o AppName=\"Git Bash\" -i \"%s\" "
             L"--store-taskbar-properties -- /usr/bin/bash --login -i",
             git_path.c_str(), git_path.c_str());
  STARTUPINFO StartInfo;
  ZeroMemory(&StartInfo, sizeof(StartInfo));
  StartInfo.wShowWindow = SW_HIDE;
  StartInfo.dwFlags = STARTF_USESHOWWINDOW;
  PROCESS_INFORMATION pi;
  ZeroMemory(&pi, sizeof(pi));
  BOOL result =
      CreateProcess(mintty_full_path.toStdWString().c_str(), args, NULL, NULL,
                    false, 0, NULL, NULL, &StartInfo, &pi);
  if (!result) {
    OutDebug("Create Process failed\n");
    return;
  }
  ::CloseHandle(pi.hProcess);
  ::CloseHandle(pi.hThread);
}

void Platypus::setGitFocus() {
  QWidget *current_widget = ui->tabWidgetProxy->tabWidget()->currentWidget();
  if (nullptr == current_widget) {
    return;
  }
  current_widget->show();
  // current_widget->setFocus();
  // GitWndHelperInstance.ShowWindow(current_widget);
  GitWndHelperInstance.SetFocus(current_widget);
}

void Platypus::OnTabInserted(int index) {
  QPushButton *button = new QPushButton();
  button->setStyleSheet(
      "QPushButton{border-image: "
      "url(:/1kplatypus/image/default_100_precent/tab_close.png)}"
      "QPushButton:hover{border-image: "
      "url(:/1kplatypus/image/default_100_precent/tab_close_hover.png)}");
  ui->tabWidgetProxy->setTabButton2(index, QTabBar::RightSide, button);
  ui->tabWidgetProxy->setTabIcon(
      index, QIcon(":/1kplatypus/image/default_100_precent/git_32x32.png"));
}

void Platypus::OnCloseTab(int index) {
  QWidget *widget = ui->tabWidgetProxy->tabWidget()->widget(index);
  if (nullptr != widget) {
    GitWndHelperInstance.Close(widget);
  }
}

void Platypus::OnAddBtnClicked() { startGitWnd(); }

void Platypus::OnCloseWnd() { this->close(); }

void Platypus::OnMinWnd() {}

void Platypus::OnMaxWnd() { setGitFocus(); }

void Platypus::OnRestoreWnd() { setGitFocus(); }

void Platypus::OnMaxOrRestore() { setGitFocus(); }