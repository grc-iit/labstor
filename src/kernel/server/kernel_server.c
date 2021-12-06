//
// Created by lukemartinlogan on 5/6/21.
//

#define pr_fmt(fmt) "%s:%s: " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/mm.h>
#include <linux/slab.h>

#include <linux/netlink.h>
#include <linux/connector.h>

#include <labstor/types/data_structures/shmem_request.h>
#include <labstor/kernel/server/module_manager.h>
#include <labstor/kernel/server/kernel_server.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that manages kernel modules for LabStor");
MODULE_LICENSE("GPL");
MODULE_ALIAS("labstor_kernel_server");

struct sock *nl_sk = NULL;
#define NETLINK_USER 31

//Prototypes
static int __init init_labstor_kernel_server(void);
static void __exit exit_labstor_kernel_server(void);
static void server_loop(struct sk_buff *skb);

static int start_server(void) {
    struct netlink_kernel_cfg cfg = {
            .input = server_loop,
    };

    //Create netlink socket
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if(!nl_sk)     {
        pr_alert("Error creating socket.\n");
        return -1;
    }
    pr_debug("Netlink socket initialized");

    return 0;
}

static void server_loop(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    struct labstor_request *rq;
    struct labstor_module *pkg;
    int code;
    int pid;

    nlh=(struct nlmsghdr*)skb->data;
    rq = (struct labstor_request*)nlmsg_data(nlh);
    pid = nlh->nlmsg_pid;

    //Load labstor module
    pr_debug("Received a request: %d\n", rq->ns_id_);
    pkg = get_labstor_module_by_runtime_id(rq->ns_id_);
    if(pkg == NULL) {
        pr_err("Could not find module %d\n", rq->ns_id_);
        code = -1;
        labstor_msg_trusted_server(&code, sizeof(code), pid);
        return;
    }

    //Process command
    if(pkg->process_request_fn_netlink == NULL) {
        pr_err("Module %s does not support access over the netlink socket\n", pkg->module_id.key);
        code = -2;
        labstor_msg_trusted_server(&code, sizeof(code), pid);
        return;
    }
    pkg->process_request_fn_netlink(pid, rq);
}

void labstor_msg_trusted_server(void *serialized_buf, size_t buf_size, int pid) {
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int res = 0;
    void *rq;

    skb_out = nlmsg_new(buf_size, 0);
    if(!skb_out) {
        pr_err("Failed to allocate new skb\n");
        return;
    }
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, buf_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    rq = nlmsg_data(nlh);
    memcpy(rq, serialized_buf, buf_size);

    res=nlmsg_unicast(nl_sk, skb_out, pid);
    if(res<0) {
        pr_err("Error while sending back to user\n");
    }
}
EXPORT_SYMBOL(labstor_msg_trusted_server);

static int __init init_labstor_kernel_server(void) {
    init_labstor_module_manager();
    if(start_server() < 0) {
        free_labstor_module_manager();
        return -1;
    }
    pr_debug("SERVER IS RUNNING!\n");
    return 0;
}

static void __exit exit_labstor_kernel_server(void) {
    free_labstor_module_manager();
    sock_release(nl_sk->sk_socket);
}

module_init(init_labstor_kernel_server)
module_exit(exit_labstor_kernel_server)
