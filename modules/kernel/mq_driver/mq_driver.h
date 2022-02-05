//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_MQ_DRIVER_H
#define LABSTOR_MQ_DRIVER_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_poll.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>

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

#ifdef __cplusplus
typedef unsigned int blk_qc_t;
namespace labstor::MQDriver {
enum class Ops {
    kRegister,
    kWrite,
    kRead,
    kIOComplete,
    kGetNumHWQueues,
    kPollCompletion
};
}
#endif

#ifdef KERNEL_BUILD
enum {
    LABSTOR_MQ_DRIVER_REGISTER,
    LABSTOR_MQ_DRIVER_WRITE,
    LABSTOR_MQ_DRIVER_READ,
    LABSTOR_MQ_IO_COMPLETE,
    LABSTOR_MQ_NUM_HW_QUEUES,
    LABSTOR_MQ_POLL_COMPLETION
};
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

#ifdef __cplusplus
    inline void Start(int ns_id, int pid, labstor::MQDriver::Ops op, int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx) {
        Start(ns_id, pid, static_cast<int>(op), dev_id, user_buf, buf_size, sector, hctx);
    }
    inline void Start(int ns_id, struct labstor_mq_driver_request *rq) {
        Start(ns_id, rq->pid_, rq->header_.op_, rq->dev_id_, rq->user_buf_, rq->buf_size_, rq->sector_, rq->hctx_);
    }
    inline void Start(int ns_id, int dev_id) {
        Start(ns_id, 0, static_cast<int>(labstor::MQDriver::Ops::kGetNumHWQueues), dev_id, nullptr, 0, 0, 0);
    }
    inline void Start(int ns_id, int pid, int op, int dev_id, void *user_buf, size_t buf_size, size_t sector, int hctx) {
        header_.ns_id_ = ns_id;
        header_.op_ = op;
        pid_ = pid;
        dev_id_ = dev_id;
        user_buf_ = user_buf;
        buf_size_ = buf_size;
        sector_ = sector;
        hctx_ = hctx;
        flags_ = 0;
    }
    int GetNumHWQueues() {
        return num_hw_queues_;
    }
    void Copy(labstor_mq_driver_request *rq) {
        header_.Copy(&rq->header_);
        num_hw_queues_ = rq->num_hw_queues_;
    }

    bool IsSubmitted() {
        return flags_ & LABSTOR_MQ_IS_SUBMITTED;
    }
    void AcknowledgeSubmission() {
        flags_ |= LABSTOR_MQ_IS_SUBMITTED;
    }
    bool PollingEnabled() {
        return flags_ & LABSTOR_MQ_POLLED_IO;
    }
    void BeginIOPoll() {
        header_.ns_id_ = MQ_DRIVER_RUNTIME_ID;
        header_.op_ = static_cast<int>(labstor::MQDriver::Ops::kPollCompletion);
    }
    bool IOIsComplete() {
        return flags_ & LABSTOR_MQ_IO_IS_COMPLETE;
    }
#endif
};

#ifdef __cplusplus
struct labstor_mq_driver_poll_request : public labstor::ipc::poll_request_single<labstor_mq_driver_request> {
    labstor_mq_driver_request *poll_rq_;
    void Init(labstor::ipc::queue_pair *qp, labstor_mq_driver_request *reply_rq, labstor::ipc::qtok_t poll_qtok, labstor_mq_driver_request *poll_rq) {
        int op = static_cast<int>(labstor::MQDriver::Ops::kIOComplete);
        poll_rq_ = poll_rq;
        labstor::ipc::poll_request_single<labstor_mq_driver_request>::Init(qp, reply_rq, poll_qtok, op);
    }
};
#endif

#endif //LABSTOR_MQ_DRIVER_H
