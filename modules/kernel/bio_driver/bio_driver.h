//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_BIO_DRIVER_H
#define LABSTOR_BIO_DRIVER_H

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_poll.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>

#define BIO_DRIVER_MODULE_ID "bio_driver"

enum {
    LABSTOR_BIO_OK=0,
    LABSTOR_BIO_CANT_ALLOCATE_PAGES=-102,
    LABSTOR_BIO_INVALID_DEVICE_ID=-103,
    LABSTOR_BIO_CANNOT_ALLOCATE_BIO=-104,
    LABSTOR_BIO_CANNOT_ALLOCATE_REQUEST=-105,
    LABSTOR_BIO_DEVICE_BUSY=-106,
    LABSTOR_BIO_NOT_OK=-107,

};

#ifdef __cplusplus
namespace labstor::BIODriver {
enum class Ops {
    kRegister,
    kWrite,
    kRead,
    kIOComplete
};
}
#endif

#ifdef KERNEL_BUILD
enum {
    LABSTOR_BIO_DRIVER_REGISTER,
    LABSTOR_BIO_DRIVER_WRITE,
    LABSTOR_BIO_DRIVER_READ
};
#endif

struct labstor_bio_driver_request {
    struct labstor_request header_;
    int dev_id_;
    void *user_buf_;
    size_t sector_;
    size_t buf_size_;
    int pid_;

#ifdef __cplusplus
    inline void Start(int ns_id, int pid, labstor::BIODriver::Ops op, int dev_id, void *user_buf, size_t buf_size, size_t sector) {
        Start(ns_id, pid, static_cast<int>(op), dev_id, user_buf, buf_size, sector);
    }
    inline void Start(int ns_id, struct labstor_bio_driver_request *rq) {
        Start(ns_id, rq->pid_, rq->header_.op_, rq->dev_id_, rq->user_buf_, rq->buf_size_, rq->sector_);
    }
    inline void Start(int ns_id, int pid, int op, int dev_id, void *user_buf, size_t buf_size, size_t sector) {
        header_.ns_id_ = ns_id;
        header_.op_ = op;
        pid_ = pid;
        dev_id_ = dev_id;
        user_buf_ = user_buf;
        buf_size_ = buf_size;
        sector_ = sector;
    }
    void Copy(labstor_bio_driver_request *rq) {
        header_.Copy(&rq->header_);
    }
#endif

    struct labstor_queue_pair *qp_;
};

#ifdef __cplusplus
struct labstor_bio_driver_poll_request : public labstor::ipc::poll_request_single<labstor_bio_driver_request> {
    void Init(labstor::ipc::queue_pair *qp, labstor_bio_driver_request *reply_rq, labstor::ipc::qtok_t &poll_qtok) {
        int op = static_cast<int>(labstor::BIODriver::Ops::kIOComplete);
        labstor::ipc::poll_request_single<labstor_bio_driver_request>::Init(qp, reply_rq, poll_qtok, op);
    }

};
#endif

#endif //LABSTOR_BIO_DRIVER_H
