//
// Created by lukemartinlogan on 1/9/22.
//

#ifndef LABSTOR_SECTORED_IO_H
#define LABSTOR_SECTORED_IO_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>

class SectoredIO {
private:
    size_t block_size_sectors_;
public:
    void Init(size_t block_size) {
        if((block_size % 512) != 0) {
            printf("Block size is not a multiple of sector size (512)\n");
            exit(1);
        }
        block_size_sectors_ = block_size / 512;
    }
    size_t GetBlockSizeSectors() {
        return block_size_sectors_;
    }
};

#endif //LABSTOR_SECTORED_IO_H
