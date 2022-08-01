#include <QtWidgets/QApplication>

#include "Server.h"
#include "gitwndhelper.h"
#include "include/const.h"
#include "platypus.h"

Server *g_Server = nullptr;

void StartServer(Platypus *mainwindow)
{
  Server *g_Server = new Server();
  g_Server->createShm(platypus_shm_name, MAX_SHM_SIZE);
  g_Server->appendRcvEvent(
      function_name, std::bind(&Platypus::FindWndTitle, mainwindow,
                               std::placeholders::_1, std::placeholders::_2));
  g_Server->appendRcvEvent(
      wnd_exit_name, std::bind(&Platypus::WndExit, mainwindow,
                               std::placeholders::_1, std::placeholders::_2));
  g_Server->appendRcvEvent(
      exit_name, std::bind(&Platypus::Stop, mainwindow,
                           std::placeholders::_1, std::placeholders::_2));
  g_Server->eventLoop(platypus_evt_name);
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  Platypus mainwindow;
  mainwindow.show();
  std::thread g_ServerThread = std::thread(StartServer, &mainwindow);

  GitWndHelperInstance;
  GitWndHelperInstance.ConnectNotify(&mainwindow, "OnAddWnd");
  int ret_code = app.exec();
  GitWndHelperFinalize;

  g_Server->stop();
  g_ServerThread.join();

  return ret_code;
}
