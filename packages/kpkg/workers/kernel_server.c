//
// Created by lukemartinlogan on 5/6/21.
//

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/pagewalk.h>

#include <linux/netlink.h>
#include <linux/connector.h>

#include "types.h"
#include "unordered_map.h"

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that manages kernel packages for LabStor");
MODULE_LICENSE("GPL");
MODULE_ALIAS("labstor_kernel_server");

struct sock *nl_sk = NULL;
struct task_struct *workers[nr_cpu_ids];

//Prototypes

void spawn_worker()

void process_request_fn(struct queue_pair *qp, struct worker_manager_request *req) {
    switch(req->op) {
        case SPAWN_WORKER : {

        }
    }
}

struct package {
    .pkg_id = REQEUST_LAYER_PKG_ID,
    .process_request_fn = process_request_fn,
    .request_size = sizeof(struct request_layer_request),
    .get_ops = NULL
} package_;

static int __init init_labstor_worker_spawner(void) {
    register_package(&package_);
    return 0;
}

static void __exit exit_labstor_worker_spawner(void)
{
}

module_init(init_labstor_kernel_server)
module_exit(exit_labstor_kernel_server)
