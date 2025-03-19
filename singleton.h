#ifndef SINGLETON_H
#define SINGLETON_H

#include "global.h"

template<typename T>
class Singleton {
// 基类中的 protected成员，可用被其派生类访问；
protected:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton<T>& operator=(const Singleton<T>&) = delete;

    static std::shared_ptr<T> _instance;

public:
    static std::shared_ptr<T> GetInstance() {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&](){
            // 为什么不能使用 make_shared<T>：
            // make_shared<T> 需要调用 T的 public构造函数；
            // 而 shared_ptr<T>(new T)则可在 T的成员函数或友元函数中调用私有构造函数来创建 shared_ptr。
            _instance = std::shared_ptr<T>(new T);
        });
        return _instance;
    }

    void PrintAddress() {
        std::cout << _instance.get() << std::endl;
    }

    ~Singleton() {
        // std::cout << "this is singleton destructor." << std::endl;
    }
};

// 类外实例化
template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;

#endif // SINGLETON_H
