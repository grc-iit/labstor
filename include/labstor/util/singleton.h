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
    static std::shared_ptr <T> obj_;
public:
    Singleton() = default;
    static std::shared_ptr <T> GetInstance() {
        return obj_;
    }
};

template<typename T>
std::shared_ptr <T> Singleton<T>::obj_ = nullptr;

}

#endif //SCS_SINGLETON_H
