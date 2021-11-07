## Client driver

The client driver can also be called as character driver.

File name: adc_char_driver.c

Link: https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/code/mcp3002.c


## Master driver

The master driver can be also called as the low-level driver/platform driver.

File name: low_level_driver.c

Link: https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/code/mcspi.c

## Code organization

![simple-driver-1](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/simple-driver-1.jpg)

The 'Simple driver Part 1' model shown above has these two major problems - 

 - The BeagleBoneBlack's mcSPI register addresses and the SPI pin configurations are part of the Master driver. This make the Master driver's code hardly re-usable across other SPI hardware modules or other channels of the same SPI hardware module. 
 
 **Solution**: The mcSPI register addresses and SPI pin configurations can be moved to a special file called Device Tree (DT). The DT contains data structures which the linux kernel developer uses to describe all the required peripherals of the board. This will be implemented in the next stage of the driver design 'Simple driver Part 2'.

  - The 
