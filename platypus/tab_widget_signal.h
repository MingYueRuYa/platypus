#ifndef TAB_WIDGET_SIGNAL_H
#define TAB_WIDGET_SIGNAL_H

#include <QObject>
#include <QMouseEvent>

class TabWidgetSignal : public QObject {
  Q_OBJECT

 public:
  TabWidgetSignal() {}
  ~TabWidgetSignal() {}

 signals:
  void addBtnClicked();
  void tabInserted(int index);
  void tabClosed(int index);
  void closeWnd();
  void minWnd();
  void maxWnd();
  void restoreWnd();
  void tabBarDoubleClicked();
  void tabBarClicked(int index);
  void helpClicked();
  void tabBarMouseRelease(QMouseEvent *e);
};

#endif  // TAB_WIDGET_SIGNAL_H
