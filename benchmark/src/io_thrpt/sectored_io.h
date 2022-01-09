//
// Created by lukemartinlogan on 1/9/22.
//

#ifndef LABSTOR_SECTORED_IO_H
#define LABSTOR_SECTORED_IO_H

class SectoredIO {
private:
    size_t block_size_sectors_;
public:
    void Init(size_t block_size) {
        block_size_sectors_ = block_size / 512;
    }
};

#endif //LABSTOR_SECTORED_IO_H
