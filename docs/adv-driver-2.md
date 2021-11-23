
If continuous collection of data from a single channel of a ADC chip is needed /dev interface from 'Advanced driver Part-1' can be used. If continuous collection of data needs to be done from all the channels available, then /sysfs interface can be used. This section 'Advanced driver Part-2' implements sysfs attributes, each representing the channel of the ADC chip. (NOTE: MCP3002 has 2 channels.)

## What is sysfs?

Sysfs is a virtual file system that provides:
 - Representation of the kobject hierarchy of the linux kernl. <br>
 NOTE: kobjects are a fundamental building block of Linux device and driver heirarchy.
 - Attributes to help user space application to interact with devices using 'classes'.


## Client driver

The client driver can also be called as character driver.

File name: adc_char_driver.c

https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/code/adv-driver-2/adc_char_driver.c


## Master driver

The master driver can be also called as the low-level driver/platform driver.

File name: low_level_driver.c

https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/code/adv-driver-2/low_level_driver.c



## Code organization

![adv-driver-2](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/adv-driver-2.jpg)

NOTE: low level driver implementation remains the as 'Advanced driver Part-1'. Syfs driver is imeplemented as part of the character driver. 

MCP3002 has two ADC channels - CHNL0 and CHNL1 which which can be read as a regular file from the user space using the respective sysfs attributes - read0 and read1. The character driver implements the functionality of these attributes.
