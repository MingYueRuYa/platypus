#ifndef custom_event_h
#define custom_event_h

#include <QEvent>
#include <QObject>
#include <qglobal.h>

enum class CusEventType
{
  GitWndExit = QEvent::User + 1000
};

class CustomEvent : public QEvent
{
public:
    CustomEvent(QEvent::Type type, const QString &data);
    virtual ~CustomEvent();
    const QString &GetData() const;

private:
    QString _data;
    Q_DISABLE_COPY_MOVE(CustomEvent);
};

#endif // custom_event_h