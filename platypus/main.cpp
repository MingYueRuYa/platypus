#include <QDir>
#include <QMessageBox>
#include <QObject>
#include <QtWidgets/QApplication>
#include <format>

#include "Server.h"
#include "common.h"
#include "debughelper.h"
#include "gitwndhelper.h"
#include "const.h"
#include "pipe_server.h"
#include "platypus.h"
#include "single_process.h"
#include "feature.h"
#include "global_config.h"
#include "client/windows/handler/exception_handler.h"

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

static bool crash_callback(const wchar_t* dump_path, const wchar_t* id,
  void* context, EXCEPTION_POINTERS* exinfo,
  MDRawAssertionInfo* assertion,
  bool succeeded) {
  MessageBoxW(NULL, L"程序已崩溃，需要重启", L"标题", MB_OK);
  return succeeded;
}

static void Dead() {
  int* i = reinterpret_cast<int*>(0xdead);
  *i = 5;  // crash!
}


int main(int argc, char *argv[]) {

  google_breakpad::ExceptionHandler eh(L".",
    NULL,
    crash_callback,
    NULL,
    google_breakpad::ExceptionHandler::HANDLER_ALL);

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

  Platypus mainwindow;
  mainwindow.show();
  std::thread g_ServerThread = std::thread(StartServer, &mainwindow);
  StartGitRegisterExec();

  GitWndHelperInstance;
  GitWndHelperInstance.ConnectNotify(&mainwindow, SLOT(OnAddWnd(HWND)));

  int ret_code = app.exec();
  GitWndHelperFinalize;

  g_server->Stop();
  g_ServerThread.join();

  return ret_code;
}
