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
	{ 0xc79d2779, "module_layout" },
	{ 0x91b79446, "netlink_kernel_release" },
	{ 0x85589edc, "blkdev_put" },
	{ 0xdb12603c, "__netlink_kernel_create" },
	{ 0x3d55a83a, "init_net" },
	{ 0xed4d8176, "blkdev_get_by_path" },
	{ 0xe914e41e, "strcpy" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x84502a47, "blk_status_to_errno" },
	{ 0xacaf5b1e, "bio_put" },
	{ 0x37a0cba, "kfree" },
	{ 0x35465e15, "wait_for_completion_io" },
	{ 0xf2215f74, "blk_finish_plug" },
	{ 0xfea6662c, "submit_bio" },
	{ 0x7a9b37e8, "blk_start_plug" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x26c2e0b5, "kmem_cache_alloc_trace" },
	{ 0x8537dfba, "kmalloc_caches" },
	{ 0xc7b1893e, "bio_add_page" },
	{ 0x7c998c00, "bio_associate_blkg" },
	{ 0x9adb7847, "bio_alloc_bioset" },
	{ 0xbc5e1a42, "fs_bio_set" },
	{ 0x995430ec, "get_user_pages_fast" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x5a921311, "strncmp" },
	{ 0x29361773, "complete" },
	{ 0xb43f9365, "ktime_get" },
	{ 0xc5850110, "printk" },
	{ 0x7515672b, "netlink_unicast" },
	{ 0x60b3bcb6, "__nlmsg_put" },
	{ 0xd9577fc1, "__alloc_skb" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "FCFC746BD5506181903805B");
