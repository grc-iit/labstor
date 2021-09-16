//
// Created by lukemartinlogan on 5/6/21.
//

/*
 * A kernel module that maps memory into different address spaces.
 * This module can only communicate with the trusted server process.
 * */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/kobject.h>

#include <kpkg_devkit/request_queue.h>
#include <kpkg_devkit/unordered_map.h>

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that provides secure memory mapping");
MODULE_LICENSE("GPL");
MODULE_ALIAS("secure_shmem");

#define SHMEM_ID "SHMEM"

void map_user_to_user(pid_t src_pid, void *src_addr, pid_t dst_pid, void *dst_addr, size_t num_pages) {
    struct task_struct *src_task;
    struct task_struct *dst_task;
    struct vm_area_struct *dst_vma;
    unsigned long pfn;
    unsigned long size;
    pgprot_t prot;
    struct page page;
    struct page page_ptr *page_arr;
    int i;

    src_task = get_pid_task(find_vpid(src_pid), PIDTYPE_PID);
    dst_task = get_pid_task(find_vpid(dst_pid), PIDTYPE_PID);
    dst_vma = dst_task->mm->mmap;
    page_arr = &page;

    for(i = 0; i < num_pages*PAGE_SIZE; i += PAGE_SIZE) {
        get_user_pages(src_task, src_task->mm, i + (unsigned long)src_addr, 1, true, false, &page_arr, NULL);
        remap_pfn_range(dst_vma, i + (unsigned long)dst_addr, page_to_phys(page), size, prot);
        SetPageReserved(page);
    }
}

int reserve_shmem(size_t size) {
}
EXPORT_SYMBOL(reserve_shmem);

void share_memory(int region_id, int pid) {
}
EXPORT_SYMBOL(share_memory)

void labstor_mmap(struct file *filp, struct vm_area_struct *vma) {
    struct vm_area_struct *vma;
    unsigned long len = vma->vm_end - vma->vm_start;
    int ret;

    for(i = 0; i < num_pages*PAGE_SIZE; i += PAGE_SIZE) {
        remap_pfn_range(vma, vma->vm_start + i, page_to_phys(page), size, prot);
        SetPageReserved(page);
    }
}

void process_request_fn() {
}

struct labstor_module {
    .module_id = SHMEM_ID,
    .process_request_fn = process_request_fn,
    .request_size = sizeof(struct request_layer_request),
    .get_ops = NULL
} request_layer_pkg;

static int __init init_secure_shmem(void)
{
}

static void __exit exit_secure_shmem(void)
{
}

module_init(init_request_layer_km)
module_exit(exit_request_layer_km)
