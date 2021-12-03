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
	{ 0x71536ee3, "unregister_labstor_module" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x67f1bb59, "labstor_msg_trusted_server" },
	{ 0x999e8297, "vfree" },
	{ 0xe8bc695c, "kthread_create_on_node" },
	{ 0xec7e781, "kthread_bind" },
	{ 0xc5850110, "printk" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x2ea2c95c, "__x86_indirect_thunk_rax" },
	{ 0x64c17a3f, "wake_up_process" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xf4130344, "get_labstor_module_by_runtime_id" },
	{ 0xcfc5fa3e, "register_labstor_module" },
};

MODULE_INFO(depends, "labstor_kernel_server");


MODULE_INFO(srcversion, "ADBDCA7796DBFCB2E1ABDB4");
