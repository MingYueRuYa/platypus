#pragma once

#include <QWidget>
#include <list>
#include <memory>

#include "qt_ext_tab_bar_define.h"

using std::list;
using std::shared_ptr;

namespace Ui {
class platypusClass;
}

class NcFramelessHelper;
using QtExtTabBarDef::TabState;

class Platypus : public QWidget {
  Q_OBJECT

 public:
  Platypus(QWidget *parent = Q_NULLPTR);
  ~Platypus();
  bool FindWndTitle(unsigned char *pPayload, unsigned long long &size);
  bool WndExit(unsigned char *pPayload, unsigned long long &size);
  bool Stop(unsigned char *pPayload, unsigned long long &size);
  void ReceiveMsg(const wchar_t *msg);

 protected:
  virtual void mouseReleaseEvent(QMouseEvent *event);
  virtual void resizeEvent(QResizeEvent *event);
  virtual void showEvent(QShowEvent *event);
  virtual bool nativeEvent(const QByteArray &eventType, void *message,
                           long *result);
  virtual void customEvent(QEvent * event);

 private:
  void setupUI();
  void initSig();
  bool filterTabBarEvent(QObject *obj, QEvent *event);
  void startGitWnd();
  void setGitFocus();
  void exitWnd(const QString &data);
  void updateTitle(const QString &data);

Q_SIGNALS:

 protected Q_SLOTS:
  void OnAddWnd(HWND git_wnd);
  void OnAddNewTab();
  void OnTabInserted(int index);
  void OnCloseTab(int index);
  void OnAddBtnClicked();
  void OnCloseWnd();
  void OnMinWnd();
  void OnMaxWnd();
  void OnRestoreWnd();
  void OnMaxOrRestore();

 private:
  Ui::platypusClass *ui;

  NcFramelessHelper *frame_less_helper_ = nullptr;
};