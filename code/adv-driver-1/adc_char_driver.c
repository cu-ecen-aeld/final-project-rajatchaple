/********************************************************************************
 * @file aesd_char_driver..c
 * @brief Functions and data related to the adc_spi char driver implementation
 *
 * @author Andrea Paterniani
 * https://github.com/torvalds/linux/blob/master/drivers/spi/spidev.c
 * Modified by Rajat Chaple and Sundar Krishnakumar
 * @date 2021-11-05
 * @copyright Copyright (c) 2019
 *
 ********************************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include <linux/uaccess.h>

#include "adc_char_driver.h"
#define FIRST_MINOR 0
#define MINOR_CNT 1

MODULE_AUTHOR("Rajat Chaple");
MODULE_LICENSE("Dual BSD/GPL");




/***********************************************************************************
 * adc char driver function for (userspace) open call
 * *********************************************************************************/
static int my_adc_open(struct inode *i, struct file *f)
{
	PDEBUG("adc_spi open");

	struct omap2_mcspi *mcspi = container_of(i->i_cdev, struct omap2_mcspi, cdev);

	if (omap2_mcspi == NULL) {
		printk("Data is null\n");
		return -1;
	}

	f->private_data = omap2_mcspi;

	return 0;
}

/***********************************************************************************
 * adc char driver function for (userspace) read call
 * *********************************************************************************/
static ssize_t my_adc_read(struct file *f, char __user *buf, size_t len, loff_t *f_pos)
{
	
	unsigned int read_data = 0;
	int rc = 0;
	uint8_t tx_buf[2] = {0x68, 0x00}; // 0x68 <-> 0b0110_1000 	0SCC MN98 where S is start bit, CC is channel select, M is for MSB First bit
									  // 0x00 To receive D7-D0
	uint8_t rx_buf[2] = {0};
	char adc_buffchar[10] = {0};
	uint8_t buffchar_len = 0;

	PDEBUG("Reading data from adc sensor");


	if (*f_pos == 0) {
		
		//Invoking the low level TX/RX function
		rc = spi_rw(mcspi, tx_buf, 2, rx_buf, 2);
		if (rc)
		{
			PDEBUG("error: reading data from adc sensor %d", rc);
			return rc; // Return the return code of spr_rw() call.
			
		}
		
		// Combine values to get the digital adc value.
		read_data = ((rx_buf[0] & 0x3U) << 8);
		read_data |= (rx_buf[1] & 0xFFU);	//reading 9th and th bit sent from SPI

		// Converting integer to string
		sprintf(adc_buffchar, "%d\n", read_data);
		buffchar_len = strlen(adc_buffchar);

		//copying from kernel space to user space
		rc = copy_to_user(buf, &adc_buffchar[0], buffchar_len);	//n=buffchar_len bytes of data needs to be sent back to user
		if (rc)
		{
			PDEBUG("error: copy_to_user %d", rc);
			return -EFAULT;
			
		}
		else
		{
			*f_pos = buffchar_len; // We transferred 1 bytes to user space
			return buffchar_len; // 2 bytes transferred. rx_buff
		}		
		
	} 
	else {
		*f_pos = 0;
		return 0;
	}

	return 0;
}

static struct file_operations driver_fops =
{
	.owner = THIS_MODULE,
	.open = my_adc_open,
	.read = my_adc_read,
};

/***********************************************************************************
 * 
 * *********************************************************************************/
static int my_spi_probe(struct spi_device *spi)
{
	int ret = 0;
	struct device *dev_ret = NULL;
	mcspi = lmcspi;

	// Dynamic allocation method. dev_t is populated by 
    // alloc_chrdev_region()
	if ((ret = alloc_chrdev_region(&mcspi->devt, FIRST_MINOR, MINOR_CNT, "spi_driver")) < 0)
	{
		PDEBUG("Error: Can't get major");
		return ret;
	}

 	cdev_init(&mcspi->cdev, &driver_fops);
	mcspi->cdev.owner = THIS_MODULE;
	ret = cdev_add(&mcspi->cdev, mcspi->devt, MINOR_CNT);	

	if (ret < 0)
	{
		unregister_chrdev_region(mcspi->devt, MINOR_CNT);
		return ret;
	}

	if (IS_ERR(lmcspi->spi_class = class_create(THIS_MODULE, "spi")))
	{
		cdev_del(&mcspi->cdev);
		unregister_chrdev_region(mcspi->devt, MINOR_CNT);
		return PTR_ERR(lmcspi->spi_class);
	}

	dev_ret = device_create(lmcspi->spi_class, NULL, mcspi->devt, NULL, "spi%d", FIRST_MINOR);

	if (IS_ERR(dev_ret))
	{
		class_destroy(mcspi->spi_class);
		cdev_del(&mcspi->cdev);
		unregister_chrdev_region(mcspi->devt, MINOR_CNT);
		return PTR_ERR(dev_ret);
	}

	PDEBUG("ADC SPI driver module initialized");

	return 0;
}

/***********************************************************************************
 * This function is called at exit
 * *********************************************************************************/
static int my_spi_remove(struct spi_device *spi)
{

	struct omap2_mcspi *mcspi = spi_get_drvdata(spi)

	// Deleting the device file & the class
	device_destroy(mcspi->spi_class, mcspi->devt);
	class_destroy(mcspi->spi_class);

    // Unregistering file operations --done
	cdev_del(&mcspi->cdev);

    //Unregistering character driver --done
	unregister_chrdev_region(mcspi->devt, MINOR_CNT);

	PDEBUG("Exiting adc spi module");
	
}


// Populate the id table as per dtb
static const struct spi_device_id my_spi_client_id[] = {
	{ "my_spi_client", 0 },
	{ }
};

MODULE_DEVICE_TABLE(spi, my_spi_client_id);

// Populate the spi_driver data structure
static struct spi_driver my_spi_client_driver = {

	.driver = {

		.name = "omap_spi_client",
		.owner = THIS_MODULE
	},
	.probe = my_spi_probe,
	.remove = my_spi_remove,
	.id_table = my_spi_client_id
};

module_spi_driver(my_spi_client_driver);  

