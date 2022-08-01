/****************************************************************************
**
** Copyright (C) 2017 liushixiongcpp@163.com
** All rights reserved.
**
****************************************************************************/

#ifndef singleton_h
#define singleton_h

#include <mutex>

using std::mutex;


namespace XIBAO {

#define SINGLETON_INHERIT(CLASSNAME) \
								public XIBAO::singleton::Singleton<CLASSNAME>

#define SINGLETON_GET_INSTANCE(CLASSNAME) \
					XBIAO::singleton::Singleton<CLASSNAME>::GetInstance()

#define SINGLETON_FINALIZE(CLASSNAME) \
					XIBAO::singleton::Singleton<CLASSNAME>::Finailize()

#define SINGLETON_INSTANCE(CLASSNAME) \
					XIBAO::singleton::Singleton<CLASSNAME>::Instance()

#define DECLARE_FRIEND_SINGLETON(CLASSNAME)	\
						private: \
							friend class XIBAO::singleton::Singleton<CLASSNAME>;

// 将构造函数和拷贝构造，赋值函数设置为私有，同时默认构造函数可以调用init函数
#define DECLARE_PRIVATE_CONSTRUCTOR(CLASSNAME, INIT_FUNCTION) 	\
				private: \
				CLASSNAME() { INIT_FUNCTION(); } \
				CLASSNAME(const CLASSNAME &) {} \
				CLASSNAME& operator =(const CLASSNAME &) { return *this; }

namespace singleton
{
template <typename T>
class Singleton
{
public:
	template<typename... Args>
	static T& Instance(Args&&... args)
	{
        if (NULL == mInstance )
        {
            mMutex.lock();
            if (NULL == mInstance) {
                mInstance = new T(std::forward<Args>(args)...);
                atexit(Destroy); 
            }
            mMutex.unlock();
            return *mInstance;
        }
        return *mInstance;
	}

	static T& GetInstance()
	{
		if (nullptr == mInstance) {
			throw std::logic_error("the instance is not init, "\
									"please initialize the instance first");
		}

		return *mInstance;
	}

	static void Finailize() {
		Destroy();
	}

protected:
    Singleton(void) {}
    virtual ~Singleton(void) {}

private:
    Singleton(const Singleton& rhs) = delete;
    Singleton(Singleton&& rhs) = delete;
    Singleton& operator= (const Singleton& rhs) = delete;
    Singleton& operator= (Singleton&& rhs) = delete;

    static void Destroy()
    {
        if (mInstance == NULL) { return; }

        delete mInstance;
        mInstance = NULL;
    }

    static T* volatile mInstance;
    static mutex mMutex;
};

template <typename T>
T* volatile Singleton<T>::mInstance = NULL;

template <typename T>
mutex Singleton<T>::mMutex;
}
}

#endif //singleton_h