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
#include "custom_event.h"
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

#define GIT_WND_EXIT

Platypus::Platypus(QWidget *parent)
    : QWidget(parent), ui(new Ui::platypusClass) {
  ui->setupUi(this);

  setupUI();
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
  if (action == "exit") {
    qApp->postEvent(this,
                    new CustomEvent((QEvent::Type)CusEventType::GitWndExit,
                                    QString::fromStdString(json_msg)));
  } else if (action == "update") {
    QString title = QString::fromStdString(json_obj.value("title", ""));
    bool result = GitWndHelperInstance.Put(git_hwnd, title);
    // we need to update title
    if (!result)
      qApp->postEvent(
          this, new CustomEvent((QEvent::Type)CusEventType::GitWndUpdateTitle,
                                QString::fromStdString(json_msg)));
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

void Platypus::customEvent(QEvent *event) {
  CustomEvent *custom = dynamic_cast<CustomEvent *>(event);
  switch (custom->type()) {
    case (int)CusEventType::GitWndExit:
      exitWnd(custom->GetData());
      break;
    case (int)CusEventType::GitWndUpdateTitle:
      updateTitle(custom->GetData());
      break;
    default:
      break;
  }
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

  initSig();
}

void Platypus::initSig() {
}

void Platypus::OnAddWnd() {
  GitWindowsWrap &git_wnd_wraps = GitWndHelperInstance.GetWindowsWrap();

  do {
    if (git_wnd_wraps.empty()) {
      break;
    }

    auto git_wraps = git_wnd_wraps.rbegin();
    // Qt controls must be initialized in the GUI thread
    git_wraps->InitWidget();
    ui->tabWidgetProxy->addTab2(git_wraps->GetSmartWidget(),
                                git_wraps->GetTitle());
    git_wraps->SetParent(this);
  } while (0);
}

void Platypus::OnAddNewTab() { startGitWnd(); }

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

void Platypus::exitWnd(const QString &data) {
  auto exit_json = json::parse(data.toStdString());
  HWND git_hwnd = (HWND)exit_json.value("HWND", 0);
  const GitWndWrap &hwndwrap = GitWndHelperInstance.Get(git_hwnd);
  QWidget *widget = hwndwrap.GetSmartWidget();
  int index = ui->tabWidgetProxy->tabWidget()->indexOf(widget);
  if (-1 == index) return;
  ui->tabWidgetProxy->tabWidget()->removeTab(index);
  GitWndHelperInstance.Close(widget);
}

void Platypus::updateTitle(const QString &data) {
  string str_json_data = data.toStdString();
  auto json_obj = json::parse(str_json_data);
  HWND git_hwnd = (HWND)json_obj.value("HWND", 0);
  string title = json_obj.value("title", "");
  const GitWndWrap &hwndwrap = GitWndHelperInstance.Get(git_hwnd);
  QWidget *widget = hwndwrap.GetSmartWidget();
  int index = ui->tabWidgetProxy->tabWidget()->indexOf(widget);
  if (-1 == index) return;
  ui->tabWidgetProxy->tabWidget()->setTabText(index, QString::fromStdString(title));
  ui->tabWidgetProxy->tabWidget()->setTabToolTip(index, QString::fromStdString(title));
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
