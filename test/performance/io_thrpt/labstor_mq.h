
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef LABSTOR_LABSTOR_MQ_H
#define LABSTOR_LABSTOR_MQ_H

#include <vector>
#include "labstor/userspace/client/client.h"
#include "labmods/blkdev_table/client/blkdev_table_client.h"
#include "labmods/mq_driver/client/mq_driver_client.h"
#include "io_test.h"

namespace labstor {

struct LabStorMQThread {
    void *buf_;
    labstor::ipc::qtok_t *qtoks_;
    int hctx_;
    LabStorMQThread(int ops_per_batch, size_t block_size) {
        int nonce = 12;
        buf_ = aligned_alloc(4096, block_size);
        memset(buf_, nonce, block_size);
        qtoks_ = new labstor::ipc::qtok_t[ops_per_batch];
        hctx_ = -1;
    }
};

class LabStorMQ : public IOTest {
private:
    LABSTOR_IPC_MANAGER_T ipc_manager_;
    int dev_id_;
    labstor::BlkdevTable::Client blkdev_table_;
    labstor::MQDriver::Client mq_driver_;
    std::vector<LabStorMQThread> thread_bufs_;
    int num_hw_queues_;
public:
    LabStorMQ() = default;
    void Init(char *path, labstor::Generator *generator) {
        IOTest::Init(generator);

        //Connect to trusted server
        ipc_manager_ = LABSTOR_IPC_MANAGER;
        ipc_manager_->Connect();

        //Register BDEV
        blkdev_table_.Register();
        dev_id_ = blkdev_table_.RegisterBlkdev(path);

        //Register MQ driver
        mq_driver_.Register();

        //View the number of HW queues
        num_hw_queues_ = mq_driver_.GetNumHWQueues(dev_id_);

        //Store per-thread data
        for(int i = 0; i < GetNumThreads(); ++i) {
            thread_bufs_.emplace_back(GetOpsPerBatch(), GetBlockSizeBytes());
        }
    }

    void Write() {
        int tid = labstor::ThreadLocal::GetTid();
        struct LabStorMQThread &thread = thread_bufs_[tid];
        if(thread.hctx_ == -1) {
            thread.hctx_ = sched_getcpu()%num_hw_queues_;
        }
        int hctx = thread.hctx_;
        for(size_t i = 0; i < GetOpsPerBatch(); ++i) {
            thread.qtoks_[i] = (mq_driver_.AWrite(
                    dev_id_,
                    thread.buf_,
                    GetBlockSizeBytes(),
                    GetOffsetUnits(tid),
                    hctx));
        }
        ipc_manager_->Wait(thread.qtoks_, GetOpsPerBatch());
    }

    void Read() {
        int tid = labstor::ThreadLocal::GetTid();
        struct LabStorMQThread &thread = thread_bufs_[tid];
        if(thread.hctx_ == -1) {
            thread.hctx_ = sched_getcpu()%num_hw_queues_;
        }
        int hctx = thread.hctx_;
        for(size_t i = 0; i < GetOpsPerBatch(); ++i) {
            thread.qtoks_[i] = (mq_driver_.ARead(
                    dev_id_,
                    thread.buf_,
                    GetBlockSizeBytes(),
                    GetOffsetUnits(tid),
                    hctx));
        }
        ipc_manager_->Wait(thread.qtoks_, GetOpsPerBatch());
    }
};

}

#endif //LABSTOR_LABSTOR_MQ_H