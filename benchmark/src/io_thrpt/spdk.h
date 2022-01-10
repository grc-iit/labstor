//
// Created by lukemartinlogan on 1/8/22.
//

#ifndef LABSTOR_SPDK_H
#define LABSTOR_SPDK_H

//https://github.com/spdk/spdk/blob/master/examples/nvme/hello_world/hello_world.c

#include "io_test.h"
#include "sectored_io.h"
#include <labstor/types/thread_local.h>

#include <spdk/stdinc.h>
#include <spdk/nvme.h>
#include <spdk/vmd.h>
#include <spdk/nvme_zns.h>
#include <spdk/env.h>
#include <spdk/string.h>
#include <spdk/log.h>

namespace labstor {

class SPDKIOThread {
public:
    size_t io_offset_;
    void *buf_;
    struct spdk_nvme_ctrlr	*ctrlr_;
    struct spdk_nvme_ns	*nvme_ns_;
    struct spdk_nvme_qpair	*qpair_;
    bool zone_complete_;
    bool *completions_;
public:
    SPDKIOThread(struct spdk_nvme_ctrlr	*ctrlr, spdk_nvme_ns *nvme_ns, int ops_per_batch, size_t block_size, size_t io_offset) {
        ctrlr_ = ctrlr;
        nvme_ns_ = nvme_ns;
        io_offset_ = io_offset;
        int nonce = 0xF;

        //Create queue pair
        qpair_ = spdk_nvme_ctrlr_alloc_io_qpair(ctrlr_, NULL, 0);
        if (qpair_ == NULL) {
            printf("ERROR: spdk_nvme_ctrlr_alloc_io_qpair() failed\n");
            exit(1);
        }

        //Create buffer
        buf_ = spdk_zmalloc(block_size, 4096, NULL, SPDK_ENV_SOCKET_ID_ANY, SPDK_MALLOC_DMA);
        if (buf_ == NULL) {
            printf("ERROR: write buffer allocation failed\n");
            return;
        }
        memset(buf_, nonce, block_size);

        //Initialize zoned namespace for writing
        if (spdk_nvme_ns_get_csi(nvme_ns_) == SPDK_NVME_CSI_ZNS) {
            zone_complete_ = false;
            if (spdk_nvme_zns_reset_zone(nvme_ns_, qpair_,
                                         0, /* starting LBA of the zone to reset */
                                         false, /* don't reset all zones */
                                         reset_zone_complete,
                                         this)) {
                fprintf(stderr, "starting reset zone I/O failed\n");
                exit(1);
            }
            while (!zone_complete_) {
                spdk_nvme_qpair_process_completions(qpair_, 0);
            }
        }

        //Completion vector
        completions_ = reinterpret_cast<bool*>(calloc(ops_per_batch, sizeof(bool)));
    }
    ~SPDKIOThread() {
        spdk_nvme_ctrlr_free_io_qpair(qpair_);
        spdk_free(buf_);
    }
    bool IsComplete(int id) {
        if(completions_[id]) {
            completions_[id] = false;
            return true;
        }
        return false;
    }

private:
    static void reset_zone_complete(void *arg, const struct spdk_nvme_cpl *completion) {
        SPDKIOThread *io_thread = reinterpret_cast<SPDKIOThread*>(arg);
        io_thread->zone_complete_ = true;
        if (spdk_nvme_cpl_is_error(completion)) {
            spdk_nvme_qpair_print_completion(io_thread->qpair_, (struct spdk_nvme_cpl *)completion);
            fprintf(stderr, "I/O error status: %s\n", spdk_nvme_cpl_get_status_string(&completion->status));
            fprintf(stderr, "Reset zone I/O failed, aborting run\n");
            exit(1);
        }
    }
};

class SPDKIO : public IOTest, public SectoredIO {
public:
    struct spdk_nvme_ctrlr	*ctrlr_;
    struct spdk_nvme_ns	*nvme_ns_;
    struct spdk_nvme_transport_id transport_id_;
    std::vector<SPDKIOThread> thread_bufs_;
public:
    void Init(size_t block_size, size_t total_size, int ops_per_batch, int nthreads) {
        int ret = 0;
        struct spdk_env_opts opts;
        IOTest::Init(block_size, total_size, ops_per_batch, nthreads);
        SectoredIO::Init(GetBlockSize());

        //Initialize SPDK environment
        spdk_env_opts_init(&opts);
        opts.name = "hello_world";
        if (spdk_env_init(&opts) < 0) {
            fprintf(stderr, "Unable to initialize SPDK env\n");
            exit(1);
        }
        printf("Initialized env\n");

        //Probe NVMe devices to get controller and transport id
        memset(reinterpret_cast<void*>(&transport_id_), 0, sizeof(struct spdk_nvme_transport_id));
        spdk_nvme_trid_populate_transport(&transport_id_, SPDK_NVME_TRANSPORT_PCIE);
        snprintf(transport_id_.subnqn, sizeof(transport_id_.subnqn), "%s", SPDK_NVMF_DISCOVERY_NQN);
        ret = spdk_nvme_probe(&transport_id_, this, probe_cb, attach_cb, NULL);
        if (ret != 0) {
            fprintf(stderr, "spdk_nvme_probe() failed\n");
            exit(1);
        }
        printf("Probed env\n");

        //Get NVMe namespace
        int nsid = spdk_nvme_ctrlr_get_first_active_ns(ctrlr_);
        nvme_ns_ = spdk_nvme_ctrlr_get_ns(ctrlr_, nsid);
        if (nvme_ns_ == NULL) {
            printf("Could not acquire namespace\n");
            exit(1);
        }
        if (!spdk_nvme_ns_is_active(nvme_ns_)) {
            printf("NVMe namespace is not active\n");
            exit(1);
        }
        printf("Namespace ID: %d size: %juGB\n", spdk_nvme_ns_get_id(nvme_ns_), spdk_nvme_ns_get_size(nvme_ns_) / 1000000000);

        //Initialize per-thread data
        for(int i = 0; i < GetNumThreads(); ++i) {
            thread_bufs_.emplace_back(ctrlr_, nvme_ns_, GetOpsPerBatch(), GetBlockSize(), GetIOPerThread());
        }
    }

    void Write() {
        int tid = labstor::ThreadLocal::GetTid(), ret;
        struct SPDKIOThread &thread = thread_bufs_[tid];
        //Submit the I/O request
        for(size_t i = 0; i < GetOpsPerBatch(); ++i) {
            ret = spdk_nvme_ns_cmd_write(
                    thread.nvme_ns_,
                    thread.qpair_,
                    thread.buf_,
                    thread.io_offset_ + i*GetBlockSize(), /* LBA start */
                    GetBlockSizeSectors(), /* number of LBAs */
                    io_complete, &thread.completions_[i], 0);
            if (ret != 0) {
                fprintf(stderr, "starting write I/O failed\n");
                exit(1);
            }
        }
        //Wait for completion
        for(size_t i = 0; i < GetOpsPerBatch(); ++i) {
            while(!thread.IsComplete(i)) {
                spdk_nvme_qpair_process_completions(thread.qpair_, 0);
            }
        }
    }

    void Read() {
        int tid = labstor::ThreadLocal::GetTid(), ret;
        struct SPDKIOThread &thread = thread_bufs_[tid];
        //Submit the I/O request
        for(size_t i = 0; i < GetOpsPerBatch(); ++i) {
            ret = spdk_nvme_ns_cmd_read(
                    thread.nvme_ns_,
                    thread.qpair_,
                    thread.buf_,
                    thread.io_offset_ + i*GetBlockSize(), /* LBA start */
                    GetBlockSizeSectors(), /* number of LBAs */
                    io_complete, &thread.completions_[i], 0);
            if (ret != 0) {
                fprintf(stderr, "starting write I/O failed\n");
                exit(1);
            }
        }
        //Wait for completion
        for(size_t i = 0; i < GetOpsPerBatch(); ++i) {
            while(!thread.IsComplete(i)) {
                spdk_nvme_qpair_process_completions(thread.qpair_, 0);
            }
        }
    }

private:
    static bool probe_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
             struct spdk_nvme_ctrlr_opts *opts) {
        return true;
    }

    static void attach_cb(void *cb_ctx, const struct spdk_nvme_transport_id *trid,
              struct spdk_nvme_ctrlr *ctrlr, const struct spdk_nvme_ctrlr_opts *opts) {
        SPDKIO *test = reinterpret_cast<SPDKIO*>(cb_ctx);
        test->transport_id_ = *trid;
        test->ctrlr_ = ctrlr;
    }

    static void io_complete(void *arg, const struct spdk_nvme_cpl *completion) {
        SPDKIOThread *io_thread = reinterpret_cast<SPDKIOThread*>(arg);
        bool *event = reinterpret_cast<bool*>(arg);
        if (spdk_nvme_cpl_is_error(completion)) {
            spdk_nvme_qpair_print_completion(io_thread->qpair_, (struct spdk_nvme_cpl *)completion);
            fprintf(stderr, "I/O error status: %s\n", spdk_nvme_cpl_get_status_string(&completion->status));
            fprintf(stderr, "Write I/O failed, aborting run\n");
            exit(1);
        }
        *event = true;
    }
};

}

#endif //LABSTOR_SPDK_H
