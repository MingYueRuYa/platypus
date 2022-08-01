﻿#ifndef gitwndhelper_h
#define gitwndhelper_h

#include "singleton.h"
#include "noncopyable.h"
#include "gitwndwrap.h"

#include <vector>
#include <thread>

#include <Windows.h>

#include <QtCore/QObject>

using std::vector;
using std::thread;

using GitWindows        = vector<HWND>;
using GitWindowsWrap    = vector<GitWndWrap>;
using GitWndIte			= vector<GitWndWrap>::iterator;

#define GitWndHelperInstance SINGLETON_INSTANCE(GitWndHelper)
#define GitWndHelperFinalize SINGLETON_FINALIZE(GitWndHelper)

class NotifyHelper : public QObject, public XIBAO::NonCopyable
{
	Q_OBJECT

public:
	void NotifyNewHandle() {
		emit onNewHandle();
	}		

	void Connect(QObject *reciver, const char *OnAddWnd) {
		// 注意最后一个参数，不能在和接受者同一个线程中调用，否则会造成死锁
		connect(this, SIGNAL(onNewHandle()), reciver, SLOT(OnAddWnd()),
				Qt::BlockingQueuedConnection);
	}

signals:
	void onNewHandle();

};

class GitWndHelper : SINGLETON_INHERIT(GitWndHelper)
{
public:    
    ~GitWndHelper();

    void EnumWindows();
    void Put(HWND hwnd);
    void Clear();
    void CloseAllWindows();
	void SetFocus(QWidget *widget);
	void Close(QWidget *widget);
	void ShowWindow(QWidget *widget);
	void ConnectNotify(QWidget *widget, const char *funName);

	GitWindowsWrap &GetWindowsWrap();

private:
	void init();

private:
    GitWindowsWrap mGitWindowWrap;

    DECLARE_FRIEND_SINGLETON(GitWndHelper);
    DECLARE_PRIVATE_CONSTRUCTOR(GitWndHelper, init);

	bool	mStopThread;
	std::thread	mPollingWndThread;

	NotifyHelper mNotifyHelper;
};

#endif // gitwndhelper_h