#include "custom_event.h"

CustomEvent::CustomEvent(QEvent::Type type, const QString &data)
    : QEvent(type), _data(data) {}

CustomEvent::~CustomEvent() {}

const QString &CustomEvent::GetData() const { return _data; }