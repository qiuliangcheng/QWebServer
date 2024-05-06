#ifndef __QLC_SINGLETON_H__
#define __QLC_SINGLETON_H__
#include <memory>
namespace qlc{

template<class T, class X= void , int N = 0 >
class Singleton{
public:
    static T* GetInstanceX() {
        static T v;
        return &v;
    }
};

template<class T, class X= void , int N = 0 >
class SingletonPtr{
    static std::shared_ptr<T> GetInstancePtr() {
        static std::shared_ptr<T> v(new T);
        return v;
    }


};

}

#endif