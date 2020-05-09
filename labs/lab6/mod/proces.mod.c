#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
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
__used
__attribute__((section("__versions"))) = {
	{ 0xe1f81815, "module_layout" },
	{ 0x5b9b5c0c, "cdev_del" },
	{ 0xd199ae7c, "device_destroy" },
	{ 0xbbab2664, "remove_proc_entry" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x83b8b576, "proc_create" },
	{ 0x93ff194d, "class_destroy" },
	{ 0x440a377a, "device_create" },
	{ 0xf33a0af0, "__class_create" },
	{ 0x9b63ce97, "cdev_add" },
	{ 0x216eb8c, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xf4fa543b, "arm_copy_to_user" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x5f754e5a, "memset" },
	{ 0x1e047854, "warn_slowpath_fmt" },
	{ 0x28cc25db, "arm_copy_from_user" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x7c32d0f0, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "2BFFD0E95E628AA2032C5EE");
