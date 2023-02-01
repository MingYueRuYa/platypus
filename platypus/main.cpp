#include <QDir>
#include <QMessageBox>
#include <QObject>
#include <QtWidgets/QApplication>
#include <format>

#include "Server.h"
#include "common.h"
#include "debughelper.h"
#include "gitwndhelper.h"
#include "hook_keyboard.h"
#include "include/const.h"
#include "pipe_server.h"
#include "platypus.h"
#include "single_process.h"
#include "feature.h"
#include "global_config.h"

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
  QString path = qApp->applicationDirPath() + QString("/") + GIT_REGISTER_EXE_X64;
    spdlog::logInstance().info(L"start winexec process:{}", path.toStdWString());
  if (!Common::StartProcess(path, "", SW_HIDE)) {
    spdlog::logInstance().error(L"start winexec process error:{}",
                                 path.toStdWString());
  }
}

int main(int argc, char *argv[]) {

  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

  QApplication app(argc, argv);
#if !DEBUG_MODE
  SingleProcess single_process(L"xyx_@_platypus_2022");
  if (single_process.isExist()) {
    QMessageBox::information(nullptr, QObject::tr("title"),
                             QObject::tr("already start same process"),
                             QMessageBox::StandardButton::Ok);
    return -1;
  }
#endif
  const std::tm &loc_tm = spdlog::details::os::localtime();
  std::string log_file_name =
      std::format("logs/{}-{}-{}-{}-{}-{}-{}.txt", LOG_NAME,
                  loc_tm.tm_year + 1900, loc_tm.tm_mon + 1, loc_tm.tm_mday,
                  loc_tm.tm_hour, loc_tm.tm_min, loc_tm.tm_sec);
  spdlog::InitLog(log_file_name, LOG_NAME);

  // It needs to be place here, otherwise the registration will fail
  // errro code: 0x5
  MyHook::Instance().start(true);

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
