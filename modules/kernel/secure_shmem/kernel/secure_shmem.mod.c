#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xb3753869, "module_layout" },
	{ 0x78f44845, "cdev_del" },
	{ 0x71536ee3, "unregister_labstor_module" },
	{ 0xa3036ef8, "cdev_init" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x67f1bb59, "labstor_msg_trusted_server" },
	{ 0x7aa1756e, "kvfree" },
	{ 0x837b7b09, "__dynamic_pr_debug" },
	{ 0xbabacef1, "device_destroy" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x999e8297, "vfree" },
	{ 0x3744cf36, "vmalloc_to_pfn" },
	{ 0x1b44c663, "current_task" },
	{ 0x2db3d320, "mutex_lock_interruptible" },
	{ 0xc5850110, "printk" },
	{ 0xff9c16ee, "device_create" },
	{ 0xe5f1ebe3, "__task_pid_nr_ns" },
	{ 0x7afe113a, "cdev_add" },
	{ 0x599fb41c, "kvmalloc_node" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x3673a548, "remap_pfn_range" },
	{ 0xb356c301, "class_destroy" },
	{ 0x8d62ea07, "__class_create" },
	{ 0xcfc5fa3e, "register_labstor_module" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
};

MODULE_INFO(depends, "labstor_kernel_server");


MODULE_INFO(srcversion, "11DC7D96E548EFB519C312B");
