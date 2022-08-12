#ifndef gitwndhelper_h
#define gitwndhelper_h

#include <Windows.h>

#include <QtCore/QObject>
#include <thread>
#include <vector>

#include "gitwndwrap.h"
#include "noncopyable.h"
#include "singleton.h"

using std::thread;
using std::vector;

using GitWindows = vector<HWND>;
using GitWindowsWrap = vector<GitWndWrap>;
using GitWndIte = vector<GitWndWrap>::iterator;
using ConstGitWndIte = vector<GitWndWrap>::const_iterator;

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

  bool Put(HWND hwnd, const QString &title);
  const GitWndWrap &Get(HWND hwnd) const;
  void Clear();
  void CloseAllWindows();
  void SetFocus(QWidget *widget);
  void Close(QWidget *widget);
  void ShowWindow(QWidget *widget);
  void ConnectNotify(QObject *obj, const char *funName);
  bool InitGitWidget(HWND git_wnd, QWidget *parent, QString &title,
                     QWidget **widget);
  GitWindowsWrap &GetWindowsWrap();

 private:
  void init();
  bool find(HWND git_hwnd, GitWndWrap **git_wrap);

 private:
  GitWindowsWrap mGitWindowWrap;

  DECLARE_FRIEND_SINGLETON(GitWndHelper);
  DECLARE_PRIVATE_CONSTRUCTOR(GitWndHelper, init);

  NotifyHelper mNotifyHelper;
};

#endif  // gitwndhelper_h