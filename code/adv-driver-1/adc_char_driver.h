#ifndef ADC_SPI_CHAR_H
#define ADC_SPI_CHAR_H

#include <linux/spi/spi.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h> // for sprintf()
#include <linux/string.h>

#define ADC_DEBUG 1 //Remove comment on this line to enable debug

#undef PDEBUG /* undef it, just in case */
#ifdef ADC_DEBUG
#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) printk(KERN_DEBUG "adc_spi: " fmt, ##args)
#else
/* This one for user space */
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ##args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#endif  //ADC_SPI_CHAR_H