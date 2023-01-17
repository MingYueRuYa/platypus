#ifndef singleton_spdlog_h
#define singleton_spdlog_h

#include "singleton.h"
#include "spdhelp.h"

namespace spdlog {
    template <typename... Arags>
    inline void InitLog(Arags&&... args) {
        CSingleton<SpdHelper>::Init(args...);
    }

    inline SpdHelper &logInstance() {
        return CSingleton<SpdHelper>::GetInstance();
    }
}

#endif // singleton_spdlog_h