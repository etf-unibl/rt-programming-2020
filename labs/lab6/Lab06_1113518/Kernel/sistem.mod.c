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
	{ 0xd199ae7c, "device_destroy" },
	{ 0x163bcc35, "kernel_kobj" },
	{ 0x5b9b5c0c, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x93ff194d, "class_destroy" },
	{ 0x84a82ee2, "sysfs_remove_file_ns" },
	{ 0x2f224ca9, "kobject_put" },
	{ 0xbd9f9f6a, "sysfs_create_file_ns" },
	{ 0x91a42eec, "kobject_create_and_add" },
	{ 0x440a377a, "device_create" },
	{ 0xf33a0af0, "__class_create" },
	{ 0x9b63ce97, "cdev_add" },
	{ 0x216eb8c, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x2257dfe8, "try_module_get" },
	{ 0x5ab46301, "module_put" },
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


MODULE_INFO(srcversion, "04084C5174B3BFA9738BC62");
