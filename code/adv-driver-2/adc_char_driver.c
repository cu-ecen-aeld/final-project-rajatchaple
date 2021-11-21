/********************************************************************************
 * @file aesd_char_driver.c
 * @brief Functions and data related to the adc_spi char driver implementation
 *
 * @author Andrea Paterniani
 * https://github.com/torvalds/linux/blob/master/drivers/spi/spidev.c
 * Modified by Rajat Chaple
 * Modified by Sundar Krishnakumar (sysfs driver implementation)
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


ssize_t readx_show(struct spi_data *dev, u8 *tx_seq, u8 len, char *buf)
{

	uint16_t read_data = 0;
	char adc_buffchar[10] = {0};
	uint8_t buffchar_len = 0;
	int err = -1;

	if ( !(tx_seq != NULL && len == 2) )
	{
		return -1; // input param validation failed.
	}	

	mutex_lock(&dev->lock);

	dev->tx_buf[0] = tx_seq[0];
	dev->tx_buf[1] = tx_seq[1];

	err = spi_sync(dev->spi, &dev->msg);

	if (err < 0)
	{
		PDEBUG("spi_sync failed\n");
		goto readx_show_return;
	}

	// Combine values to get the digital adc value.
	read_data = ((dev->rx_buf[0] & 0x3U) << 8);
	read_data |= (dev->rx_buf[1] & 0xFFU);	//reading 9th and th bit sent from SPI

	sprintf(adc_buffchar, "%d\n", read_data);
	buffchar_len = strlen(adc_buffchar);

	// Exchanging the rx_buf data with user space 
	if (copy_to_user(buf, adc_buffchar, buffchar_len))
	{
		PDEBUG("User space copy failed\n");
		err =  -EFAULT;
		goto readx_show_return;
	}	

	err = buffchar_len;

readx_show_return:
	mutex_unlock(&dev->lock);
	return err;

}


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
	uint16_t read_data = 0;
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


ssize_t read0_show(struct device *dev, struct device_attribute *attr, char *buf)
{

	u8 tx_seq[2] = {0x68, 0x00};
	struct spi_data *data = dev_get_drvdata(dev);
	PDEBUG("\n###### In %s ######\n", __func__);

	return readx_show(data, tx_seq, 2, buf);

}

ssize_t read1_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	u8 tx_seq[2] = {0x78, 0x00};
	struct spi_data *data = dev_get_drvdata(dev);
	PDEBUG("\n###### In %s ######\n", __func__);

	return readx_show(data, tx_seq, 2, buf);
}

/* Create sysfs atrributes of type struct device_attribute
 * NOTE: Use struct device_attribute and not struct attribute
 * only struct device_attribute has fields to add pointers to show & store methods
 */ 
static DEVICE_ATTR(read0, S_IRUGO, read0_show, NULL); // adc digital output channel0
static DEVICE_ATTR(read1, S_IRUGO, read1_show, NULL); // adc digital output channel1

struct attribute *adc_attrs[] = 
{
	&dev_attr_read0.attr,
	&dev_attr_read1.attr,
	NULL
};

struct attribute_group adc_attr_group =
{
	.attrs = adc_attrs
};
	

int adc_sysfs_create_files(struct device *adc_device)
{
	// Other apporach is to use sysfs_create_file() on each attribute.
	// But using sysfs_create_group is the optimal approach.
	return sysfs_create_group(&adc_device->kobj, &adc_attr_group);
	
}



/***********************************************************************************
 * This function initializes char driver
 * *********************************************************************************/
static int my_adc_probe(struct spi_device *spi)
{
	struct spi_data *data;
	int init_result;
	struct device *dev = NULL;

	PDEBUG("\nInside spi client probe function\n");

	// devm_kzalloc() is resource-managed kzalloc() 
	// If we use devm_kzalloc() then no need to free this memory
	data = devm_kzalloc(&spi->dev, sizeof(struct spi_data), GFP_KERNEL);

	if(!data)
	{
		PDEBUG("Cannot allocate memory\n");
		return -1;
	}

	data->spi = spi;

	// Assigning the tx_buf and rx_buf of dummy_data to corresponding fields of transfer DS
	data->transfer[0].tx_buf = &data->tx_buf;
	data->transfer[0].rx_buf = &data->rx_buf;
	data->transfer[0].len = sizeof(data->tx_buf);

	// Initializing the data->msg with transfer structures
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

	PDEBUG("Major No. %d\n", MAJOR(data->devt));

	if ((data->class = class_create(THIS_MODULE, "spiclient")) == NULL)
	{
		PDEBUG("Class creation failed\n" );
		unregister_chrdev_region(data->devt, 1);
		return -1;
	}
	//Createing the device file
	dev = device_create(data->class, NULL, data->devt, NULL, "spi%d", 0);

	if (dev == NULL)
	{
		PDEBUG("Device creation failed\n" );
		class_destroy(data->class);
		unregister_chrdev_region(data->devt, 1);
		return -1;
	}

	// store the 'struct device' handle returned by device_create, in data->adc_device field
	data->adc_device = dev;

	init_result = adc_sysfs_create_files(data->adc_device);

	if (init_result < 0)
	{
		PDEBUG("Sysfs attribute creation failed\n" );
		device_destroy(data->class, data->devt);
		class_destroy(data->class);
		unregister_chrdev_region(data->devt, 1);
		return -1;
	}

	// save private data pointer in platform device structure
	dev_set_drvdata(data->adc_device, data);	
	
	cdev_init(&data->cdev, &driver_fops);

	// Registering the file ops
	// last argument=1 The number of consecutive minor numbers.
	init_result = cdev_add(&data->cdev, data->devt, 1);


	if (init_result < 0)
	{
		PDEBUG("Device addition failed\n" );
		device_destroy(data->class, data->devt);
		class_destroy(data->class);
		unregister_chrdev_region(data->devt, 1);
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