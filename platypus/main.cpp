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

#define EXE_NAME "WinExec_x64.exe"

PipeServer *g_server = nullptr;
void StartServer(Platypus *mainwindow) {
  g_server = new PipeServer();
  g_server->Start(pipe_name);
  g_server->BindFunc(
      std::bind(&Platypus::ReceiveMsg, mainwindow, std::placeholders::_1));
  g_server->Run();
}

void StartWinExec() {
  QString path = qApp->applicationDirPath() + QString("/") + EXE_NAME;
  if (!Common::StartProcess(path, "", SW_SHOW)) {
    OutDebug("error. create exe error.");
  }
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
