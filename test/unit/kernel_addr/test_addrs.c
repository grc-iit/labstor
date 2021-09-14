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
#include <linux/kobject.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/pagewalk.h>

#include <linux/netlink.h>
#include <linux/connector.h>
#include "kernel_server.h"

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that performs I/O with underlying storage devices");
MODULE_LICENSE("GPL");
MODULE_ALIAS("test_addrs");

struct sock *nl_sk = NULL;

//Prototypes
static int __init init_test_addrs(void);
static void __exit exit_test_addrs(void);
static void server_loop(struct sk_buff *skb);
void send_kaddr(int pid);

static int start_server(void)
{
    struct netlink_kernel_cfg cfg = {
            .input = server_loop,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if(!nl_sk)
    {
        printk(KERN_ALERT "test_addrs: Error creating socket.\n");
        return -10;
    }
    printk(KERN_INFO "test_addrs: Netlink socket initialized");
    return 0;
}

static void server_loop(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    struct km_request *rq;
    int pid;

    nlh=(struct nlmsghdr*)skb->data;
    rq = (struct km_request*)nlmsg_data(nlh);
    pid = nlh->nlmsg_pid; /*pid of sending process */

    switch(rq->code) {
        case 1: {
            send_kaddr(pid);
            break;
        }
    }
}

static void send_msg_to_usr(int code, void *data, void *fun, int pid)
{
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int res = 0;
    struct km_request *rq;

    skb_out = nlmsg_new(sizeof(struct km_request), 0);
    if(!skb_out) {
        printk(KERN_ERR "time_linux_driver_io_km: Failed to allocate new skb\n");
        return;
    }
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, sizeof(struct km_request), 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    rq = nlmsg_data(nlh);
    rq->code = code;
    rq->data = data;
    rq->fun = fun;
    res=nlmsg_unicast(nl_sk, skb_out, pid);
    if(res<0) {
        printk(KERN_ERR "time_linux_driver_io_km: Error while sending back to user\n");
    }
}

void myfun(int x) {
    printk("test_addrs: X: %d\n", x);
}

int disable_pte_supervisor(pte_t *pte, unsigned long addr, unsigned long next, struct mm_walk *walk) {
    pte_mkwrite(*pte);
    return 0;
}

void send_kaddr(int pid) {
    int data;
    struct task_struct *task;
    unsigned long src_addr = 0xffff800000000000;
    unsigned long dst_addr = 0xffffffffffffffff;
    struct mm_walk_ops ops = {
        .pte_entry = disable_pte_supervisor
    };
    struct mm_struct *mm;

    data = 5;
    printk("test_addrs: myfun=%p\n", myfun);
    printk("test_addrs: data=%p\n", &data);

    //Get mm struct for user
    task = get_pid_task(find_vpid(pid), PIDTYPE_PID);
    mm = task->mm;

    //Walk page range and disble supervisor bit
    printk("test_addrs: starting walk\n");
    walk_page_range(mm, src_addr, dst_addr, &ops, NULL);
    printk("test_addrs: ending walk\n");

    send_msg_to_usr(0, (void*)&data, (void*)myfun, pid);
}

static int __init init_test_addrs(void)
{
    start_server();
    return 0;
}

static void __exit exit_test_addrs(void)
{
}

//Virt mem management: https://unix.stackexchange.com/questions/509607/how-a-64-bit-process-virtual-address-space-is-divided-in-linux
//Page table protections: https://www.halolinux.us/kernel-reference/page-table-handling.html

module_init(init_test_addrs)
module_exit(exit_test_addrs)
