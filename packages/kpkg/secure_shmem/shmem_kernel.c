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

MODULE_AUTHOR("Luke Logan <llogan@hawk.iit.edu>");
MODULE_DESCRIPTION("A kernel module that provides secure memory mapping");
MODULE_LICENSE("GPL");
MODULE_ALIAS("secure_shmem");

int trusted_server_pid;
MODULE_PARM(trusted_server_pid, "pid");

void map_pages_to_process(pid_t src_pid, void *src_addr, pid_t dst_pid, void *dst_addr, size_t num_pages) {
    struct task_struct *src_task;
    struct task_struct *dst_task;
    struct vm_area_struct *dst_vma;
    unsigned long pfn;
    unsigned long size;
    pgprot_t prot;
    struct page page;
    struct page page_ptr *page_arr;
    size_t i;

    src_task = get_pid_task(find_vpid(src_pid), PIDTYPE_PID);
    dst_task = get_pid_task(find_vpid(dst_pid), PIDTYPE_PID);
    dst_vma = dst_task->mm->mmap;

    for(i = 0; i < num_pages; ++i) {
        get_user_pages(src_task, src_task->mm, (unsigned long)src_addr, 1, true, false, &page_arr, NULL);
        remap_pfn_range(dst_vma, (unsigned long)dst_addr, page_to_phys(page), size, prot);
    }
}

static int __init init_secure_shmem(void)
{
}

static void __exit exit_secure_shmem(void)
{
}

module_init(init_request_layer_km)
module_exit(exit_request_layer_km)
