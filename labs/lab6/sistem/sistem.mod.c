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
	{ 0xd5616ba1, "module_layout" },
	{ 0xb942c936, "device_destroy" },
	{ 0x2823aa74, "kernel_kobj" },
	{ 0xae6d880f, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xe20e8040, "class_destroy" },
	{ 0xb6e56e4c, "sysfs_remove_file_ns" },
	{ 0x2f224ca9, "kobject_put" },
	{ 0x5c27fe1b, "sysfs_create_file_ns" },
	{ 0x91a42eec, "kobject_create_and_add" },
	{ 0xc2bdbd62, "device_create" },
	{ 0xf6b02151, "__class_create" },
	{ 0x1180f3bc, "cdev_add" },
	{ 0xc914e16, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x9d669763, "memcpy" },
	{ 0x97255bdf, "strlen" },
	{ 0x91715312, "sprintf" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x20c55ae0, "sscanf" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x7c32d0f0, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "37AE561D893D138250C0B74");
