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

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that manages kernel packages for LabStor");
MODULE_LICENSE("GPL");
MODULE_ALIAS("labstor_kernel_server");

struct sock *nl_sk = NULL;

//Prototypes
static int __init init_labstor_kernel_server(void);
static void __exit exit_labstor_kernel_server(void);
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
        printk(KERN_ALERT "labstor_kernel_server: Error creating socket.\n");
        return -10;
    }
    printk(KERN_INFO "labstor_kernel_server: Netlink socket initialized");
    return 0;
}

static void server_loop(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;
    struct km_request *rq;
    int pid;

    nlh=(struct nlmsghdr*)skb->data;
    rq = (struct km_request*)nlmsg_data(nlh);
    pid = nlh->nlmsg_pid; /*pid of sending process? */

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

void send_kaddr(int pid) {
}

static int __init init_labstor_kernel_server(void)
{
    start_server();
    return 0;
}

static void __exit exit_labstor_kernel_server(void)
{
}

module_init(init_labstor_kernel_server)
module_exit(exit_labstor_kernel_server)
