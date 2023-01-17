#ifndef _SINGLETON_H_
#define _SINGLETON_H_
#include <assert.h>
#include <stdlib.h>

#include <mutex>
#include <thread>

template <typename T>
class CSingleton {
 public:
  static T& GetInstance() {
    assert(m_instance != NULL);
    return *m_instance;
  }

  template <typename... Args>
  static void Init(Args&&... args) {
    std::call_once(
        flag,
        [](auto&&... args) {
          m_instance = new T(std::forward<Args>(args)...);
          if (m_instance) {
            ::atexit(Release);
          }
        },
        args...);
  }

 private:
  static void Release() {
    if (m_instance) {
      delete m_instance;
      m_instance = NULL;
    }
  }
  CSingleton() {}
  ~CSingleton() {}
  CSingleton(const CSingleton& rhs);
  CSingleton& operator=(const CSingleton& rhs);

 private:
  static std::once_flag flag;
  static T* m_instance;
};

template <typename T>
std::once_flag CSingleton<T>::flag;

template <typename T>
T* CSingleton<T>::m_instance = NULL;

#endif  //_SINGLETON_H_