
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

## Generic python GUI
File name: https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/code/gui/gen_python_gui.py
This generic GUI is quite useful where display cannot be run or integrated on the embedded platform. The GUI has features to read sysfs driver data and display in intuitive form. This utility is developed keeping generic aaproach in mind as opposed to tightly bound application specific gui application developed using tkinter https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/code/gui/adc_gui.py

![gen_python_gui](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/gui.jpg)


Application interacts with Beaglebone Black over serial using USB to TTL converter. Serial connections are as below. Blow diagram shows communication with MCP3002 chip.

![Connection-diagram](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/connection_diagram.JPG)


The developed utility has been developed using pyqt platform and uses multithreading to schedule tasks. 
Utiliy also includes mechanism to select COM port and send/receive data over selected serial port.
![GUI](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/gui.gif)

This gui assumes that sysfs drivers are implemented. To use this application for I2C,
1) Add I2C command inside init_continuous_write() similar to SPI0_CH0
2) edit read_from_port() for the required data coversion 

Note: This application has been tested on Windows 10.
