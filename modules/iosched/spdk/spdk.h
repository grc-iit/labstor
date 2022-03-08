//
// Created by lukemartinlogan on 9/7/21.
//

#ifndef LABSTOR_SPDK_DRIVER_H
#define LABSTOR_SPDK_DRIVER_H

#include <labstor/userspace/util/errors.h>
#include <labstor/constants/constants.h>
#include <labstor/types/data_structures/shmem_qtok.h>
#include "labstor/types/data_structures/queue_pair.h"
#include "labstor/types/data_structures/spsc/shmem_queue_pair.h"
#include "labstor/types/data_structures/async_queue_pair.h"

#include <vector>
#include <list>
#include <cstdio>

#include <spdk/stdinc.h>
#include <spdk/nvme.h>
#include <spdk/vmd.h>
#include <spdk/nvme_zns.h>
#include <spdk/env.h>
#include <spdk/string.h>
#include <spdk/log.h>

#include <labstor/types/basics.h>
#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/types/data_structures/shmem_poll.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
#include <modules/registrar/registrar.h>

#define SPDK_MODULE_ID "SPDK"

namespace labstor::SPDK {
struct queue_pair;

enum class Ops {
    kWrite, kRead
};

struct Device {
    struct spdk_nvme_ctrlr	*ctrlr_;
    struct spdk_nvme_ns	*nvme_ns_;
    struct spdk_nvme_transport_id transport_id_;
    int ns_id_;
    int max_qps_, max_io_rqs_, max_admin_rqs_;
    int sector_size_, max_transfer_size_bytes_;
    const struct spdk_nvme_ctrlr_opts *opts_;

    Device() = default;
    Device(struct spdk_nvme_ctrlr *ctrlr, struct spdk_nvme_ns *nvme_ns, const struct spdk_nvme_transport_id *transport_id, int nsid, const struct spdk_nvme_ctrlr_opts *opts) : opts_(opts) {
        ctrlr_ = ctrlr;
        nvme_ns_ = nvme_ns;
        transport_id_ = *transport_id;
        ns_id_ = nsid;
        sector_size_ = spdk_nvme_ns_get_sector_size(nvme_ns_);
        max_transfer_size_bytes_ = spdk_nvme_ns_get_max_io_xfer_size(nvme_ns_);
        if(opts) {
            max_qps_ = opts->num_io_queues;
            max_io_rqs_ = opts->io_queue_requests;
            max_admin_rqs_ = opts->admin_queue_size;
        }
    }

    void Print() {
        //const struct spdk_nvme_ctrlr_opts *opts = spdk_nvme_ctrlr_get_opts(ctrlr_);
        printf("-----------------------\n");
        printf("TRANSPORT ADDR: %s\n", transport_id_.traddr);
        printf("NAMESPACE ID: %d / %d\n", ns_id_, spdk_nvme_ctrlr_get_num_ns(ctrlr_));
        printf("NAMESPACE SIZE (GiB): %lu\n", spdk_nvme_ns_get_size(nvme_ns_)/(1<<30));
        printf("SECTOR SIZE (bytes): %d\n", sector_size_);
        printf("MAX TRANSFER SIZE (KiB): %d\n", max_transfer_size_bytes_/1024);
        printf("NUM I/O Queues: %d\n", max_qps_);
        printf("MAX I/O Requests: %d\n", max_io_rqs_);
        printf("MAX Admin Requests: %d\n", max_admin_rqs_);
        printf("-----------------------\n");
    }
};

struct io_request : labstor::async_request {
    void *buf_;
    size_t buf_size_bytes_;
    size_t sector_;
    labstor::ipc::shmem_queue_pair *priv_qp_;
    void Init(labstor::SPDK::Ops io_type, void *buf, size_t buf_size_bytes, size_t sector) {
        op_ = static_cast<int>(io_type);
        buf_ = buf;
        buf_size_bytes_ = buf_size_bytes;
        sector_ = sector;
    }
};

struct queue_pair : labstor::async_queue_pair {
    struct spdk_nvme_qpair* qp_;
    Device *dev_;
    bool zone_complete_;
    labstor::ipc::shmem_queue_pair *priv_qp_;

    queue_pair(labstor::ipc::shmem_queue_pair *priv_qp, labstor::ipc::qid_t &qid, Device *dev) {
        //Create QP
        SetQID(qid);
        priv_qp_ = priv_qp;
        dev_ = dev;
        qp_ = spdk_nvme_ctrlr_alloc_io_qpair(dev->ctrlr_, NULL, 0);
        if (qp_ == NULL) {
            throw labstor::SPDK_CANT_CREATE_QP.format(priv_qp->GetQID().cnt_);
        }

        //Initialize zoned namespace for writing
        if (spdk_nvme_ns_get_csi(dev->nvme_ns_) == SPDK_NVME_CSI_ZNS) {
            zone_complete_ = false;
            if (spdk_nvme_zns_reset_zone(dev->nvme_ns_, qp_,
                                         0, /* starting LBA of the zone to reset */
                                         false, /* don't reset all zones */
                                         reset_zone_complete,
                                         this)) {
                throw labstor::SPDK_CANT_RESET_ZONE.format();
            }
            while (!zone_complete_) {
                spdk_nvme_qpair_process_completions(qp_, 0);
            }
        }
    }

    ~queue_pair() {
        spdk_nvme_ctrlr_free_io_qpair(qp_);
    }

    bool _Enqueue(labstor::ipc::request *rq, labstor::ipc::qtok_t &qtok) {
        int ret = -1;
        priv_qp_->Enqueue(rq, qtok);
        priv_qp_->Dequeue(rq);
        qtok.qid_ = GetQID();
        labstor::SPDK::io_request *spdk_rq = reinterpret_cast<labstor::SPDK::io_request*>(rq);
        spdk_rq->priv_qp_ = priv_qp_;
        TRACEPOINT("sector", spdk_rq->sector_, "Buf Size (sectors + rem)", spdk_rq->buf_size_bytes_ / dev_->sector_size_, spdk_rq->buf_size_bytes_ % dev_->sector_size_)
        switch(static_cast<Ops>(spdk_rq->op_)) {
            case labstor::SPDK::Ops::kWrite: {
                ret = spdk_nvme_ns_cmd_write(
                        dev_->nvme_ns_,
                        qp_,
                        spdk_rq->buf_,
                        spdk_rq->sector_, /* LBA start */
                        spdk_rq->buf_size_bytes_ / dev_->sector_size_, /* number of LBAs */
                        _IOComplete, spdk_rq, 0);
                break;
            }
            case labstor::SPDK::Ops::kRead: {
                ret = spdk_nvme_ns_cmd_read(
                        dev_->nvme_ns_,
                        qp_,
                        spdk_rq->buf_,
                        spdk_rq->sector_, /* LBA start */
                        spdk_rq->buf_size_bytes_ / dev_->sector_size_, /* number of LBAs */
                        _IOComplete, spdk_rq, 0);
                break;
            }
        }
        return ret==0;
    }

    inline bool _IsComplete(uint32_t req_id, labstor::ipc::request **rq) {
        _PollCompletions();
        return priv_qp_->IsComplete(req_id, *rq);
    }
private:
    static void reset_zone_complete(void *arg, const struct spdk_nvme_cpl *completion) {
        queue_pair *lib_qp = reinterpret_cast<queue_pair*>(arg);
        lib_qp->zone_complete_ = true;
        if (spdk_nvme_cpl_is_error(completion)) {
            spdk_nvme_qpair_print_completion(lib_qp->qp_, (struct spdk_nvme_cpl *)completion);
            fprintf(stderr, "I/O error status: %s\n", spdk_nvme_cpl_get_status_string(&completion->status));
            fprintf(stderr, "Reset zone I/O failed, aborting run\n");
            exit(1);
        }
    }

    bool _PollCompletions() {
        int ret = spdk_nvme_qpair_process_completions(qp_, 0);
        if(ret < 0) {
            return false;
        }
        return true;
    }

    static void _IOComplete(void *arg, const struct spdk_nvme_cpl *completion) {
        AUTO_TRACE("")
        labstor::SPDK::io_request *spdk_rq = reinterpret_cast<labstor::SPDK::io_request*>(arg);
        spdk_rq->SetCode(LABSTOR_REQUEST_SUCCESS);
        spdk_rq->priv_qp_->Complete(spdk_rq);
    }
};

class Context {
private:
    Device dev_;
    std::list<Device> devs_;
    bool init_;
public:
    Context() : init_(false) {}

    void Init() {
        struct spdk_env_opts opts;
        spdk_env_opts_init(&opts);
        opts.name = "labstor_spdk";
        if (spdk_env_init(&opts) < 0) {
            fprintf(stderr, "Unable to initialize SPDK env\n");
            exit(1);
        }
        printf("Initialized env\n");
    }

    void Probe(bool print=false) {
        struct spdk_nvme_transport_id transport_id;
        int ret;
        memset(reinterpret_cast<void*>(&transport_id), 0, sizeof(struct spdk_nvme_transport_id));
        spdk_nvme_trid_populate_transport(&transport_id, SPDK_NVME_TRANSPORT_PCIE);
        snprintf(transport_id.subnqn, sizeof(transport_id.subnqn), "%s", SPDK_NVMF_DISCOVERY_NQN);
        ret = spdk_nvme_probe(&transport_id, this, probe_cb, attach_cb, NULL);
        if (ret != 0) {
            fprintf(stderr, "spdk_nvme_probe() failed\n");
            exit(1);
        }

        //Print each device
        if(print) {
            for (auto &dev: devs_) {
                dev.Print();
            }
        }
    }

    bool SelectDevice(const std::string &traddr, int ns_id) {
        for(auto &dev : devs_) {
            if(dev.ns_id_ != ns_id) {
                continue;
            }
            if(traddr != std::string(dev.transport_id_.traddr)) {
                continue;
            }
            dev_ = dev;
            return true;
        }
        return false;
    }

    void* Alloc(size_t size) {
        //Create buffer
        void *buf = spdk_malloc(size, 4096, NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_DMA);
        if (buf == NULL) {
            return nullptr;
        }
        return buf;
    }

    void Free(void *buffer) {
        spdk_free(buffer);
    }

    Device* GetDevice() {
        return &dev_;
    }

    int GetNumQueuePairs() {
        return dev_.max_qps_;
    }

    int GetMaxQueueDepth() {
        return dev_.max_io_rqs_;
    }

    void _AddDevice(Device &&dev) {
        devs_.emplace_back(dev);
    }

private:
    static bool probe_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
                         struct spdk_nvme_ctrlr_opts *opts) {
        return true;
    }

    static void attach_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
                          struct spdk_nvme_ctrlr *ctrlr, const struct spdk_nvme_ctrlr_opts *opts) {
        Context *spdk_lib = reinterpret_cast<Context*>(cb_ctx);
        struct spdk_nvme_ns *ns;
        for (int nsid = spdk_nvme_ctrlr_get_first_active_ns(ctrlr);
             nsid != 0;
             nsid = spdk_nvme_ctrlr_get_next_active_ns(ctrlr, nsid)) {
            ns = spdk_nvme_ctrlr_get_ns(ctrlr, nsid);
            if (ns == NULL) {
                printf("NO NS\n");
                continue;
            }
            spdk_lib->_AddDevice(Device(ctrlr, ns, trid, nsid, opts));
        }
    }
};

}

#endif //LABSTOR_SPDK_DRIVER_H
