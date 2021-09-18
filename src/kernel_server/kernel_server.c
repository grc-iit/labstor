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
#include <linux/pagewalk.h>

#include <linux/netlink.h>
#include <linux/connector.h>

#include "kpkg_devkit/module_registrar.h"
#include "kpkg_devkit/types.h"
#include "kpkg_devkit/unordered_map.h"

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that manages kernel modules for LabStor");
MODULE_LICENSE("GPL");
MODULE_ALIAS("labstor_kernel_server");

struct sock *nl_sk = NULL;
struct unordered_map modules;

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
    pr_info("Netlink socket initialized");

    return 0;
}

static void server_loop(struct sk_buff *skb) {
    struct nlmsghdr *nlh;
    struct km_request *km_rq;
    struct labstor_module *pkg;
    int code;
    void *rq;
    int pid;

    nlh=(struct nlmsghdr*)skb->data;
    km_rq = (struct km_request*)nlmsg_data(nlh);
    pid = nlh->nlmsg_pid;

    //Load labstor module
    pkg = get_labstor_module(km_rq->module_id);
    if(pkg == NULL) {
        pr_err("Could not find module %s\n", km_rq->module_id.key);
        code = -1;
        labstor_msg_trusted_server(&code, sizeof(code), pid);
        return;
    }
    rq = (void*)(km_rq + 1);

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

void register_labstor_module(struct labstor_module *pkg) {
    unordered_map_add(&modules, pkg->module_id, pkg);
}
EXPORT_SYMBOL(register_labstor_module);

void unregister_labstor_module(struct labstor_module *pkg) {
    //unordered_map_remove(&modules, pkg->module_id);
}
EXPORT_SYMBOL(unregister_labstor_module);

struct labstor_module *get_labstor_module(struct labstor_id module_id) {
    int id;
    return unordered_map_get(&modules, module_id, &id);
}
EXPORT_SYMBOL(get_labstor_module);

struct labstor_module *get_labstor_module_by_runtime_id(int runtime_id) {
    return unordered_map_get_idx(&modules, runtime_id);
}
EXPORT_SYMBOL(get_labstor_module_by_runtime_id);

static int __init init_labstor_kernel_server(void) {
    unordered_map_init(&modules, 256);
    if(start_server() < 0) {
        unordered_map_free(&modules);
        return -1;
    }
    pr_info("SERVER IS RUNNING!\n");
    return 0;
}

static void __exit exit_labstor_kernel_server(void)
{
    sock_release(nl_sk->sk_socket);
    unordered_map_free(&modules);
}

module_init(init_labstor_kernel_server)
module_exit(exit_labstor_kernel_server)
