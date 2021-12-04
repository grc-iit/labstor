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
#include <linux/cdev.h>
#include <linux/device.h>

#include <labstor/constants/constants.h>
#include <labstor/types/data_structures/shmem_queue_pair.h>
#include <labstor/kernel/server/module_manager.h>
#include <labstor/kernel/server/kernel_server.h>
#include <secure_shmem/secure_shmem.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that provides secure memory mapping");
MODULE_LICENSE("GPL");
MODULE_ALIAS("secure_shmem");

atomic_t cur_region_id;
DEFINE_MUTEX(labstor_mmap_mutex);
LIST_HEAD(region_map);
LIST_HEAD(pid_regions);

#define LABSTOR_MMAP_LOCK mutex_lock_interruptible(&labstor_mmap_mutex);
#define LABSTOR_MMAP_UNLOCK mutex_unlock(&labstor_mmap_mutex);

#define N_MINORS 1
struct class *shmem_class;
struct cdev shmem_cdev[N_MINORS];
struct device *shmem_dev_struct;
dev_t shmem_dev;

struct shmem_pid_region {
    struct list_head node;
    int pid;
    struct shmem_region_info *region;
    bool is_mapped;
};


/*LOCKLESS UPDATES*/

struct shmem_region_info *find_shmem_region_nolock(int region_id) {
    struct shmem_region_info *pos;
    list_for_each_entry(pos, &region_map, node) {
        if(pos->region_id == region_id) {
            return pos;
        }
    }
    return NULL;
}

struct shmem_pid_region *find_pid_region_nolock(int pid, int region_id) {
    struct shmem_pid_region *pos;
    list_for_each_entry(pos, &pid_regions, node) {
        if(pos->pid == pid && pos->region->region_id == region_id) {
            return pos;
        }
    }
    return NULL;
}

void* reserve_shmem_nolock(size_t size, bool user_owned, int *new_region_id) {
    struct shmem_region_info *region_info;

    if(size % PAGE_SIZE != 0) {
        pr_warn("Shared memory is not page-aligned\n");
        size = size + PAGE_SIZE - (size % PAGE_SIZE);
    }

    region_info = kvmalloc(sizeof(struct shmem_region_info), GFP_HIGHUSER);
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

bool grant_pid_shmem_nolock(int region_id, int pid) {
    struct shmem_pid_region *pid_region;
    pid_region = kvmalloc(sizeof(struct shmem_pid_region), GFP_HIGHUSER);
    if(pid_region == NULL) {
        pr_err("Could not allocate a permissions element");
        return false;
    }
    pid_region->pid = pid;
    pid_region->region = find_shmem_region_nolock(region_id);
    if(pid_region->region == NULL) {
        pr_err("Could not find region %d\n", region_id);
        vfree(pid_region);
        return false;
    }
    pid_region->is_mapped = false;
    list_add(&pid_region->node, &pid_regions);
    return true;
}

void free_shmem_region_by_id_nolock(int region_id) {
    struct shmem_region_info *region = find_shmem_region_nolock(region_id);
    struct shmem_pid_region *pid_pos, *pid_pos_temp;

    if(region == NULL) {
        pr_warn("Could not locate region %d to be freed\n", region_id);
        return;
    }

    list_del(&region->node);
    vfree(region->vmalloc_ptr);
    kvfree(region);

    list_for_each_entry_safe(pid_pos, pid_pos_temp, &pid_regions, node) {
        if(pid_pos->region->region_id == region_id) {
            list_del(&pid_pos->node);
            kvfree(pid_pos);
        }
    }
}

void free_all_regions_nolock(void) {
    struct shmem_region_info *region_pos, *region_pos_temp;
    struct shmem_pid_region *pid_pos, *pid_pos_temp;
    list_for_each_entry_safe(region_pos, region_pos_temp, &region_map, node) {
        list_del(&region_pos->node);
        vfree(region_pos->vmalloc_ptr);
        kvfree(region_pos);
    }
    list_for_each_entry_safe(pid_pos, pid_pos_temp, &pid_regions, node) {
        list_del(&pid_pos->node);
        kvfree(pid_pos);
    }
}

int labstor_mmap_nolock(struct file *filp, struct vm_area_struct *vma) {
    struct shmem_pid_region *pid_region;
    struct shmem_region_info *region;
    size_t size, i;
    pid_t pid;
    int region_id = (int)filp->private_data;

    //Acquire shared memory region
    pid =  task_tgid_vnr(current);
    size = vma->vm_end - vma->vm_start;
    pid_region = find_pid_region_nolock(pid, region_id);
    if(pid_region == NULL) {
        pr_err("Process %d tried accessing region %d without permission\n", pid, region_id);
        return -1;
    }
    region = pid_region->region;

    //Map region into userspace
    for(i = 0; i < size; i += PAGE_SIZE) {
        if(remap_pfn_range(vma, vma->vm_start + i, vmalloc_to_pfn((char*)region->vmalloc_ptr + i), PAGE_SIZE, vma->vm_page_prot)) {
            pr_err("Could not map region %d into pid %d's address space", region->region_id, pid);
            return -1;
        }
        //SetPageReserved(page);
    }
    return 0;
}


/*LOCKED UPDATES*/

struct shmem_region_info *find_shmem_region_info(int region_id) {
    struct shmem_region_info *pos;
    LABSTOR_MMAP_LOCK
    pos = find_shmem_region_nolock(region_id);
    LABSTOR_MMAP_UNLOCK
    return pos;
}
EXPORT_SYMBOL(find_shmem_region_info);

void *find_shmem_region(int region_id) {
    struct shmem_region_info *pos;
    LABSTOR_MMAP_LOCK
    pos = find_shmem_region_nolock(region_id);
    LABSTOR_MMAP_UNLOCK
    if(pos) { return pos->vmalloc_ptr; }
    return NULL;
}
EXPORT_SYMBOL(find_shmem_region);

struct shmem_pid_region *find_pid_region(int pid, int region_id) {
    struct shmem_pid_region *pid_region;
    LABSTOR_MMAP_LOCK
    pid_region = find_pid_region_nolock(pid, region_id);
    LABSTOR_MMAP_UNLOCK
    return pid_region;
}

void* reserve_shmem(size_t size, bool user_owned, int *new_region_id) {
    void *region;
    LABSTOR_MMAP_LOCK
    region = reserve_shmem_nolock(size, user_owned, new_region_id);
    LABSTOR_MMAP_UNLOCK
    return region;
}

bool grant_pid_shmem(int region_id, int pid) {
    bool granted;
    LABSTOR_MMAP_LOCK
    granted = grant_pid_shmem_nolock(region_id, pid);
    LABSTOR_MMAP_UNLOCK
    return granted;
}

void free_shmem_region_by_id(int region_id) {
    LABSTOR_MMAP_LOCK
    free_shmem_region_by_id_nolock(region_id);
    LABSTOR_MMAP_UNLOCK
}

void free_all_regions(void) {
    LABSTOR_MMAP_LOCK
    free_all_regions_nolock();
    LABSTOR_MMAP_UNLOCK
}

int labstor_mmap(struct file *filp, struct vm_area_struct *vma) {
    int ret;
    LABSTOR_MMAP_LOCK
    ret = labstor_mmap_nolock(filp, vma);
    LABSTOR_MMAP_UNLOCK
    return ret;
}

static int labstor_open(struct inode *inode, struct file *filp) {
    pr_debug("labstor_open\n");
    return 0;
}

static loff_t labstor_lseek(struct file *file, loff_t offset, int orig) {
    file->private_data = (void*)offset;
    return offset;
}



void shmem_process_request_fn_netlink(int pid, struct secure_shmem_request *rq) {
    int code = 0;
    switch(rq->header.op_) {
        case RESERVE_SHMEM: {
            pr_debug("Reserving shared memory of size %lu\n", rq->reserve.size);
            if(reserve_shmem(rq->reserve.size, rq->reserve.user_owned, &code)) {}
            else { code = -1; }
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        case GRANT_PID_SHMEM: {
            pr_debug("Granting %d permission for region %d\n", rq->grant.pid, rq->grant.region_id);
            if(grant_pid_shmem(rq->grant.region_id, rq->grant.pid)) { code = 0; }
            else { code = -1; }
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        case FREE_SHMEM: {
            pr_debug("Freeing region %d\n", rq->free.region_id);
            free_shmem_region_by_id(rq->free.region_id);
            labstor_msg_trusted_server(&code, sizeof(code), pid);
            break;
        }

        default: {
            pr_warn("Invalid request id: %d\n", rq->header.op_);
            code = -1;
            labstor_msg_trusted_server(&code, sizeof(code), pid);
        }
    }
}

struct labstor_module shmem_module = {
    .module_id = SHMEM_ID,
    .runtime_id = SHMEM_MODULE_RUNTIME_ID,
    .process_request_fn = NULL,
    .process_request_fn_netlink = (process_request_fn_netlink_type)shmem_process_request_fn_netlink,
};

struct file_operations shmem_fs = {
    .owner  = THIS_MODULE,
    .open   = labstor_open,
    .mmap   = labstor_mmap,
    .llseek   = labstor_lseek
};

static int __init init_secure_shmem(void) {
    int ret;
    dev_t cur_dev;
    pr_info("Starting the SHMEM module");
    atomic_set(&cur_region_id, 0);

    pr_info("Creating chrdev class");
    shmem_class = class_create(THIS_MODULE, "labstor_shmem_class");
    if(IS_ERR(shmem_class)) {
        pr_err("Could not create shmem class");
        cdev_del(shmem_cdev);
        unregister_chrdev_region(shmem_dev, N_MINORS);
        return -1;
    }

    pr_info("Allocating chrdev region");
    ret = alloc_chrdev_region(&shmem_dev, 0, N_MINORS, "labstor_shmem_driver");
    if(ret < 0) {
        pr_err("Could not open chardev");
        return -1;
    }

    pr_info("Creating chrdev");
    cdev_init(shmem_cdev, &shmem_fs);
    cur_dev = MKDEV(MAJOR(shmem_dev), MINOR(shmem_dev));

    pr_info("Creating device in proc");
    shmem_dev_struct = device_create(shmem_class, NULL, cur_dev, NULL, "labstor_shared_shmem%d", 0);
    if(IS_ERR(shmem_dev_struct)) {
        pr_err("Could not creat device node");
        class_destroy(shmem_class);
        cdev_del(shmem_cdev);
        unregister_chrdev_region(shmem_dev, N_MINORS);
        return -1;
    }

    ret = cdev_add(shmem_cdev, cur_dev, 1);
    if(ret < 0) {
        pr_err("Could not add chardev");
        return -1;
    }

    pr_info("Device is registered!");
    register_labstor_module(&shmem_module);
    return 0;
}

static void __exit exit_secure_shmem(void) {
    device_destroy(shmem_class, shmem_dev);
    class_destroy(shmem_class);
    cdev_del(shmem_cdev);
    free_all_regions();
    unregister_chrdev_region(shmem_dev, N_MINORS);
    unregister_labstor_module(&shmem_module);
}

module_init(init_secure_shmem)
module_exit(exit_secure_shmem)
