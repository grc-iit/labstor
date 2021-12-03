//
// Created by lukemartinlogan on 8/1/21.
//

#ifndef SCS_SINGLETON_H
#define SCS_SINGLETON_H

#include <memory>

namespace scs {

template<typename T>
class Singleton {
private:
    static T* obj_;
public:
    Singleton() = default;
    static T* GetInstance() {
        if(!obj_) { obj_ = new T(); }
        return obj_;
    }
};

#define SINGLETON_T(T) T*

}

#endif //SCS_SINGLETON_H
