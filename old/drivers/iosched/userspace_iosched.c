//
// Created by lukemartinlogan on 5/6/21.
//

/*
 * A kernel module that constructs bio and request objects, and submits them to the underlying drivers.
 * The request gets submitted, but can't read from device afterwards...
 * But the I/O completes?
 * */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>

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

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that interacts with a userspace I/O scheduler for handling I/O");
MODULE_LICENSE("GPL");
MODULE_ALIAS("userspace-ioshched");

#define DD_ATTR(name) \
	__ATTR(name, 0644, deadline_##name##_show, deadline_##name##_store)

static struct elv_fs_entry deadline_attrs[] = {
        DD_ATTR(read_expire),
        DD_ATTR(write_expire),
        DD_ATTR(writes_starved),
        DD_ATTR(front_merges),
        DD_ATTR(fifo_batch),
        __ATTR_NULL
};

static struct elevator_type userspace_iosched = {
        .ops = {
                .insert_requests	= usp_insert_requests,
                .dispatch_request	= usp_dispatch_request,
                .prepare_request	= usp_prepare_request,
                .finish_request		= usp_finish_request,
                .next_request		= NULL,
                .former_request		= NULL,
                .bio_merge		= NULL,
                .request_merge		= NULL,
                .requests_merged	= NULL,
                .request_merged		= NULL,
                .has_work		= usp_has_work,
                .init_sched		= usp_init_queue,
                .exit_sched		= usp_exit_queue,
        },
        .elevator_attrs = deadline_attrs,
        .elevator_name = "userspace-iosched",
        .elevator_alias = "userspace",
        .elevator_features = ELEVATOR_F_ZBD_SEQ_WRITE,
        .elevator_owner = THIS_MODULE,
};

//Prototypes
static int __init init_userspace_iosched(void);
static void __exit exit_userspace_iosched(void);

static void usp_insert_requests(struct blk_mq_hw_ctx *hctx,
                               struct list_head *list, bool at_head)
{
    struct request_queue *q = hctx->queue;
    struct deadline_data *usp = q->elevator->elevator_data;

    spin_lock(&usp->lock);
    while (!list_empty(list)) {
        struct request *rq;

        rq = list_first_entry(list, struct request, queuelist);
        list_del_init(&rq->queuelist);
        usp_insert_request(hctx, rq, at_head);
    }
    spin_unlock(&usp->lock);
}

static struct request *usp_dispatch_request(struct blk_mq_hw_ctx *hctx)
{
    struct deadline_data *usp = hctx->queue->elevator->elevator_data;
    struct request *rq;

    spin_lock(&usp->lock);
    rq = __usp_dispatch_request(usp);
    spin_unlock(&usp->lock);

    return rq;
}

static void usp_prepare_request(struct request *rq, struct bio *bio)
{
}

static void usp_finish_request(struct request *rq)
{
    struct request_queue *q = rq->q;

    if (blk_queue_is_zoned(q)) {
        struct deadline_data *usp = q->elevator->elevator_data;
        unsigned long flags;

        spin_lock_irqsave(&usp->zone_lock, flags);
        blk_req_zone_write_unlock(rq);
        if (!list_empty(&usp->fifo_list[WRITE]))
            blk_mq_sched_mark_restart_hctx(rq->mq_hctx);
        spin_unlock_irqrestore(&usp->zone_lock, flags);
    }
}

static bool usp_has_work(struct blk_mq_hw_ctx *hctx)
{
    struct deadline_data *usp = hctx->queue->elevator->elevator_data;

    return !list_empty_careful(&usp->dispatch) ||
           !list_empty_careful(&usp->fifo_list[0]) ||
           !list_empty_careful(&usp->fifo_list[1]);
}

static int usp_init_queue(struct request_queue *q, struct elevator_type *e)
{
    struct deadline_data *usp;
    struct elevator_queue *eq;

    eq = elevator_alloc(q, e);
    if (!eq)
        return -ENOMEM;

    usp = kzalloc_node(sizeof(*usp), GFP_KERNEL, q->node);
    if (!usp) {
        kobject_put(&eq->kobj);
        return -ENOMEM;
    }
    eq->elevator_data = usp;

    INIT_LIST_HEAD(&usp->fifo_list[READ]);
    INIT_LIST_HEAD(&usp->fifo_list[WRITE]);
    usp->sort_list[READ] = RB_ROOT;
    usp->sort_list[WRITE] = RB_ROOT;
    usp->fifo_expire[READ] = read_expire;
    usp->fifo_expire[WRITE] = write_expire;
    usp->writes_starved = writes_starved;
    usp->front_merges = 1;
    usp->fifo_batch = fifo_batch;
    spin_lock_init(&usp->lock);
    spin_lock_init(&usp->zone_lock);
    INIT_LIST_HEAD(&usp->dispatch);

    q->elevator = eq;
    return 0;
}

static void usp_exit_queue(struct elevator_queue *e)
{
    struct deadline_data *usp = e->elevator_data;

    BUG_ON(!list_empty(&usp->fifo_list[READ]));
    BUG_ON(!list_empty(&usp->fifo_list[WRITE]));

    kfree(usp);
}

//Registration and finalization
static void __init init_userspace_iosched(void) {
    elv_register(&userspace_iosched);
}

static void __exit exit_userspace_iosched(void) {
    elv_unregister(&userspace_iosched);
}

module_init(init_userspace_iosched)
module_exit(exit_userspace_iosched)
