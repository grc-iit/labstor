//
// Created by lukemartinlogan on 5/6/21.
//

/*
 * Provides shared memory between the kernel and a process.
 * */

#define pr_fmt(fmt) "%s:%s: " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/fs.h>
#include <linux/mm.h>

#include <linux/types.h>
#include <linux/list.h>

#include <kpkg_devkit/types.h>
#include <kpkg_devkit/module_registrar.h>
#include "secure_shmem.h"

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that provides secure memory mapping");
MODULE_LICENSE("GPL");
MODULE_ALIAS("secure_shmem");

#define SHMEM_ID "SHMEM"

atomic_t cur_region_id;
LIST_HEAD(region_map);
LIST_HEAD(pid_regions);

struct shmem_region_info {
    struct list_head node;
    int region_id;
    size_t  size;
    void *vmalloc_ptr;
    bool user_owned;
};

struct shmem_pid_region {
    struct list_head node;
    int pid;
    struct shmem_region_info *region;
    bool is_mapped;
};

struct shmem_region_info *find_region(int region_id) {
    struct shmem_region_info *pos;
    list_for_each_entry(pos, &pid_regions, node) {
        if(pos->region_id == region_id) {
            return pos;
        }
    }
    return NULL;
}

struct shmem_pid_region *find_unmapped_pid_region(int pid, size_t size) {
    struct shmem_pid_region *pos;
    list_for_each_entry(pos, &pid_regions, node) {
        if(pos->pid == pid && pos->region->size == size) {
            return pos;
        }
    }
    return NULL;
}

void* reserve_shmem(size_t size, bool user_owned, int *new_region_id) {
    struct shmem_region_info *region_info;

    region_info = vmalloc(sizeof(struct shmem_region_info));
    if(region_info == NULL) {
        pr_err("Could not allocate another secure shared memory region element");
        return NULL;
    }
    region_info->region_id = atomic_inc_return(&cur_region_id);
    region_info->size = size;
    region_info->vmalloc_ptr = vmalloc(size);
    if(region_info->vmalloc_ptr == NULL) {
        pr_err("Could not allocate another secure shared memory region of size %lu", size);
        vfree(region_info);
        return NULL;
    }
    region_info->user_owned = user_owned;
    list_add(&region_info->node, &region_map);

    if(new_region_id) {
        *new_region_id = region_info->region_id;
    }
    return region_info->vmalloc_ptr;
}

bool grant_pid_shmem(int region_id, int pid) {
    struct shmem_pid_region *pid_region;
    pid_region = vmalloc(sizeof(struct shmem_pid_region));
    if(pid_region == NULL) {
        pr_err("Could not allocate a permissions element");
        return false;
    }
    pid_region->pid = pid;
    pid_region->region = find_region(region_id);
    if(pid_region->region == NULL) {
        pr_err("Could not find region %d\n", region_id);
        vfree(pid_region);
        return false;
    }
    pid_region->is_mapped = false;
    list_add(&pid_region->node, &pid_regions);
    return true;
}

void free_shmem_region_by_id(int region_id) {
    struct shmem_region_info *region = find_region(region_id);
    if(region == NULL) {
        pr_warn("Could not locate region %d to be freed\n", region_id);
        return;
    }
    list_del(&region->node);
    vfree(region->vmalloc_ptr);
    vfree(region);
}

int labstor_mmap(struct file *filp, struct vm_area_struct *vma) {
    struct shmem_pid_region *pid_region;
    struct shmem_region_info *region;
    size_t size, i;
    pid_t pid;

    //Acquire shared memory region
    pid = current->pid;
    size = vma->vm_end - vma->vm_start;
    pid_region = find_unmapped_pid_region(pid, size);
    if(pid_region == NULL) {
        pr_err("Process %d tried reserving secure shmem of size %lu without permission\n", pid, size);
        return -1;
    }
    region = pid_region->region;

    //Map region into userspace
    for(i = 0; i < size; i += PAGE_SIZE) {
        if(remap_pfn_range(vma, vma->vm_start + i, vmalloc_to_pfn(region->vmalloc_ptr), size, vma->vm_page_prot)) {
            pr_err("Could not map region %d into pid %d's address space", region->region_id, pid);
            return 0;
        }
        //SetPageReserved(page);
    }
    return 0;
}

void shmem_process_request_fn_netlink(int pid, struct shmem_request *rq) {
    int code = 0;
    switch(rq->op) {
        case RESERVE_SHMEM: {
            if(reserve_shmem(rq->reserve.size, rq->reserve.user_owned, &code)) {}
            else { code = -1; }
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        case GRANT_PID_SHMEM: {
            if(grant_pid_shmem(rq->grant.region_id, rq->grant.pid)) { code = 0; }
            else { code = -1; }
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        case FREE_SHMEM: {
            free_shmem_region_by_id(rq->free.region_id);
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }
    }
}

struct labstor_module shmem_module = {
    .module_id = SHMEM_ID,
    .process_request_fn = NULL,
    .process_request_fn_netlink = (process_request_fn_netlink_type)shmem_process_request_fn_netlink,
    .get_ops = NULL
};

struct file_operations shmem_fs = {
    .owner  = THIS_MODULE,
    .open   = nonseekable_open,
    .mmap   = labstor_mmap,
    .llseek = no_llseek,
};

static int __init init_secure_shmem(void) {
    atomic_set(&cur_region_id, 0);
    register_chrdev(100, "labstor_shmem", &shmem_fs);
    register_labstor_module(&shmem_module);
    return 0;
}

static void __exit exit_secure_shmem(void) {
    unregister_chrdev(100, "labstor_shmem");
    unregister_labstor_module(&shmem_module);
}

module_init(init_secure_shmem)
module_exit(exit_secure_shmem)
