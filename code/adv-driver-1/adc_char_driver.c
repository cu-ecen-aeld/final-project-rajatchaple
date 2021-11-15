/********************************************************************************
 * @file aesd_char_driver..c
 * @brief Functions and data related to the adc_spi char driver implementation
 *
 * @author Andrea Paterniani
 * https://github.com/torvalds/linux/blob/master/drivers/spi/spidev.c
 * Modified by Rajat Chaple
 * @date 2021-14-05
 * @copyright Copyright (c) 2019
 *
 ********************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/platform_data/serial-omap.h>
#include "adc_char_driver.h"

#define FIRST_MINOR 0
#define MINOR_CNT 1

MODULE_AUTHOR("Rajat Chaple");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_LICENSE("GPL v2");

struct spi_data {
	struct spi_device *spi;
	struct spi_message msg;
	struct spi_transfer transfer[1];
	u8 tx_buf[2];
	u8 rx_buf[2];
	struct mutex lock;
	/* Character Driver Files */
	dev_t devt;
	struct cdev cdev;
	struct class *class;
};

/***********************************************************************************
 * adc char driver function for (userspace) open call
 * *********************************************************************************/
static int my_adc_open(struct inode *i, struct file *f)
{
	
	struct spi_data *dev = container_of(i->i_cdev, struct spi_data, cdev);
	if (dev == NULL) {
		PDEBUG("Data is null\n");
		return -1;
	}

	f->private_data = dev;

	return 0;
}

/***********************************************************************************
 * adc char driver function for (userspace) read call
 * *********************************************************************************/
static ssize_t my_adc_read(struct file *f, char __user *buf, size_t len, loff_t *f_pos)
{

	struct spi_data *dev = (struct spi_data *)(f->private_data);
	int ret = -1;
	char adc_buffchar[10] = {0};
	uint8_t buffchar_len = 0;
	unsigned int read_data = 0;
	int err = 0;

	mutex_lock(&dev->lock);

	if (*f_pos == 0) {
		
		dev->tx_buf[0] = 0x68;
		dev->tx_buf[1] = 0x00;
		PDEBUG("tx = 0x%x 0x%x\t", dev->tx_buf[0], dev->tx_buf[1]);

		//Initiating the spi transaction
		ret = spi_sync(dev->spi, &dev->msg);

		if (ret < 0)
		{
			err = ret;
			goto my_read_return;
		}

		// Combine values to get the digital adc value.
		read_data = ((dev->rx_buf[0] & 0x3U) << 8);
		read_data |= (dev->rx_buf[1] & 0xFFU);	//reading 9th and th bit sent from SPI

		// Converting integer to string
		sprintf(adc_buffchar, "%d\n", read_data);
		buffchar_len = strlen(adc_buffchar);

		//Exchanging the rx_buf data with user space 
		if (copy_to_user(buf, adc_buffchar, buffchar_len))
		{
			PDEBUG("User space copy failed\n");
			err =  -EFAULT;
			goto my_read_return;
		}

		*f_pos = buffchar_len; // We transferred n=buffchar_len bytes to user space
		mutex_unlock(&dev->lock);
		return buffchar_len; 
	} 
	else {

		*f_pos = 0;
		err = 0;
		goto my_read_return;

	}

my_read_return:
	mutex_unlock(&dev->lock);
	return err;

}

static struct file_operations driver_fops =
{
	.owner = THIS_MODULE,
	.open = my_adc_open,
	.read = my_adc_read,
};

/***********************************************************************************
 * This function initializes char driver
 * *********************************************************************************/
static int my_adc_probe(struct spi_device *spi)
{
	struct spi_data *data;
	int init_result;
	struct device *dev_ret = NULL;

	PDEBUG("\nInside spi client probe function\n");

	// devm_kzalloc() is resource-managed kzalloc() 
	// If we use devm_kzalloc() then no need to free this memory
	data = devm_kzalloc(&spi->dev, sizeof(struct spi_data), GFP_KERNEL);
	data->spi = spi;

	//Assigning the tx_buf and rx_buf of dummy_data to corresponding fields of transfer DS
	data->transfer[0].tx_buf = &data->tx_buf;
	data->transfer[0].rx_buf = &data->rx_buf;
	data->transfer[0].len = sizeof(data->tx_buf);

	//Initializing the data->msg with transfer structures
	spi_message_init_with_transfers(&data->msg, 
									data->transfer, 
									ARRAY_SIZE(data->transfer));

	spi_set_drvdata(spi, data);

	init_result = alloc_chrdev_region(&data->devt, 0, 1, "spi_client");

	if (0 > init_result)
	{
		PDEBUG("Device Registration failed\n");
		unregister_chrdev_region(data->devt, 1);
		return -1;
	}
	PDEBUG("Major Nr: %d\n", MAJOR(data->devt));

	if ((data->class = class_create(THIS_MODULE, "spiclient")) == NULL)
	{
		PDEBUG("Class creation failed\n" );
		unregister_chrdev_region(data->devt, 1);
		return -1;
	}
	//Createing the device file
	dev_ret = device_create(data->class, NULL, data->devt, NULL, "spi%d", 0);

	if (dev_ret == NULL)
	{
		PDEBUG("Device creation failed\n" );
		class_destroy(data->class);
		unregister_chrdev_region(data->devt, 1);
		return -1;
	}
	
	cdev_init(&data->cdev, &driver_fops);

	//Registering the file ops
	// last argument=1 The number of consecutive minor numbers.
	init_result = cdev_add(&data->cdev, data->devt, 1);


	if (init_result == -1)
	{
		PDEBUG("Device addition failed\n" );
		device_destroy(data->class, data->devt);
		class_destroy(data->class);
		unregister_chrdev_region(data->devt, 1 );
		return -1;
	}

	mutex_init(&data->lock);
	
	return 0;
}

/***********************************************************************************
 * This function is called at exit
 * *********************************************************************************/
static int my_adc_remove(struct spi_device *spi)
{
	struct spi_data *data = spi_get_drvdata(spi);
	// Deleting the device file & the class 
	device_destroy(data->class, data->devt);
	class_destroy(data->class);

    // Unregistering file operations 
	cdev_del(&data->cdev);

    // Unregistering character driver 
	unregister_chrdev_region(data->devt, 1);

	PDEBUG("\nMy spi probe remove function\n");

	return 0;
}

//Populating the id table as per dtb
static const struct spi_device_id my_spi_client_id[] = {
	{ "adc3002", 0 },
	{ }
};

MODULE_DEVICE_TABLE(spi, my_spi_client_id);

//Populating the spi_driver data structure
static struct spi_driver my_spi_client_driver = {

	.driver = {

		.name = "omap_spi_client",
		.owner = THIS_MODULE
	},
	.probe = my_adc_probe,
	.remove = my_adc_remove,
	.id_table = my_spi_client_id
};

module_spi_driver(my_spi_client_driver); // reg. with framework. 

