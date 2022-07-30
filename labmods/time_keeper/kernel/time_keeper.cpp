
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
#include <labstor/kernel/server/module_manager.h>
#include <labstor/kernel/server/kernel_server.h>

#include <time_keeper/kernel/ipc_test_kernel.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A non-interruptable process for managing time");
MODULE_LICENSE("GPL");
MODULE_ALIAS_FS("time_keeper");

inline void start_time_keeper(struct labstor_queue_pair *qp, struct labstor_time_keeper_request *kern_rq) {
    int shmem_region_id;
    shmem_region_id = kern_rq->region_id;
    shmem_region = labstor_find_shmem_region(shmem_region_id);
    region = shmem_region;
}

void time_keeper_process_request_fn(struct labstor_queue_pair *qp, struct labstor_request *rq) {
    switch(rq->op_) {
        case LABSTOR_START_IPC_TEST: {
            complete_test(qp, (struct labstor_time_keeper_request *)rq);
            break;
        }
    }
}

struct labstor_module ipc_test_pkg = {
        .module_id = IPC_TEST_MODULE_ID,
        .runtime_id = IPC_TEST_MODULE_RUNTIME_ID,
        .process_request_fn = ipc_test_process_request_fn,
};

/**
 * MY FUNCTIONS
 * */


static int __init init_time_keeper(void) {
    register_labstor_module(&ipc_test_pkg);
    pr_info("TimeKeeper has started");
    return 0;
}

static void __exit exit_time_keeper(void) {
    unregister_labstor_module(&ipc_test_pkg);
    pr_info("TimeKeeper has ended");
}

module_init(init_ipc_test)
module_exit(exit_ipc_test)