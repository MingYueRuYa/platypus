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
#include "global_config.h"
#include "helpdialog.h"
#include "hook_keyboard.h"
#include "json.hpp"
#include "process_utils.hpp"
#include "round_shadow_helper.h"
#include "spdlog/spdlog.h"
#include "string_utils.hpp"
#include "tab_bar_draw_helper.h"
#include "ui_platypus.h"
#include "weak_call_back.hpp"
#include "qxtglobalshortcut.h"

const int kLAYOUT_ITEM_WIDTH = 30;

using nlohmann::json;
using std::vector;
using XIBAO::DebugHelper;

using Shortcut = HookShortCut::Shortcut;

Platypus::Platypus(QWidget *parent)
    : QWidget(parent), ui(new Ui::platypusClass) {
  ui->setupUi(this);

  setupUI();
}

Platypus::~Platypus() {
  delete ui;
  delete frame_less_helper_;
  if (0 != win_exe_hwnd_) ::PostMessageA(win_exe_hwnd_, WM_CLOSE, 0, 0);
  exitGitRegExec();
}

void Platypus::ReceiveMsg(const wchar_t *json_str) {
  string json_msg = to_utf8_string(json_str);
  json json_obj = json::parse(json_msg);
  string action = json_obj.value("action", "");
  HWND hwnd = (HWND)json_obj.value("HWND", 0);
  if (action == "exit") {
    qApp->postEvent(this,
                    new CustomEvent((QEvent::Type)CusEventType::GitWndExit,
                                    QString::fromStdString(json_msg)));
  } else if (action == "update") {
    QString title = QString::fromStdString(json_obj.value("title", ""));
    bool result = GitWndHelperInstance.Add(hwnd, title);
    // we need to update title
    if (!result)
      qApp->postEvent(
          this, new CustomEvent((QEvent::Type)CusEventType::GitWndUpdateTitle,
                                QString::fromStdString(json_msg)));
  } else if ("init_hwnd" == action) {
    win_exe_hwnd_ = hwnd;
  } else if ("set_foreground" == action) {
    qApp->postEvent(this,
                    new CustomEvent((QEvent::Type)CusEventType::SetForeground,
                                    QString::fromStdString(json_msg)));
  }
  //   spdlog::get(LOG_NAME)->info("receive message:{}", json_msg);
  spdlog::logInstance().info(L"receive message:{}", to_wide_string(json_msg));
}

void Platypus::ReceiveShortcut(int vkcode) {
  json init_json = {{"shortcut", vkcode}};
  qApp->postEvent(this,
                  new CustomEvent((QEvent::Type)CusEventType::ShortCut,
                                  QString::fromStdString(init_json.dump())));
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
    case WM_SETFOCUS:
      setGitFocus();
      break;
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
      setGitFocus();
      break;
    case (int)CusEventType::ShortCut:
      // getShortcut(custom->GetData());
      break;
    case (int)CusEventType::SetForeground:
      setForeGroundWnd(custom->GetData());
      break;
    default:
      break;
  }
}

void Platypus::setupUI() {
  this->setWindowFlags(Qt::FramelessWindowHint |
                       Qt::X11BypassWindowManagerHint |
                       Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
  frame_less_helper_ = new NcFramelessHelper();
  frame_less_helper_->activateOn(this);
  frame_less_helper_->setFilterEventCallBack(
      std::bind(&TabWidgetProxy::filterTabBarEvent, ui->tabWidgetProxy,
                std::placeholders::_1, std::placeholders::_2));
  frame_less_helper_->activeOnWithChildWidget(this,
                                              ui->tabWidgetProxy->tabBar());
  initSig();
  initShortcut();
  ui->tabWidgetProxy->updateDrawHelp(new TabBarDrawHelper());
  QTimer::singleShot(100, [this]() { this->startGitWnd(); });
}

void Platypus::initSig() {
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
  connect(&(ui->tabWidgetProxy->getSignal()), SIGNAL(tabBarClicked(int)), this,
          SLOT(OnTabBarClicked(int)));
  connect(&(ui->tabWidgetProxy->getSignal()), SIGNAL(helpClicked()), this,
          SLOT(OnHelpClicked()));
  connect(&(ui->tabWidgetProxy->getSignal()),
          SIGNAL(tabBarMouseRelease(QMouseEvent *)), this,
          SLOT(OnTabBarMouseRelease(QMouseEvent *)));
}

void Platypus::initShortcut() {
  QxtGlobalShortcut *shortcut = new QxtGlobalShortcut(this);
  shortcut->setShortcut(QKeySequence("ALT+1"));
  connect(shortcut, &QxtGlobalShortcut::activated,
          [=]() { moveTabWigetIndex(false); });

  shortcut = new QxtGlobalShortcut(this);
  shortcut->setShortcut(QKeySequence("ALT+2"));
  connect(shortcut, &QxtGlobalShortcut::activated,
          [=]() { moveTabWigetIndex(true); });

  shortcut = new QxtGlobalShortcut(this);
  shortcut->setShortcut(QKeySequence("SHIFT+CTRL+A"));
  connect(shortcut, &QxtGlobalShortcut::activated, [=]() { startGitWnd(); });

  shortcut = new QxtGlobalShortcut(this);
  shortcut->setShortcut(QKeySequence("SHIFT+CTRL+W"));
  connect(shortcut, &QxtGlobalShortcut::activated, [=]() {
    OnCloseTab(ui->tabWidgetProxy->tabWidget()->currentIndex());
  });

  shortcut = new QxtGlobalShortcut(this);
  shortcut->setShortcut(QKeySequence("ALT+F11"));
  connect(shortcut, &QxtGlobalShortcut::activated,
          [=]() { ui->tabWidgetProxy->maxOrRestore(); });
}

void Platypus::OnAddWnd(HWND git_wnd) {
  QString title = "";
  QWidget *git_widget = nullptr;
  bool result =
      GitWndHelperInstance.InitGitWidget(git_wnd, this, title, &git_widget);
  if (result) {
    ui->tabWidgetProxy->addTab2(git_widget, title);
    spdlog::logInstance().info(L"{}", L"Create git widget finished.");
  } else
    spdlog::logInstance().error(L"{}", L"add new git wnd error.");
  setGitFocus();
}

void Platypus::OnAddNewTab() { startGitWnd(); }

void Platypus::startGitWnd() {
  // 1.get git indestall dir
  static QString git_dir = Common::GetInstallGitPath();

  if (git_dir.isEmpty()) {
    spdlog::logInstance().error("Not find git install path");
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

  if (!Common::StartProcess(mintty_full_path, QString::fromStdWString(args),
                            SW_HIDE)) {
    spdlog::logInstance().error("Create git Process failed");
  } else {
    spdlog::logInstance().info("Create git Process finished");
  }
}

void Platypus::setGitFocus() {
  QWidget *current_widget = ui->tabWidgetProxy->tabWidget()->currentWidget();
  if (nullptr == current_widget) {
    return;
  }
  GitWndHelperInstance.SetFocus(current_widget);
}

void Platypus::exitWnd(const QString &data) {
  auto exit_json = json::parse(data.toStdString());
  HWND git_hwnd = (HWND)exit_json.value("HWND", 0);
  QWidget *widget = GitWndHelperInstance.GetWidget(git_hwnd);
  if (nullptr == widget) {
    spdlog::logInstance().error("Not find widget");
    return;
  }
  int index = ui->tabWidgetProxy->tabWidget()->indexOf(widget);
  if (-1 == index) return;
  ui->tabWidgetProxy->tabWidget()->removeTab2(index);
  ui->tabWidgetProxy->tabWidget()->setNextCurrentIndex(index);
  QTimer::singleShot(100, this, [this] { setGitFocus(); });
  GitWndHelperInstance.Delete(widget);
  spdlog::logInstance().info("receive:git window exited message");
}

void Platypus::updateTitle(const QString &data) {
  string str_json_data = data.toStdString();
  auto json_obj = json::parse(str_json_data);
  HWND git_hwnd = (HWND)json_obj.value("HWND", 0);
  string title = json_obj.value("title", "");
  QWidget *widget = GitWndHelperInstance.GetWidget(git_hwnd);
  int index = ui->tabWidgetProxy->tabWidget()->indexOf(widget);
  if (-1 == index) return;
  ui->tabWidgetProxy->tabWidget()->setTabText(index,
                                              QString::fromStdString(title));
  ui->tabWidgetProxy->tabWidget()->setTabToolTip(index,
                                                 QString::fromStdString(title));
  spdlog::logInstance().info(str_json_data);
}

void Platypus::getShortcut(const QString &data) {
  string str_json_data = data.toStdString();
  auto json_obj = json::parse(str_json_data);
  Shortcut vkcode = (Shortcut)json_obj.value("shortcut", 0);
  if (Shortcut::Unknow == vkcode || !acceptShortcut((int)vkcode)) return;
  switch (vkcode) {
    case Shortcut::TAB_CTRL:
      moveTabWigetIndex(false);
      break;
    case Shortcut::TAB_CTRL_SHIFT:
      moveTabWigetIndex(true);
      break;
    case Shortcut::CTRL_SHIFT_A:
      startGitWnd();
      break;
    case Shortcut::CTRL_SHIFT_W:
      OnCloseTab(ui->tabWidgetProxy->tabWidget()->currentIndex());
      break;
    case Shortcut::ALT_F11:
      ui->tabWidgetProxy->maxOrRestore();
      break;
    default:
      break;
  }
}

void Platypus::moveTabWigetIndex(bool forward) {
  int cur_index = ui->tabWidgetProxy->tabWidget()->currentIndex();
  int max_count = ui->tabWidgetProxy->tabWidget()->count() - 1;
  if (forward) {
    cur_index -= 1;
    if (cur_index < 0) cur_index = max_count - 1;
  } else {
    cur_index += 1;
    if (cur_index >= max_count) cur_index = 0;
  }
  ui->tabWidgetProxy->tabWidget()->setCurrentIndex(cur_index);
  setGitFocus();
}

bool Platypus::acceptShortcut(int vkcode) const {
  if (!this->isVisible()) return false;
  HWND hwnd = ::GetFocus();
  if (nullptr != GitWndHelperInstance.GetWidget(::GetFocus()) ||
      (HWND)this->winId() == hwnd)
    return true;
  return false;
}

void Platypus::setForeGroundWnd(const QString &data) {
  auto exit_json = json::parse(data.toStdString());
  HWND git_hwnd = (HWND)exit_json.value("HWND", 0);
  GitWndHelperInstance.SetForegroundWnd(git_hwnd);
}

void Platypus::exitGitRegExec() {
  while (1) {
    if (!ProcessUtils::FindProcess(GIT_REGISTER_EXE_X64_W)) {
      break;
    }
    ProcessUtils::KillProcess(GIT_REGISTER_EXE_X64_W, true);
    spdlog::logInstance().info(L"kill process, {}", GIT_REGISTER_EXE_X64_W);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
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
    GitWndHelperInstance.Delete(widget);
  }
  QTimer::singleShot(100, this, [this] { setGitFocus(); });
  ui->tabWidgetProxy->tabWidget()->setNextCurrentIndex(index);
  spdlog::logInstance().info("Close git window");
}

void Platypus::OnAddBtnClicked() { startGitWnd(); }

void Platypus::OnCloseWnd() { this->close(); }

void Platypus::OnMinWnd() {}

void Platypus::OnMaxWnd() { setGitFocus(); }

void Platypus::OnRestoreWnd() { setGitFocus(); }

void Platypus::OnMaxOrRestore() { setGitFocus(); }

void Platypus::OnTabBarClicked(int index) {
  QWidget *widget = ui->tabWidgetProxy->tabWidget()->widget(index);
  if (nullptr == widget) return;
  GitWndHelperInstance.SetFocus(widget);
}

void Platypus::OnHelpClicked() {
  HelpDialog dialog(this);
  dialog.exec();
}

void Platypus::OnTabBarMouseRelease(QMouseEvent *) { setGitFocus(); }
