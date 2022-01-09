//
// Created by lukemartinlogan on 1/8/22.
//

#ifndef LABSTOR_DAX_H
#define LABSTOR_DAX_H

#include "io_test.h"

namespace labstor {

struct DAXThread {
    size_t io_offset_;
    void *buf_;
    DAXThread(int ops_per_batch, size_t block_size) : io_offset_(0) {
        int nonce = 12;
        buf_ = spdk_dma_malloc(block_size, 0, NULL);
        memset(buf_, nonce, block_size);
    }
};

class DAXIO : public IOTest {
public:
    void Init();
};

}


#endif //LABSTOR_DAX_H
