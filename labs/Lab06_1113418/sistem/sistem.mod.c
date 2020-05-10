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
	{ 0x8921f06d, "module_layout" },
	{ 0x92e0a3bd, "device_destroy" },
	{ 0xdf3ebb55, "kernel_kobj" },
	{ 0xe1c891a1, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x326abfc4, "class_destroy" },
	{ 0x3558043f, "sysfs_remove_file_ns" },
	{ 0x2f224ca9, "kobject_put" },
	{ 0xc6fb5b7, "sysfs_create_file_ns" },
	{ 0x91a42eec, "kobject_create_and_add" },
	{ 0x628b6f1b, "device_create" },
	{ 0xe987e507, "__class_create" },
	{ 0xb74b1cd0, "cdev_add" },
	{ 0x7d65a186, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x20c55ae0, "sscanf" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0x7c32d0f0, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "5842D7CFC78388EE225FCBD");
