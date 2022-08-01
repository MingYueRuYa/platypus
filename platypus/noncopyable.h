/****************************************************************************
**
** Copyright (C) 2019 liushixiongcpp@163.com
** All rights reserved.
**
****************************************************************************/

#ifndef noncopyable_h
#define noncopyable_h

namespace XIBAO {

class NonCopyable
{
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable& operator=(const NonCopyable &) = delete;
};

};

#endif // noncopyable_h
