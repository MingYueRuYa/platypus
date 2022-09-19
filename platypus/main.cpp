#include <QDir>
#include <QMessageBox>
#include <QtWidgets/QApplication>
#include <QObject>

#include "Server.h"
#include "common.h"
#include "debughelper.h"
#include "gitwndhelper.h"
#include "include/const.h"
#include "pipe_server.h"
#include "platypus.h"
#include "single_process.h"
#include "spdlog/spdlog.h"

#define EXE_NAME_X64 "WinExec_x64.exe"
#define EXE_NAME "WinExec.exe"

namespace spd = spdlog;

PipeServer *g_server = nullptr;
void StartServer(Platypus *mainwindow) {
  g_server = new PipeServer();
  g_server->Start(pipe_name);
  g_server->BindFunc(
      std::bind(&Platypus::ReceiveMsg, mainwindow, std::placeholders::_1));
  g_server->Run();
}

void StartWinExec() {
  QString path = qApp->applicationDirPath() + QString("/") + EXE_NAME_X64;
  if (!Common::StartProcess(path, "", SW_SHOW)) {
    OutDebug("error. create exe error.");
  }
}

bool InitLog()
{
  spdlog::set_pattern("%Y-%m-%d %H:%M:%S [%l] [tid %t] %v");
  spdlog::set_level(spdlog::level::info);

  const std::tm &loc_tm = spdlog::details::os::localtime();

  std::string log_file_name = std::format(
      "{}-{}-{}-{}-{}-{}-{}.txt", LOG_NAME, loc_tm.tm_year + 1900, loc_tm.tm_mon + 1,
      loc_tm.tm_mday, loc_tm.tm_hour, loc_tm.tm_min, loc_tm.tm_sec);
  auto rotating_logger =
      spd::basic_logger_mt(LOG_NAME, log_file_name, false);
  rotating_logger->flush_on(spd::level::err);
  return true;
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  SingleProcess single_process(L"xyx_@_platypus_2022");
  if (single_process.isExist()) {
    QMessageBox::information(nullptr, QObject::tr("title"),
                             QObject::tr("already start same process"),
                             QMessageBox::StandardButton::Ok);
    return -1;
  }

  InitLog();

  Platypus mainwindow;
  mainwindow.show();
  std::thread g_ServerThread = std::thread(StartServer, &mainwindow);
  StartWinExec();
  GitWndHelperInstance;
  GitWndHelperInstance.ConnectNotify(&mainwindow, SLOT(OnAddWnd(HWND)));
  int ret_code = app.exec();
  GitWndHelperFinalize;

  g_server->Stop();
  g_ServerThread.join();

  return ret_code;
}
