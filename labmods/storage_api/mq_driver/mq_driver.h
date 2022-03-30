//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_MQ_DRIVER_H
#define LABSTOR_MQ_DRIVER_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
#include "labstor/types/data_structures/c/shmem_queue_pair.h"

#define MQ_DRIVER_MODULE_ID "MQ_DRIVER"

enum {
    LABSTOR_MQ_OK=0,
    LABSTOR_MQ_CANT_ALLOCATE_PAGES=-102,
    LABSTOR_MQ_INVALID_DEVICE_ID=-103,
    LABSTOR_MQ_CANNOT_ALLOCATE_BIO=-104,
    LABSTOR_MQ_CANNOT_ALLOCATE_REQUEST=-105,
    LABSTOR_MQ_DEVICE_BUSY=-106,
    LABSTOR_MQ_NOT_OK=-107,
    LABSTOR_MQ_CANNOT_POLL=-108,

};

enum {
    LABSTOR_MQ_IS_SUBMITTED_BIT,
    LABSTOR_MQ_POLLED_IO_BIT,
    LABSTOR_MQ_IO_IS_COMPLETE_BIT
};

#define LABSTOR_MQ_IS_SUBMITTED (1 << LABSTOR_MQ_IS_SUBMITTED_BIT)
#define LABSTOR_MQ_POLLED_IO (1 << LABSTOR_MQ_POLLED_IO_BIT)
#define LABSTOR_MQ_IO_IS_COMPLETE (1 << LABSTOR_MQ_IO_IS_COMPLETE_BIT)

enum {
    LABSTOR_MQ_DRIVER_REGISTER,
    LABSTOR_MQ_NUM_HW_QUEUES,
    LABSTOR_MQ_POLL_COMPLETION,
    LABSTOR_MQ_DRIVER_WRITE,
    LABSTOR_MQ_DRIVER_READ
};

#ifdef __cplusplus
typedef unsigned int blk_qc_t;
namespace labstor::MQDriver {
enum class Ops {
    kRegister,
    kGetNumHWQueues,
    kPollCompletion,
    kWrite,
    kRead,
};

struct register_request : labstor::Registrar::register_request {
    int dev_id_;
    void ConstructModuleStart(const std::string &module_id, const std::string &key, int dev_id) {
        labstor::Registrar::register_request::ConstructModuleStart(module_id, key);
        dev_id_ = dev_id;
    }
};

struct io_request : public labstor::ipc::request {
    int dev_id_;
    void *user_buf_;
    size_t sector_;
    size_t buf_size_;
    int hctx_;
    int pid_;
    blk_qc_t cookie_;
    int flags_;
    int lock_;
    struct labstor_qtok_t kern_qtok_;
    void *kern_rq_;

    inline void IOClientStart(int ns_id, int pid, labstor::MQDriver::Ops op, void *user_buf, size_t buf_size, size_t sector, int hctx) {
        IOStart(ns_id, pid, static_cast<int>(op), 0, user_buf, buf_size, sector, hctx);
    }
    inline void IOKernelStart(int ns_id, io_request *rq) {
        IOStart(ns_id, rq->pid_, rq->op_, rq->dev_id_, rq->user_buf_, rq->buf_size_, rq->sector_, rq->hctx_);
    }
    inline void IOStart(int ns_id, int pid, int op, int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx) {
        ns_id_ = ns_id;
        op_ = op;
        pid_ = pid;
        dev_id_ = dev_id;
        user_buf_ = user_buf;
        buf_size_ = buf_size;
        sector_ = sector;
        hctx_ = hctx;
        flags_ = 0;
        code_ = 0;
    }
    inline void PollStart() {
        op_ = static_cast<int>(Ops::kPollCompletion);
    }
    bool IsSubmitted() {
        return __atomic_load_n(&flags_, __ATOMIC_RELAXED) & LABSTOR_MQ_IS_SUBMITTED;
    }
    void AcknowledgeSubmission() {
        __atomic_or_fetch(&flags_, LABSTOR_MQ_IS_SUBMITTED, __ATOMIC_RELAXED);
    }
    bool PollingEnabled() {
        return __atomic_load_n(&flags_, __ATOMIC_RELAXED) & LABSTOR_MQ_POLLED_IO;
    }
    bool IOIsComplete() {
        return __atomic_load_n(&flags_, __ATOMIC_RELAXED) & LABSTOR_MQ_IO_IS_COMPLETE;
    }
};

}
#endif

struct labstor_mq_driver_request {
    struct labstor_request header_;
    int dev_id_;
    void *user_buf_;
    size_t sector_;
    size_t buf_size_;
    int hctx_;
    int pid_;
    int num_hw_queues_;
    blk_qc_t cookie_;
    int flags_;
    int lock_;
    struct labstor_qtok_t kern_qtok_;
    void *kern_rq_;
};

#endif //LABSTOR_MQ_DRIVER_H
