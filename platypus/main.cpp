#include <QDir>
#include <QMessageBox>
#include <QObject>
#include <QtWidgets/QApplication>

#include "Server.h"
#include "common.h"
#include "debughelper.h"
#include "gitwndhelper.h"
#include "hook_keyboard.h"
#include "include/const.h"
#include "pipe_server.h"
#include "platypus.h"
#include "single_process.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#define EXE_NAME_X64 "git_register_exec_x64.exe"
#define EXE_NAME "git_register_exec.exe"

namespace spd = spdlog;

PipeServer *g_server = nullptr;
void StartServer(Platypus *mainwindow) {
  g_server = new PipeServer();
  g_server->Start(pipe_name);
  g_server->BindFunc(
      std::bind(&Platypus::ReceiveMsg, mainwindow, std::placeholders::_1));
  g_server->Run();
}

void StartGitRegisterExec() {
  QString path = qApp->applicationDirPath() + QString("/") + EXE_NAME_X64;
  spdlog::get(LOG_NAME)->info("start winexec process:{}", path.toStdString());
  if (!Common::StartProcess(path, "", SW_HIDE)) {
    OutDebug("error. create exe error.");
    spdlog::get(LOG_NAME)->error("start winexec process error:{}",
                                 path.toStdString());
  }
}

bool InitLog() {
  const QString log_dir_name = "log";
  QDir log_dir;
  if (!log_dir.exists(log_dir_name) && !log_dir.mkdir(log_dir_name)) {
    QMessageBox::information(nullptr, "title", "create log dir error",
                             QMessageBox::StandardButton::Ok);
    return false;
  }
  spdlog::set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid %t] %v");
  spdlog::set_level(spdlog::level::info);

  spdlog::flush_every(std::chrono::seconds(1));
  const std::tm &loc_tm = spdlog::details::os::localtime();
  std::string log_file_name =
      std::format("./{}/{}-{}-{}-{}-{}-{}-{}.txt", log_dir_name.toStdString(),
                  LOG_NAME, loc_tm.tm_year + 1900, loc_tm.tm_mon + 1,
                  loc_tm.tm_mday, loc_tm.tm_hour, loc_tm.tm_min, loc_tm.tm_sec);
  auto rotating_logger = spdlog::create<spdlog::sinks::basic_file_sink_mt>(
      LOG_NAME, log_file_name, false);
  rotating_logger->flush_on(spd::level::err);

  return true;
}

int main(int argc, char *argv[]) {
  InitLog();
  SingleProcess single_process(L"xyx_@_platypus_2022");
  if (single_process.isExist()) {
    QMessageBox::information(nullptr, QObject::tr("title"),
                             QObject::tr("already start same process"),
                             QMessageBox::StandardButton::Ok);
    spdlog::get(LOG_NAME)->error("already start same process");
    return -1;
  }

  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

  QApplication app(argc, argv);
  // 需要放在这里否则注册的时候会失败，错误码：0x5
  MyHook::Instance().start();

  Platypus mainwindow;
  mainwindow.show();
  std::thread g_ServerThread = std::thread(StartServer, &mainwindow);
  StartGitRegisterExec();

  MyHook::Instance().setNotifyCallBack(std::bind(
      &Platypus::ReceiveShortcut, &mainwindow, std::placeholders::_1));

  GitWndHelperInstance;
  GitWndHelperInstance.ConnectNotify(&mainwindow, SLOT(OnAddWnd(HWND)));
  int ret_code = app.exec();
  GitWndHelperFinalize;

  g_server->Stop();
  g_ServerThread.join();

  return ret_code;
}
