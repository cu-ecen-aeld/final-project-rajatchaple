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
	{ 0xcbc4e676, "spi_register_controller" },
	{ 0xd787a335, "__spi_alloc_controller" },
	{ 0x24a68f66, "__platform_driver_register" },
	{ 0x15ba50a6, "jiffies" },
	{ 0xc5850110, "printk" },
	{ 0xde80cd09, "ioremap" },
	{ 0x7cc9c3d6, "spi_unregister_controller" },
	{ 0x9fa11cef, "platform_get_resource" },
	{ 0x95da34ff, "put_device" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x1cd6dfbd, "platform_driver_unregister" },
	{ 0x590fcc81, "spi_finalize_current_message" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cti-omap,spi0");
MODULE_ALIAS("of:N*T*Cti-omap,spi0C*");

MODULE_INFO(srcversion, "B66498393D04947DAED72D0");
