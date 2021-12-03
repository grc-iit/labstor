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
	{ 0x1ecc5356, "unregister_labstor_module" },
	{ 0xd8d99ada, "register_labstor_module" },
	{ 0xeb95027c, "labstor_request_queue_allocate" },
	{ 0x67f1bb59, "labstor_msg_trusted_server" },
	{ 0xc5850110, "printk" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x12f4a8ac, "labstor_request_queue_attach" },
	{ 0x155db414, "find_shmem_region" },
	{ 0xbdfb6dbb, "__fentry__" },
};

MODULE_INFO(depends, "labstor_kpkg_devkit,labstor_kernel_server,secure_shmem");


MODULE_INFO(srcversion, "301A0DB0F7A8767F7215EA4");
