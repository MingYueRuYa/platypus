#ifndef TAB_BAR_SIGNAL_H
#define TAB_BAR_SIGNAL_H

#include <QObject>
#include <QMouseEvent>

class TabBarSingal : public QObject
{
    Q_OBJECT

public:
    TabBarSingal() {}
    ~TabBarSingal()  {}

signals:
  void addBtnClicked();
  void closeTab(int index);
  void signalStartDrag(int index);
  void signalEndDrag();
  void mouseRelease(QMouseEvent *);

};

#endif // TAB_BAR_SIGNAL_H