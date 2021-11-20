#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0xe09a23a6, "module_layout" },
	{ 0x600ab2aa, "cdev_del" },
	{ 0x97823d2e, "cdev_init" },
	{ 0x247907d6, "device_destroy" },
	{ 0xe907b4a7, "__spi_register_driver" },
	{ 0x409bcb62, "mutex_unlock" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x977f511b, "__mutex_init" },
	{ 0xc5850110, "printk" },
	{ 0x51ca4507, "sysfs_create_group" },
	{ 0xd21730d6, "driver_unregister" },
	{ 0x2ab7989d, "mutex_lock" },
	{ 0xdca0b809, "device_create" },
	{ 0x754a7f08, "spi_sync" },
	{ 0x27900d89, "cdev_add" },
	{ 0xa916b694, "strnlen" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0x9f95103f, "class_destroy" },
	{ 0x6fae0ddd, "devm_kmalloc" },
	{ 0xfcff42d, "__class_create" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("spi:adc3002");

MODULE_INFO(srcversion, "D4A39631B77E73B91C4221F");
