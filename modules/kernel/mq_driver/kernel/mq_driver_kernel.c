//
// Created by lukemartinlogan on 5/6/21.
//

/*
 * A kernel module that constructs bio and request objects, and submits them to the underlying drivers.
 * */

#define pr_fmt(fmt) "%s:%s: " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kobject.h>

#include <linux/sbitmap.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/bio.h>
#include <linux/blk_types.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/list.h>
#include <linux/cpumask.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include <labstor/constants/constants.h>
#include <labstor/types/data_structures/spsc/shmem_queue_pair.h>
#include <labstor/kernel/server/module_manager.h>
#include <labstor/kernel/server/kernel_server.h>

#include <mq_driver/kernel/mq_driver_kernel.h>
#include <blkdev_table/kernel/blkdev_table_kernel.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that performs I/O with underlying storage devices");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("request_layer_km");

//Prototypes
static int __init init_mq_driver_km(void);
static void __exit exit_mq_driver_km(void);


/**
 * Required Kernel Code
 * */

#define BLK_MQ_DISPATCH_BUSY_EWMA_WEIGHT  8
#define BLK_MQ_DISPATCH_BUSY_EWMA_FACTOR  4

enum {
    BLK_MQ_TAG_FAIL		= -1U,
    BLK_MQ_TAG_MIN		= 1,
    BLK_MQ_TAG_MAX		= BLK_MQ_TAG_FAIL - 1,
};

struct blk_mq_tags {
    unsigned int nr_tags;
    unsigned int nr_reserved_tags;

    atomic_t active_queues;

    struct sbitmap_queue bitmap_tags;
    struct sbitmap_queue breserved_tags;

    struct request **rqs;
    struct request **static_rqs;
    struct list_head page_list;
};

struct blk_mq_alloc_data {
    /* input parameter */
    struct request_queue *q;
    blk_mq_req_flags_t flags;
    unsigned int shallow_depth;
    unsigned int cmd_flags;

    /* input & output parameter */
    struct blk_mq_ctx *ctx;
    struct blk_mq_hw_ctx *hctx;
};

struct blk_mq_ctxs {
    struct kobject kobj;
    struct blk_mq_ctx __percpu	*queue_ctx;
};

struct blk_mq_ctx {
    struct {
        spinlock_t		lock;
        struct list_head	rq_lists[HCTX_MAX_TYPES];
    } ____cacheline_aligned_in_smp;

    unsigned int		cpu;
    unsigned short		index_hw[HCTX_MAX_TYPES];
    struct blk_mq_hw_ctx 	*hctxs[HCTX_MAX_TYPES];

    /* incremented at dispatch time */
    unsigned long		rq_dispatched[2];
    unsigned long		rq_merged;

    /* incremented at completion time */
    unsigned long		____cacheline_aligned_in_smp rq_completed[2];

    struct request_queue	*queue;
    struct blk_mq_ctxs      *ctxs;
    struct kobject		kobj;
} ____cacheline_aligned_in_smp;

static inline void blk_rq_bio_prep(struct request *rq, struct bio *bio, unsigned int nr_segs)
{
    rq->nr_phys_segments = nr_segs;
    rq->__data_len = bio->bi_iter.bi_size;
    rq->bio = rq->biotail = bio;
    rq->ioprio = bio_prio(bio);
    if (bio->bi_disk)
        rq->rq_disk = bio->bi_disk;
}

static void hctx_unlock(struct blk_mq_hw_ctx *hctx, int srcu_idx) __releases(hctx->srcu)
{
if (!(hctx->flags & BLK_MQ_F_BLOCKING))
rcu_read_unlock();
else
srcu_read_unlock(hctx->srcu, srcu_idx);
}

static void hctx_lock(struct blk_mq_hw_ctx *hctx, int *srcu_idx) __acquires(hctx->srcu)
{
if (!(hctx->flags & BLK_MQ_F_BLOCKING)) {
*srcu_idx = 0;
rcu_read_lock();
} else
*srcu_idx = srcu_read_lock(hctx->srcu);
}

static inline bool hctx_may_queue(struct blk_mq_hw_ctx *hctx,
                                  struct sbitmap_queue *bt)
{
    unsigned int depth, users;

    if (!hctx || !(hctx->flags & BLK_MQ_F_TAG_SHARED))
        return true;
    if (!test_bit(BLK_MQ_S_TAG_ACTIVE, &hctx->state))
        return true;

    /*
     * Don't try dividing an ant
     */
    if (bt->sb.depth == 1)
        return true;

    users = atomic_read(&hctx->tags->active_queues);
    if (!users)
        return true;

    /*
     * Allow at least some tags
     */
    depth = max((bt->sb.depth + users - 1) / users, 4U);
    return atomic_read(&hctx->nr_active) < depth;
}

static inline bool blk_mq_tag_is_reserved(struct blk_mq_tags *tags,
                                          unsigned int tag) {
    return tag < tags->nr_reserved_tags;
}

static inline bool blk_mq_hctx_stopped(struct blk_mq_hw_ctx *hctx)
{
    return test_bit(BLK_MQ_S_STOPPED, &hctx->state);
}

bool __blk_mq_tag_busy(struct blk_mq_hw_ctx *hctx)
{
    if (!test_bit(BLK_MQ_S_TAG_ACTIVE, &hctx->state) &&
        !test_and_set_bit(BLK_MQ_S_TAG_ACTIVE, &hctx->state))
        atomic_inc(&hctx->tags->active_queues);

    return true;
}

static inline struct blk_mq_tags *blk_mq_tags_from_data(struct blk_mq_alloc_data *data)
{
    if (data->flags & BLK_MQ_REQ_INTERNAL)
        return data->hctx->sched_tags;

    return data->hctx->tags;
}

static void blk_mq_update_dispatch_busy(struct blk_mq_hw_ctx *hctx, bool busy)
{
    unsigned int ewma;

    if (hctx->queue->elevator)
        return;

    ewma = hctx->dispatch_busy;

    if (!ewma && !busy)
        return;

    ewma *= BLK_MQ_DISPATCH_BUSY_EWMA_WEIGHT - 1;
    if (busy)
        ewma += 1 << BLK_MQ_DISPATCH_BUSY_EWMA_FACTOR;
    ewma /= BLK_MQ_DISPATCH_BUSY_EWMA_WEIGHT;

    hctx->dispatch_busy = ewma;
}

static int __blk_mq_get_tag(struct blk_mq_alloc_data *data,
                            struct sbitmap_queue *bt)
{
    if (!(data->flags & BLK_MQ_REQ_INTERNAL) &&
        !hctx_may_queue(data->hctx, bt))
        return -1;
    if (data->shallow_depth)
        return __sbitmap_queue_get_shallow(bt, data->shallow_depth);
    else
        return __sbitmap_queue_get(bt);
}

unsigned int blk_mq_get_tag(struct blk_mq_alloc_data *data)
{
    struct blk_mq_tags *tags = blk_mq_tags_from_data(data);
    struct sbitmap_queue *bt;
    unsigned int tag_offset;
    int tag;

    if (data->flags & BLK_MQ_REQ_RESERVED) {
        if (unlikely(!tags->nr_reserved_tags)) {
            WARN_ON_ONCE(1);
            return BLK_MQ_TAG_FAIL;
        }
        bt = &tags->breserved_tags;
        tag_offset = 0;
    } else {
        bt = &tags->bitmap_tags;
        tag_offset = tags->nr_reserved_tags;
    }

    tag = __blk_mq_get_tag(data, bt);
    if (tag != -1)
        return tag + tag_offset;
    return BLK_MQ_TAG_FAIL;

}

void blk_mq_put_tag(struct blk_mq_hw_ctx *hctx, struct blk_mq_tags *tags,
                    struct blk_mq_ctx *ctx, unsigned int tag)
{
    if (!blk_mq_tag_is_reserved(tags, tag)) {
        const int real_tag = tag - tags->nr_reserved_tags;
        sbitmap_queue_clear(&tags->bitmap_tags, real_tag, ctx->cpu);
    } else {
        sbitmap_queue_clear(&tags->breserved_tags, tag, ctx->cpu);
    }
}

static inline void blk_mq_put_dispatch_budget(struct blk_mq_hw_ctx *hctx)
{
    struct request_queue *q = hctx->queue;

    if (q->mq_ops->put_budget)
        q->mq_ops->put_budget(hctx);
}

static inline bool blk_mq_get_dispatch_budget(struct blk_mq_hw_ctx *hctx)
{
    struct request_queue *q = hctx->queue;

    if (q->mq_ops->get_budget)
        return q->mq_ops->get_budget(hctx);
    return true;
}

static inline void __blk_mq_put_driver_tag(struct blk_mq_hw_ctx *hctx,
                                           struct request *rq)
{
    blk_mq_put_tag(hctx, hctx->tags, rq->mq_ctx, rq->tag);
    rq->tag = -1;

    if (rq->rq_flags & RQF_MQ_INFLIGHT) {
        rq->rq_flags &= ~RQF_MQ_INFLIGHT;
        atomic_dec(&hctx->nr_active);
    }
}

static inline void blk_mq_put_driver_tag(struct request *rq)
{
    if (rq->tag == -1 || rq->internal_tag == -1)
        return;

    __blk_mq_put_driver_tag(rq->mq_hctx, rq);
}

static inline bool blk_mq_tag_busy(struct blk_mq_hw_ctx *hctx)
{
    if (!(hctx->flags & BLK_MQ_F_TAG_SHARED))
        return false;

    return __blk_mq_tag_busy(hctx);
}

bool blk_mq_get_driver_tag(struct request *rq)
{
    struct blk_mq_alloc_data data = {
            .q = rq->q,
            .hctx = rq->mq_hctx,
            .flags = BLK_MQ_REQ_NOWAIT,
            .cmd_flags = rq->cmd_flags,
    };
    bool shared;

    if (rq->tag != -1)
        goto done;

    if (blk_mq_tag_is_reserved(data.hctx->sched_tags, rq->internal_tag))
        data.flags |= BLK_MQ_REQ_RESERVED;

    shared = blk_mq_tag_busy(data.hctx);
    rq->tag = blk_mq_get_tag(&data);
    if (rq->tag >= 0) {
        if (shared) {
            rq->rq_flags |= RQF_MQ_INFLIGHT;
            atomic_inc(&data.hctx->nr_active);
        }
        data.hctx->tags->rqs[rq->tag] = rq;
    }

    done:
    return rq->tag != -1;
}

static blk_status_t __blk_mq_issue_directly(struct blk_mq_hw_ctx *hctx,
                                            struct request *rq,
                                            blk_qc_t *cookie, bool last)
{
    struct request_queue *q = rq->q;
    struct blk_mq_queue_data bd = {
            .rq = rq,
            .last = last,
    };
    blk_qc_t new_cookie;
    blk_status_t ret;

    new_cookie = request_to_qc_t(hctx, rq);

    /*
     * For OK queue, we are done. For error, caller may kill it.
     * Any other error (busy), just add it to our list as we
     * previously would have done.
     */
    ret = q->mq_ops->queue_rq(hctx, &bd);
    switch (ret) {
        case BLK_STS_OK:
            blk_mq_update_dispatch_busy(hctx, false);
            *cookie = new_cookie;
            break;
        case BLK_STS_RESOURCE:
        case BLK_STS_DEV_RESOURCE:
            printk("request_layer_km: NEEDS REQUEUE\n");
            blk_mq_update_dispatch_busy(hctx, false);
            *cookie = BLK_QC_T_NONE;
            break;
        default:
            blk_mq_update_dispatch_busy(hctx, false);
            *cookie = BLK_QC_T_NONE;
            printk("request_layer_km: FAILED\n");
            break;
    }

    return ret;
}

static blk_status_t __blk_mq_try_issue_directly(struct blk_mq_hw_ctx *hctx,
                                                struct request *rq,
                                                blk_qc_t *cookie,
                                                bool bypass_insert, bool last)
{
    struct request_queue *q = rq->q;
    bool run_queue = true;

    if (blk_mq_hctx_stopped(hctx) || blk_queue_quiesced(q)) {
        return -1;
    }

    if (q->elevator && !bypass_insert)
        return -1;

    if (!blk_mq_get_dispatch_budget(hctx))
        return -1;

    if (!blk_mq_get_driver_tag(rq)) {
        blk_mq_put_dispatch_budget(hctx);
        return -1;
    }

    pr_debug("GET DRIVER TAG: %d\n", rq->tag);
    return __blk_mq_issue_directly(hctx, rq, cookie, last);
}

static int blk_mq_try_issue_directly(struct request *rq, blk_qc_t *cookie)
{
    blk_status_t ret;
    int srcu_idx;
    struct blk_mq_hw_ctx *hctx = rq->mq_hctx;
    int success = LABSTOR_MQ_OK;

    might_sleep_if(hctx->flags & BLK_MQ_F_BLOCKING);

    hctx_lock(hctx, &srcu_idx);

    ret = __blk_mq_try_issue_directly(hctx, rq, cookie, true, true); //TODO: May not be last...
    if (ret == BLK_STS_RESOURCE || ret == BLK_STS_DEV_RESOURCE) {
        blk_mq_end_request(rq, ret); //TODO: theoretically requeue
        success = LABSTOR_MQ_DEVICE_BUSY;
    }
    else if (ret != BLK_STS_OK) {
        blk_mq_end_request(rq, ret);
        success = LABSTOR_MQ_NOT_OK;
    }

    hctx_unlock(hctx, srcu_idx);

    return success;
}

/**
 * I/O REQUEST FUNCTIONS
 * */

static void labstor_complete_io(struct labstor_mq_driver_request *rq, int code) {
    rq->header_.code_ = code;
    labstor_queue_pair_CompleteInf(rq->qp_, (struct labstor_request*)rq);
}

static void io_complete(struct bio *bio) {
    int code = 0;
    struct labstor_mq_driver_request *rq;
    if(!bio || IS_ERR(bio)) {
        pr_err("io_complete was passed NULL bio");
        return;
    }
    rq = bio->bi_private;
    if(rq == NULL) {
        pr_err("io_complete request is NULL\n");
        return;
    }
    if(bio->bi_status != BLK_STS_OK) {
        code = bio->bi_status;
        pr_debug("Request did not complete: %d\n", code);
    }
    pr_debug("SUCCESS! Request: (qid=%llu,req_id=%u, code=%d)\n",
             labstor_queue_pair_GetQid(rq->qp_), rq->header_.req_id_, code);
    labstor_complete_io(rq, code);
}

static inline struct page **convert_user_buf(int pid, void *user_buf, size_t length, int *num_pagesp) {
    struct task_struct *task;
    struct page **pages;
    int num_pages;

    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    num_pages = (length % PAGE_SIZE) ? length/PAGE_SIZE + 1 : length/PAGE_SIZE;
    pages = (struct page **)kmem_cache_alloc(page_cache, GFP_KERNEL);
    if(pages == NULL) {
        pr_err("Could not allocate space for the user's pages");
        *num_pagesp = 0;
        return NULL;
    }
    get_user_pages_remote(task, task->mm, (long unsigned)user_buf, num_pages, 0, pages, NULL, NULL);

    *num_pagesp = num_pages;
    return pages;
}

static inline struct bio *create_bio(struct labstor_mq_driver_request *rq, struct block_device *bdev, struct page **pages, int num_pages, size_t sector, int op) {
    struct bio *bio;
    int i;

    bio = bio_alloc(GFP_KERNEL, num_pages);
    if(bio == NULL) {
        pr_err("Could not allocate bio request (%ld bytes)\n", sizeof(struct bio)+num_pages*sizeof(struct bio_vec));
        return NULL;
    }
    bio_set_dev(bio, bdev);
    //bio_set_op_attrs(bio, op, 0);
    bio->bi_opf = op | REQ_NOWAIT;
    bio_set_flag(bio, BIO_USER_MAPPED);
    //bio->bi_flags |= (1U << BIO_USER_MAPPED);
    bio->bi_iter.bi_sector = sector;
    for(i = 0; i < num_pages; ++i) {
        bio_add_page(bio, pages[i], PAGE_SIZE, 0);
    }
    bio->bi_private = rq;
    bio->bi_end_io = &io_complete;
    bio->bi_status = BLK_STS_OK;
    bio->bi_ioprio = 0;
    bio->bi_write_hint = 0;

    bio_integrity_prep(bio);
    return bio;
}

struct request *bio_to_rq(struct request_queue *q, int hctx_idx, struct bio *bio, unsigned int nr_segs, int op) {
    struct request *rq = blk_mq_alloc_request_hctx(q, op, BLK_MQ_REQ_NOWAIT, hctx_idx);
    pr_debug("Request: %p\n", rq);
    if(IS_ERR(rq)) {
        switch (PTR_ERR(rq)) {
            case -EINVAL: {
                pr_err("Could not allocate request on HCTX, invalid flags?");
                return NULL;
            }
            case -EIO: {
                pr_err("Invalid HCTX ID? %d\n", hctx_idx);
                return NULL;
            }
            case -EBUSY: {
                pr_err("HCTX is busy");
                return NULL;
            }
            case -ENODEV: {
                pr_err("HCTX queue dying?\n");
                return NULL;
            }
            default: {
                pr_err("Some other error: %ld\n", PTR_ERR(rq));
                return NULL;
            }
        }
    }
    rq->rq_flags &= ~RQF_IO_STAT;
    rq->bio = rq->biotail = NULL;
    rq->rq_disk = bio->bi_disk;
    rq->__sector = bio->bi_iter.bi_sector;
    rq->write_hint = bio->bi_write_hint;
    blk_rq_bio_prep(rq, bio, nr_segs);
    blk_mq_get_driver_tag(rq);
    return rq;
}

inline void submit_mq_driver_io(struct labstor_queue_pair *qp, struct labstor_mq_driver_request *rq) {
    struct page **pages;
    struct request *dev_rq;
    blk_qc_t cookie;
    int success, num_pages;
    struct block_device *bdev;
    struct request_queue *q;
    struct bio *bio;
    rq->qp_ = qp;
    success = LABSTOR_MQ_OK;

    pr_debug("Received %s request [%p]\n", (rq->header_.op_ == LABSTOR_MQ_DRIVER_WRITE) ? "REQ_OP_WRITE" : "REQ_OP_READ", rq);

    //Convert user's buffer to pages
    pr_debug("Converting user pages: %p %lu\n", rq->user_buf_, rq->buf_size_);
    pages = convert_user_buf(rq->pid_, rq->user_buf_, rq->buf_size_, &num_pages);
    if(pages == NULL) {
        pr_err("Not enough space to allocate user pages\n");
        success = LABSTOR_MQ_CANT_ALLOCATE_PAGES;
        goto err_complete_before;
    }
    //Get block device associated with semantic label
    bdev = labstor_get_bdev(rq->dev_id_);
    pr_debug("BDEV device: %p\n", bdev);
    if(bdev == NULL) {
        pr_err("Invalid block device id: %d\n", rq->dev_id_);
        success = LABSTOR_MQ_INVALID_DEVICE_ID;
        goto err_complete;
    }
    //Get request queue associated with device
    q = bdev->bd_disk->queue;
    //Disable stats
    q->queue_flags &= ~QUEUE_FLAG_STATS;
    //Create bio
    bio = create_bio(rq, bdev, pages, num_pages, rq->sector_, (rq->header_.op_ == LABSTOR_MQ_DRIVER_WRITE) ? REQ_OP_WRITE : REQ_OP_READ);
    pr_debug("Create BIO: %p\n", bio);
    if(bio == NULL) {
        success = LABSTOR_MQ_CANNOT_ALLOCATE_BIO;
        pr_err("Cannot allocate more BIOs\n");
        goto err_complete;
    }
    //Create request to hctx
    dev_rq = bio_to_rq(q, rq->hctx_, bio, num_pages, bio->bi_opf);
    pr_debug("Create HCTX request: %p\n", dev_rq);
    if(dev_rq == NULL) {
        success = LABSTOR_MQ_CANNOT_ALLOCATE_REQUEST;
        goto err_complete;
    }
    //Submit I/O
    pr_debug("Submitting I/O\n");
    success = blk_mq_try_issue_directly(dev_rq, &cookie);
    pr_debug("Driver Submission Status: %d\n", success);
    if(success <  0) {
        goto err_complete;
    }

    //I/O was successfully submitted
    kmem_cache_free(page_cache, pages);
    return;

    //I/O was not successfully submitted (after page cache)
    err_complete:
    kmem_cache_free(page_cache, pages);

    //I/O was not successfully submitted (before page cache)
    err_complete_before:
    labstor_complete_io(rq, success);
}

inline int get_stats(struct labstor_queue_pair *qp, struct labstor_mq_driver_request *rq) {
    //int nr_hw_queues;
    //nr_hw_queues = q->nr_hw_queues;
    return 0;
}

void mq_process_request_fn(struct labstor_queue_pair *qp, struct labstor_request *rq) {
    switch(rq->op_) {
        case LABSTOR_MQ_DRIVER_WRITE:
        case LABSTOR_MQ_DRIVER_READ: {
            submit_mq_driver_io(qp, (struct labstor_mq_driver_request*)rq);
            break;
        }
        default: {
            pr_err("Invalid MQ request: %d\n", rq->op_);
            break;
        }
    }
}

struct labstor_module mq_driver_module = {
        .module_id = MQ_DRIVER_MODULE_ID,
        .runtime_id = MQ_DRIVER_RUNTIME_ID,
        .process_request_fn = mq_process_request_fn,
        .process_request_fn_netlink = NULL
};

/**
 * MY FUNCTIONS
 * */


static int __init init_mq_driver_km(void) {
    register_labstor_module(&mq_driver_module);
    pr_info("MQ driver module started");
    return 0;
}

static void __exit exit_mq_driver_km(void) {
    unregister_labstor_module(&mq_driver_module);
    pr_info("MQ driver module ended");
}

module_init(init_mq_driver_km)
module_exit(exit_mq_driver_km)
