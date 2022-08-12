#ifndef gitwndhelper_h
#define gitwndhelper_h

#include <Windows.h>

#include <QtCore/QObject>
#include <thread>
#include <vector>

#include "gitwndwrap.h"
#include "singleton.h"

using GitWndsMap = std::map<HWND, GitWndWrap*>;
using GitWndMapItr = GitWndsMap::iterator;
using ConstGitWndMapItr = GitWndsMap::const_iterator;
using GitWndPair = std::pair<const HWND, GitWndWrap*>;

#define GitWndHelperInstance SINGLETON_INSTANCE(GitWndHelper)
#define GitWndHelperFinalize SINGLETON_FINALIZE(GitWndHelper)

class NotifyHelper : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NotifyHelper);

 public:
  NotifyHelper() {}
  virtual ~NotifyHelper() {}

  void NotifyNewHandle(HWND git_wnd) { emit onNewHandle(git_wnd); }

  void Connect(QObject *reciver, const char *OnAddWnd) {
    // 注意最后一个参数，不能在和接受者同一个线程中调用，否则会造成死锁
    connect(this, SIGNAL(onNewHandle(HWND)), reciver, SLOT(OnAddWnd(HWND)),
            Qt::BlockingQueuedConnection);
  }

 signals:
  void onNewHandle(HWND);
};

class GitWndHelper : SINGLETON_INHERIT(GitWndHelper) {
 public:
  ~GitWndHelper();

  bool Add(HWND hwnd, const QString &title);
  void Delete(QWidget *widget);
  void Clear();
  void CloseAllWindows();
  void SetFocus(QWidget *widget);
  void ShowWindow(QWidget *widget);
  void ConnectNotify(QObject *obj, const char *funName);
  bool InitGitWidget(HWND git_wnd, QWidget *parent, QString &title,
                     QWidget **widget);
  QWidget *GetWidget(HWND git_wnd);

 private:
  void init();
  bool find(HWND git_hwnd, GitWndWrap **git_wrap);
  GitWndMapItr find(QWidget *widget);

 private:
  GitWndsMap git_wnds_maps_;
  NotifyHelper mNotifyHelper;

  DECLARE_FRIEND_SINGLETON(GitWndHelper);
  DECLARE_PRIVATE_CONSTRUCTOR(GitWndHelper, init);

};

#endif  // gitwndhelper_h