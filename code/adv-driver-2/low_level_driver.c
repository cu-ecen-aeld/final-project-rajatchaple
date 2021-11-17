/********************************************************************************
 * @file low_level_driver.c
 * @brief Functions and data related to the adc_spi platform driver implementation
 *
 * @author Samuel Ortiz, Juha Yrjola
 * https://android.googlesource.com/kernel/msm/+/android-6.0.1_r0.74/drivers/spi/spi-omap2-mcspi.c
 * Modified by Sundar Krishnakumar
 * @copyright Copyright (c) 2019
 *
 ********************************************************************************/


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/pm_runtime.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gcd.h>

#include <linux/spi/spi.h>
#include "low_level_driver.h"

static inline void mcspi_write_reg(struct omap2_mcspi *mcspi,
        int idx, u32 val)
{
    __raw_writel(val, mcspi->base + idx);
}

static inline u32 mcspi_read_reg(struct omap2_mcspi *mcspi, int idx)
{
    return __raw_readl(mcspi->base + idx);
}

static inline void mcspi_write_chconf0(struct omap2_mcspi *mcspi, u32 val)
{
	// Why do we do a dummy read after doing a write?
	// Read: https://devzone.nordicsemi.com/f/nordic-q-a/42437/reading-a-register-right-after-writing-to-it-why
    mcspi_write_reg(mcspi, OMAP2_MCSPI_CHCONF0, val);
    mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCONF0);

}

static int mcspi_wait_for_reg_bit(void __iomem *reg, unsigned long bit)
{
    unsigned long timeout;

    timeout = jiffies + msecs_to_jiffies(1000);
    while (!(__raw_readl(reg) & bit)) 
	{
        if (time_after(jiffies, timeout)) 
		{
            if (!(__raw_readl(reg) & bit))
                return -ETIMEDOUT;
            else
                return 0;
        }

        cpu_relax();
    }
    return 0;
}

static void omap2_mcspi_force_cs(struct omap2_mcspi *mcspi, int cs_active)
{
    u32 l;
    PDEBUG("\n###### In %s ######\n", __func__);

    l = mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCONF0);
    if (cs_active)
        l |= OMAP2_MCSPI_CHCONF_FORCE;
    else
        l &= ~OMAP2_MCSPI_CHCONF_FORCE;

    mcspi_write_chconf0(mcspi, l);
}

static u32 omap2_mcspi_calc_divisor(u32 speed_hz)
{
    u32 div;
    PDEBUG("\n###### In %s ######\n", __func__);

    for (div = 0; div < 15; div++)
        if (speed_hz >= (OMAP2_MCSPI_MAX_FREQ >> div))
            return div;
    return 15;
}

static int omap2_mcspi_setup_transfer(struct spi_device *spi, struct spi_transfer *t)
{
	struct omap2_mcspi *mcspi = spi_master_get_devdata(spi->master);
	u32 l = 0;
	u32 div = 0;

	// If not set in client driver. Bits_per_word field containes default: 8 bits per word.
	// Note: This is not set on the client side. So defaults to 8 bits per word.
	// Ref: https://www.linuxtv.org/downloads/v4l-dvb-internals/device-drivers/API-struct-spi-device.html
    u8 word_len = spi->bits_per_word;
	u32 speed_hz = spi->max_speed_hz;


	PDEBUG("\n###### In %s ######\n", __func__);

	if (t != NULL && t->bits_per_word)
        word_len = t->bits_per_word;
	if (t && t->speed_hz)
        speed_hz = t->speed_hz;


    speed_hz = min_t(u32, speed_hz, OMAP2_MCSPI_MAX_FREQ);
    div = omap2_mcspi_calc_divisor(speed_hz);

	l = mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCONF0); 
	// Select Data line 0 from reception & Data line 1 for transmission
	l &= ~(OMAP2_MCSPI_CHCONF_IS);
	l &= ~(OMAP2_MCSPI_CHCONF_DPE1);
	l |= OMAP2_MCSPI_CHCONF_DPE0;

	// Set the word len as per word_len
	// Clear the word length bits before setting them.
	l &= ~OMAP2_MCSPI_CHCONF_WL_MASK;
	l |= (word_len - 1) << 7;

	// Set the SPIEN state as low during active state
	l |= (OMAP2_MCSPI_CHCONF_EPOL); /* active-low; normal */

	/* set clock divisor */
	l &= ~OMAP2_MCSPI_CHCONF_CLKD_MASK; // Clear the bits before setting
	
	// Set the clock divider
	l |= (div << 2); 

	// Set the PHA so that the data is latched on odd numbered edges
	l &= ~(OMAP2_MCSPI_CHCONF_PHA);

	// Update the chconf0 register
	mcspi_write_reg(mcspi, OMAP2_MCSPI_CHCONF0, l);

	return 0;
}

static int omap2_mcspi_setup(struct spi_device *spi) {
	return omap2_mcspi_setup_transfer(spi, NULL);
}

static void omap2_mcspi_set_enable(struct omap2_mcspi *mcspi, int enable)
{
    u32 l;
    PDEBUG("\n###### In %s ######\n", __func__);

    l = enable ? OMAP2_MCSPI_CHCTRL_EN : 0;

	// Why do we do a dummy read after doing a write?
	// Read: https://devzone.nordicsemi.com/f/nordic-q-a/42437/reading-a-register-right-after-writing-to-it-why
    mcspi_write_reg(mcspi, OMAP2_MCSPI_CHCTRL0, l);
    mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCTRL0);
}

static int spi_transfer_one_message(struct spi_master *master,
		struct spi_message *m)
{
	struct omap2_mcspi *mcspi = spi_master_get_devdata(master);
	struct spi_device *spi = m->spi;
	void __iomem *base = mcspi->base;
    void __iomem *tx_reg;
    void __iomem *rx_reg;
    void __iomem *chstat_reg;
	unsigned int count;
    u8 *rx;
    const u8 *tx;
	struct spi_transfer *t = NULL;
	int status = 0;
	u32 l;

	PDEBUG("\n###### In %s ######\n", __func__);

    /* We store the pre-calculated register addresses on stack to speed
     * up the transfer loop. */
    tx_reg = base + OMAP2_MCSPI_TX0;
    rx_reg = base + OMAP2_MCSPI_RX0;
    chstat_reg = base + OMAP2_MCSPI_CHSTAT0;

	if (list_empty(&m->transfers))
    {
        return -EINVAL;
    }
		
	list_for_each_entry(t, &m->transfers, transfer_list) 
    {
		if (t->speed_hz || t->bits_per_word) 
        {
			status = omap2_mcspi_setup_transfer(spi, t);
			if (status < 0)
				break;
		}

		count = t->len;
		rx = t->rx_buf;
		tx = t->tx_buf;

		if (t->len) 
        {
			while (count) 
            {

				l = mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCONF0);
				l &= ~OMAP2_MCSPI_CHCONF_TRM_MASK; // clear bits before setting them.
				l &= ~OMAP2_MCSPI_CHCONF_TURBO;


				if (t->tx_buf == NULL)
					l |= OMAP2_MCSPI_CHCONF_TRM_RX_ONLY;
				else if (t->rx_buf == NULL)
					l |= OMAP2_MCSPI_CHCONF_TRM_TX_ONLY;	

				mcspi_write_chconf0(mcspi, l);

				// Enable the channel 
				omap2_mcspi_set_enable(mcspi, 1);

				// Force the Chipselect
				// NOTE: EPOL=1. So 1 means CS=LOW and 0 means CS=HIGH
				omap2_mcspi_force_cs(mcspi, 1);

				/* RX_ONLY mode needs dummy data in TX reg */
				if (t->tx_buf == NULL)
                {

                    __raw_writel(0, mcspi->base + OMAP2_MCSPI_TX0);

                }


				if (tx != NULL) 
				{
					// Wait for TXS bit to be set
					if (mcspi_wait_for_reg_bit(chstat_reg, OMAP2_MCSPI_CHSTAT_TXS) != 0)
					{

						status = -ETIMEDOUT;
					}	

					if (status < 0) 
					{
						printk("TXS timed out\n");
						break;
					}

					// Write into the tx_reg with __raw_writel
					__raw_writel(*tx++, mcspi->base + OMAP2_MCSPI_TX0);
				}

				status = 0;

				if (rx != NULL) 
				{
					// Wait for RXS bit to be set
						if (mcspi_wait_for_reg_bit(chstat_reg, OMAP2_MCSPI_CHSTAT_RXS) != 0)
						{
							printk("Receive timed out\n");
							status = -ETIMEDOUT;
						}

					if (status < 0) 
					{
						printk("RXS timed out\n");
						break;
					}

					*rx++ = __raw_readl(rx_reg);
					printk("rx = %x\t", *rx);
				}

				count--;

			}
		}
	}

	// Disable the cs force
	// NOTE: EPOL=1. So 1 means CS=LOW and 0 means CS=HIGH
	omap2_mcspi_force_cs(mcspi, 0);


	// Disable the channel
	omap2_mcspi_set_enable(mcspi, 0);

	m->status = status;
	spi_finalize_current_message(master);
	return 0;
}

static void omap2_mcspi_set_master_mode(struct omap2_mcspi *mcspi)
{
    u32 l;
    PDEBUG("\n###### In %s ######\n", __func__);
	
	mcspi_write_reg(mcspi, OMAP2_MCSPI_WAKEUPENABLE,
            OMAP2_MCSPI_WAKEUPENABLE_WKEN);

    l = mcspi_read_reg(mcspi, OMAP2_MCSPI_MODULCTRL);

	// Set single channel master mode & put the controller in functional mode 
    l &= ~(OMAP2_MCSPI_MODULCTRL_STEST | OMAP2_MCSPI_MODULCTRL_MS);
    l |= OMAP2_MCSPI_MODULCTRL_SINGLE;


    mcspi_write_reg(mcspi, OMAP2_MCSPI_MODULCTRL, l);
}

static int my_mcspi_probe(struct platform_device *pdev)
{
    struct omap2_mcspi  *mcspi;
	struct spi_master *master;
    struct resource *r = NULL;
	int status;
	struct device_node *node = pdev->dev.of_node;

	PDEBUG("\n###### In %s ######\n", __func__);
	
	// Allocate the spi master along with mscpi 
	// The size argument: private data to this spi controller.
	// To get allcated private data, do spi_master_get_devdata(master)
	master = spi_alloc_master(&pdev->dev, sizeof(*mcspi));

	// Register callback handler for setup 
	master->setup = omap2_mcspi_setup;

	// Register callback handler for transfer_one_message 
	master->transfer_one_message = spi_transfer_one_message;


	master->dev.of_node = node;

    platform_set_drvdata(pdev, master);

	mcspi = spi_master_get_devdata(master);
	mcspi->master = master;

	//  Get the base address from platform device
	// Get IORESOURCE_MEM from the pdev supplied in this probe callback function.
    // 0 - Interrupt number, If there is 2nd controller, then number it as 1.
    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);

    if (r == NULL) 
	{
        status = -ENODEV;
		goto free_master;
    }

	// Get the virtual address for the spi0 base address and store it
    // in 'base' field of mcspi. Add the offset of 0x100 to base address (See TRM).
	mcspi->base = ioremap(r->start, resource_size(r));

    if (IS_ERR(mcspi->base)) 
	{
        printk(KERN_ERR "Unable to ioremap\n");
        status = PTR_ERR(mcspi->base);
		goto free_master;
    }

	mcspi->base += 0x100;
	mcspi->dev = &pdev->dev;

	omap2_mcspi_set_master_mode(mcspi);
	status = spi_register_master(master);
	if (status < 0)
         goto free_master;

    return status;

free_master:
	spi_master_put(master);
	return status;
}

static int my_mcspi_remove(struct platform_device *pdev)
{
	struct spi_master *master;
	struct omap2_mcspi *mcspi;

	PDEBUG("\n###### In %s ######\n", __func__);

	master = platform_get_drvdata(pdev);
	mcspi = spi_master_get_devdata(master);
	spi_unregister_master(master);

	return 0;
}

// Populate the id table with compatible string as per dtb
static const struct of_device_id my_mcspi_of_match[] = {
	{ .compatible = "ti-omap,spi0" },
    { },
};

MODULE_DEVICE_TABLE(of, my_mcspi_of_match);

// Populate the platform driver structure
static struct platform_driver my_mcspi_driver = {

    .driver = {

        .name = "omap_spi", // Any name. Was used before dtb was developed in linux
        .owner = THIS_MODULE,
        .of_match_table = my_mcspi_of_match // contains the compatible 'match' string
                                                      
    },
    .probe = my_mcspi_probe, // registering the probe callback
    .remove = my_mcspi_remove


};

static int __init omap_spi_init_driver(void)
{
	// Register the platform driver
	return platform_driver_register(&my_mcspi_driver);
}

static void __exit omap_spi_exit_driver(void)
{
	// De-register the platform driver
	platform_driver_unregister(&my_mcspi_driver);

}
module_init(omap_spi_init_driver);
module_exit(omap_spi_exit_driver);

MODULE_AUTHOR("Sundar Krishnakumar");
MODULE_DESCRIPTION("Low level SPI driver");
MODULE_LICENSE("GPL");