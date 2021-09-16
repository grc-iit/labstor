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

#include "kpkg_devkit/types.h"
#include "kpkg_devkit/unordered_map.h"

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that manages kernel modules for LabStor");
MODULE_LICENSE("GPL");
MODULE_ALIAS("labstor_kserver");

struct sock *nl_sk = NULL;
struct unordered_map modules;

#define NETLINK_USER 31

//Prototypes
static int __init init_labstor_kernel_server(void);
static void __exit exit_labstor_kernel_server(void);
static void server_loop(struct sk_buff *skb);
static void send_msg_to_usr(int code, int pid);
int worker(void *worker_data);

static int start_server(void)
{
    struct netlink_kernel_cfg cfg = {
            .input = server_loop,
    };

    //Create netlink socket
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if(!nl_sk)
    {
        printk(KERN_ALERT "labstor_kernel_server: Error creating socket.\n");
        return -10;
    }
    printk(KERN_INFO "labstor_kernel_server: Netlink socket initialized");

    return 0;
}

static void server_loop(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    struct km_startup_request *rq;
    int pid;

    nlh=(struct nlmsghdr*)skb->data;
    rq = (struct km_startup_request*)nlmsg_data(nlh);
    pid = nlh->nlmsg_pid;

    //Create worker threads
    /*for(int i = 0; i < nr_cpu_ids; ++i) {
        workers[i] = kthread_run(worker_data, &i, );
        kthread_bind(workers[i], i);
    }*/

    //Create SHMEM queues

    //Send the final message to the user
    send_msg_to_usr(0, pid);

}

static void send_msg_to_usr(int code, int pid)
{
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int res = 0;
    struct km_startup_request *rq;

    skb_out = nlmsg_new(sizeof(struct km_startup_request), 0);
    if(!skb_out) {
        printk(KERN_ERR "time_linux_driver_io_km: Failed to allocate new skb\n");
        return;
    }
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, sizeof(struct km_startup_request), 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    rq = nlmsg_data(nlh);
    rq->code = code;
    res=nlmsg_unicast(nl_sk, skb_out, pid);
    if(res<0) {
        printk(KERN_ERR "time_linux_driver_io_km: Error while sending back to user\n");
    }
}

void register_labstor_module(struct labstor_module *pkg) {
    unordered_map_add(&modules, pkg->module_id, pkg);
}
EXPORT_SYMBOL(register_labstor_module);

void unregister_labstor_module(struct labstor_module *pkg) {
}
EXPORT_SYMBOL(unregister_labstor_module);

struct labstor_module *get_labstor_module(struct labstor_id module_id) {
    //return unordered_map_get(&modules, module_id);
    return NULL;
}
EXPORT_SYMBOL(get_labstor_module);

static int __init init_labstor_kernel_server(void) {
    unordered_map_init(&modules, 256);
    start_server();
    printk(KERN_INFO "labstor_kernel_server: SERVER IS RUNNING!\n");
    return 0;
}

static void __exit exit_labstor_kernel_server(void)
{
    unordered_map_free(&modules);
}

module_init(init_labstor_kernel_server)
module_exit(exit_labstor_kernel_server)
