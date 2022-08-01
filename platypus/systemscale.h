#ifndef systemscale_h
#define systemscale_h

#include "noncopyable.h"
#include "singleton.h"

class SystemScale;

#define SystemScaleInstance (SINGLETON_INSTANCE(SystemScale))

class SystemScale : SINGLETON_INHERIT(SystemScale)
{
    DECLARE_PRIVATE_CONSTRUCTOR(SystemScale, void)
    DECLARE_FRIEND_SINGLETON(SystemScale) 

public:
    ~SystemScale();
    static bool IsScale(); 
    static double GetScalePrecision();

};

#endif // systemscale_h