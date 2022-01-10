//
// Created by lukemartinlogan on 1/3/22.
//

#ifndef LABSTOR_LABSTOR_MQ_H
#define LABSTOR_LABSTOR_MQ_H

#include <vector>
#include <labstor/userspace/client/client.h>
#include <modules/kernel/blkdev_table/client/blkdev_table_client.h>
#include <modules/kernel/bio_driver/client/bio_driver_client.h>
#include "io_test.h"
#include "sectored_io.h"


namespace labstor {

struct LabStorBIOThread {
    size_t sector_;
    void *buf_;
    labstor::ipc::qtok_set qtoks_;
    LabStorBIOThread(int ops_per_batch, size_t block_size) : sector_(0) {
        int nonce = 12;
        buf_ = aligned_alloc(4096, block_size);
        memset(buf_, nonce, block_size);
        void *qtok_buf_ = malloc(labstor::ipc::qtok_set::GetSize(ops_per_batch));
        qtoks_.Init(qtok_buf_, labstor::ipc::qtok_set::GetSize(ops_per_batch));
    }
};

class LabStorBIO : public IOTest, public SectoredIO {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    int dev_id_;
    labstor::BlkdevTable::Client blkdev_table_;
    labstor::BIODriver::Client bio_driver_;
    size_t block_size_, block_size_sectors_;
    std::vector<LabStorBIOThread> thread_bufs_;
public:
    void Init(char *path, size_t block_size, size_t total_size, int ops_per_batch, int nthreads) {
        IOTest::Init(block_size, total_size, ops_per_batch, nthreads);
        SectoredIO::Init(GetBlockSize());

        //Connect to trusted server
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        ipc_manager_->Connect();

        //Register BDEV
        blkdev_table_.Register();
        dev_id_ = blkdev_table_.RegisterBlkdev(path);

        //Register MQ driver
        bio_driver_.Register();

        //Store per-thread data
        for(int i = 0; i < nthreads_; ++i) {
            thread_bufs_.emplace_back(ops_per_batch, block_size_);
        }
    }

    void Write() {
        int tid = labstor::ThreadLocal::GetTid();
        struct LabStorBIOThread &thread = thread_bufs_[tid];
        for(int i = 0; i < GetOpsPerBatch(); ++i) {
            thread.qtoks_.Enqueue(bio_driver_.AWrite(dev_id_, thread.buf_, block_size_, thread.sector_));
            thread.sector_ += block_size_sectors_;
        }
        ipc_manager_->Wait(thread.qtoks_);
        thread.sector_ += block_size_sectors_;
    }

    void Read() {
        int tid = labstor::ThreadLocal::GetTid();
        struct LabStorBIOThread &thread = thread_bufs_[tid];
        for(int i = 0; i < GetOpsPerBatch(); ++i) {
            thread.qtoks_.Enqueue(bio_driver_.ARead(dev_id_, thread.buf_, block_size_, thread.sector_));
            thread.sector_ += block_size_sectors_;
        }
        ipc_manager_->Wait(thread.qtoks_);
        thread.sector_ += block_size_sectors_;
    }
};

}

#endif //LABSTOR_LABSTOR_MQ_H
