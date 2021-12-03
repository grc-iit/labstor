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
	{ 0xc958a62d, "sock_release" },
	{ 0xc5850110, "printk" },
	{ 0xc468b012, "netlink_unicast" },
	{ 0x1c66fb49, "init_net" },
	{ 0xb601be4c, "__x86_indirect_thunk_rdx" },
	{ 0x865dc1d3, "__alloc_skb" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xafc62920, "__netlink_kernel_create" },
	{ 0x69acdf38, "memcpy" },
	{ 0x4493bb20, "__nlmsg_put" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "E56FCFD655F7390D673565E");
