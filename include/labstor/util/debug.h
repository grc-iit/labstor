//
// Created by lukemartinlogan on 11/27/21.
//

#ifndef LABSTOR_DEBUG_H
#define LABSTOR_DEBUG_H

class AutoDebug {
public:
    template<typename ...Args>
    AutoDebug(std::string format, Args ...args) {
    }
};

#endif //LABSTOR_DEBUG_H
