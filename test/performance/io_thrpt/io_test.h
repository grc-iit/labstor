//
// Created by lukemartinlogan on 1/3/22.
//

#ifndef LABSTOR_IO_TEST_H
#define LABSTOR_IO_TEST_H

#include <stdint.h>

namespace labstor {

class IOTest {
public:
    virtual void Read() = 0;
    virtual void Write() = 0;
    virtual size_t GetTotalIO() = 0;
    virtual size_t GetNumOps() = 0;
};

}

#endif //LABSTOR_IO_TEST_H
