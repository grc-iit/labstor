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
	{ 0xd9b7119a, "module_layout" },
	{ 0xdf772057, "bio_integrity_prep" },
	{ 0x6e53c6ba, "bio_add_page" },
	{ 0x8f9caa21, "bio_associate_blkg" },
	{ 0x8a527097, "bio_alloc_bioset" },
	{ 0x968b77fc, "fs_bio_set" },
	{ 0x390e80de, "blkdev_get_by_path" },
	{ 0x746c1d80, "vmalloc_to_page" },
	{ 0x91b6ee8a, "kmem_cache_alloc_trace" },
	{ 0x67ee46da, "kmalloc_caches" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0xc5850110, "printk" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "FCDA92B403EA78491F2E294");
