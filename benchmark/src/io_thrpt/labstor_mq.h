//
// Created by lukemartinlogan on 1/3/22.
//

#ifndef LABSTOR_LABSTOR_MQ_H
#define LABSTOR_LABSTOR_MQ_H

#include <labstor/userspace/client/client.h>
#include <modules/kernel/blkdev_table/client/blkdev_table_client.h>
#include <modules/kernel/mq_driver/client/mq_driver_client.h>
#include "io_test.h"

namespace labstor {

class LabstorMQ : public IOTest {
private:
    int fd_;
    labstor::BlkdevTable::Client blkdev_table_;
    labstor::MQDriver::Client mq_driver_;
    int dev_id_;
    size_t block_size_, block_size_sectors_;
    size_t total_size_;
    void *buf_;
public:
    void Init(char *path, size_t block_size, size_t total_size) {
        //Inputs
        block_size_ = block_size;
        block_size_sectors_ = block_size / 512;
        if((block_size % 512) != 0) {
            printf("Error: block size is not a multiple of 512 bytes\n");
            exit(1);
        }
        total_size_ = total_size;

        //Connect to trusted server
        LABSTOR_IPC_MANAGER_T ipc_manager_ = LABSTOR_IPC_MANAGER;
        ipc_manager_->Connect();

        //Register BDEV
        blkdev_table_.Register();
        dev_id_ = blkdev_table_.RegisterBlkdev(path);

        //Create I/O request data
        int nonce = 12;
        buf_ = aligned_alloc(4096, block_size_);
        memset(buf_, nonce, block_size_);

        //Register MQ driver
        mq_driver_.Register();
    }

    void Write() {
        int hctx = 0;
        size_t sector = 0;
        for (size_t i = 0; i < total_size_; i += block_size_) {
            mq_driver_.Write(dev_id_, buf_, block_size_, sector, hctx);
            sector += block_size_sectors_;
        }
    }

    void Read() {
        int hctx = 0;
        size_t sector = 0;
        for (size_t i = 0; i < total_size_; i += block_size_) {
            mq_driver_.Read(dev_id_, buf_, block_size_, sector, hctx);
            sector += block_size_sectors_;
        }
    }

    size_t GetTotalIO() {
        return total_size_;
    }

    size_t GetNumOps() {
        if ((total_size_ % block_size_) == 0) {
            return total_size_ / block_size_;
        } else {
            return total_size_ / block_size_ + 1;
        }
    }
};

}

#endif //LABSTOR_LABSTOR_MQ_H
