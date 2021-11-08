/********************************************************************************
 * @file low_level_driver.c
 * @brief Functions and data related to the adc_spi platform driver implementation
 *
 * @author Samuel Ortiz, Juha Yrjola
 * https://android.googlesource.com/kernel/msm/+/android-6.0.1_r0.74/drivers/spi/spi-omap2-mcspi.c
 * Modified by Rajat Chaple and Sundar Krishnakumar
 * @copyright Copyright (c) 2019
 *
 ********************************************************************************/


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/slab.h>


#include "adc_char_driver.h"
#include "low_level_driver.h"

struct omap2_mcspi mcspi;

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
    while (!(__raw_readl(reg) & bit)) {
        if (time_after(jiffies, timeout)) {
            if (!(__raw_readl(reg) & bit))
                return -ETIMEDOUT;
            else
                return 0;
        }
        cpu_relax(); // ARM32: This instruction does not yield. It is just a barrier.
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

int omap2_mcspi_setup_transfer(struct omap2_mcspi *mcspi)
{
	u32 l = 0, div = 0;

	// Variable parameters - Should be set to match ADC chip's requirements
    u32 speed_hz = 400000; // 400KHz
    u8 word_len = 8;

	PDEBUG("\n###### In %s ######\n", __func__);

    speed_hz = min_t(u32, speed_hz, OMAP2_MCSPI_MAX_FREQ);
    div = omap2_mcspi_calc_divisor(speed_hz);

	l = mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCONF0);
	// Select Data line 0 for reception & Data line 1 for transmission
	l &= ~(OMAP2_MCSPI_CHCONF_IS);
	l &= ~(OMAP2_MCSPI_CHCONF_DPE1);
	l |= OMAP2_MCSPI_CHCONF_DPE0;

	// Set the word length as per word_len
	// Clear the bits at 11:7
	l &= ~(OMAP2_MCSPI_CHCONF_WL_MASK);
	// Set the bits at 11:7 for 8-bit word length
	l |= (word_len - 1) << 7;

	// Set the SPIEN state as high during active state
	l |= (OMAP2_MCSPI_CHCONF_EPOL); /* active-low; normal */

	mcspi_write_reg(mcspi, OMAP2_MCSPI_CHCONF0, l);

	/* set clock divisor */

    l &= ~OMAP2_MCSPI_CHCONF_CLKD_MASK; // Clear the bits before setting
	// Set the clock divider
	l |= (div << 2); // Close to speed_hz

	// CPOL is 0. So the lines will be high when idle.
	// Set the PHA so that the data is latched on odd numbered edges
	l &= ~(OMAP2_MCSPI_CHCONF_PHA);

	// Update the chconf0 register with above values
	mcspi_write_reg(mcspi, OMAP2_MCSPI_CHCONF0, l);
	return 0;
}


static void omap2_mcspi_set_enable(struct omap2_mcspi *mcspi, int enable)
{
    u32 l;
    PDEBUG("\n###### In %s ######\n", __func__);

    l = enable ? OMAP2_MCSPI_CHCTRL_EN : 0;
    mcspi_write_reg(mcspi, OMAP2_MCSPI_CHCTRL0, l);
    /* Flash post-writes */
    mcspi_read_reg(mcspi, OMAP2_MCSPI_CHCTRL0);
}



int spi_rw(struct omap2_mcspi *mcspi, uint8_t *buff)
{
	

	void __iomem        *base = mcspi->base;
    void __iomem        *tx_reg;
    void __iomem        *rx_reg;
    void __iomem        *chstat_reg;
    u8 rx;
	u8 tx = *buff;

	if (buff == NULL)
	{
		PDEBUG("buff is NULL\n");
		return -ENOMEM;
	}

	PDEBUG("\n###### In %s ######\n", __func__);

    /* We store the pre-calculated register addresses on stack to speed
     * up the transfer loop. */
    tx_reg = base + OMAP2_MCSPI_TX0;
    rx_reg = base + OMAP2_MCSPI_RX0;
    chstat_reg = base + OMAP2_MCSPI_CHSTAT0;

	// Enable the channel
	omap2_mcspi_set_enable(mcspi, 1);

	// Force the Chipselect
	// NOTE: EPOL=1. So 1 means CS=LOW and 0 means CS=HIGH
	omap2_mcspi_force_cs(mcspi, 1);

	// Wait for TXS bit to be set
	if (mcspi_wait_for_reg_bit(chstat_reg, OMAP2_MCSPI_CHSTAT_TXS) != 0)
	{
		PDEBUG("Transmit timed out\n");
		return -ETIMEDOUT;
	}

	// Do a dummy write to receive data.
	// This dummy write is done to drive the clock.
	// Write into the tx_reg with __raw_writel
	__raw_writel(tx, tx_reg);	


	// Wait for RXS bit to be set
	if (mcspi_wait_for_reg_bit(chstat_reg, OMAP2_MCSPI_CHSTAT_RXS) != 0)
	{
		PDEBUG("Receive timed out\n");
		return -ETIMEDOUT;
	}

	// Read a bytes of data into the rx buffer
	rx = __raw_readl(rx_reg);
	PDEBUG("\nrx = %x\n", rx);


	// Disable the cs force
	// NOTE: EPOL=1. So 1 means CS=LOW and 0 means CS=HIGH
	omap2_mcspi_force_cs(mcspi, 0);

	// Disable the channel
	omap2_mcspi_set_enable(mcspi, 0);

	return rx;
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
	l |= (OMAP2_MCSPI_MODULCTRL_SINGLE);

	// Write the l back to OMAP2_MCSPI_MODULCTRL register
	mcspi_write_reg(mcspi, OMAP2_MCSPI_MODULCTRL, l);
        

}

static int __init omap_spi_init_driver(void)
{
	void __iomem    *spi_pad_base;
	/*
	 * 
	 * NOTE: ioremap() - 
	 * 1. Uses the page tables for virtual and phy address mapping.
	 * 2. The mapping is not page aligned.
	 * 2. Does not caches.
	 * 
    */
    mcspi.base = ioremap(0x48030100, 0x1000);   

    if (IS_ERR(mcspi.base)) {   
        PDEBUG(KERN_ERR "Unable to ioremap\n");
        return PTR_ERR(mcspi.base);
    }

	/* Set up the pin mux for the spi0 pins */
	spi_pad_base = ioremap(0x44E10950, 0x10);
    if (IS_ERR(spi_pad_base)) {
        PDEBUG(KERN_ERR "Unable to ioremap\n");
        return PTR_ERR(spi_pad_base);
    }
	__raw_writel(0x30, spi_pad_base);
	__raw_writel(0x30, spi_pad_base + 0x4);
	__raw_writel(0x10, spi_pad_base + 0x8);
	__raw_writel(0x10, spi_pad_base + 0xc);

	omap2_mcspi_set_master_mode(&mcspi);

    // Initialize the character driver interface
    chrdev_init(&mcspi);

    return 0;
}

static void __exit omap_spi_exit_driver(void)
{
    // De-initialize the character driver interface
    chrdev_exit();
}

module_init(omap_spi_init_driver);
module_exit(omap_spi_exit_driver);

MODULE_AUTHOR("sundar");
MODULE_DESCRIPTION("Low level SPI driver");
MODULE_LICENSE("GPL");