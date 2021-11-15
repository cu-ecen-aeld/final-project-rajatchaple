## Client driver

The client driver can also be called as character driver.

File name: adc_char_driver.c

https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/code/simple-driver/adc_char_driver.c


## Master driver

The master driver can be also called as the low-level driver/platform driver.

File name: low_level_driver.c

https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/code/simple-driver/low_level_driver.c


## Code organization

![simple-driver](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/simple-driver.jpg)

The 'Simple driver' model shown above has these two major problems - 

 - The BeagleBoneBlack's mcSPI register addresses and the SPI pin configurations are part of the Master driver. This make the Master driver's code hardly re-usable across other SPI hardware modules or other channels of the same SPI hardware module. 
 
 **Solution**: The mcSPI register addresses and SPI pin configurations can be moved to a special file called Device Tree (DT). The DT contains data structures which the linux kernel developer uses to describe all the required peripherals of the board. The kernel parses the Device Tree and initiates the associated low level driver with the help of a virtual bus called 'platform bus'. The bus uses a method called probing to initiate the driver. The low level driver must register itself with this bus to take part in probing. This will be implemented in the next stage of the driver design 'Advanced driver Part-1'.

  - The character driver and low level driver are tightly coupled. Any change on one code will affect the other. 
  
 **Solution**: The tight coupling between the character driver and the low level driver can be broken by intrdoucing a middle layer called the 'Linux SPI framework'. The SPI char driver and the low level driver register themselves to the framework. The framework mediates the communication between these two drivers. This will be implemented in the next stage of the driver design 'Advanced driver Part-1'.
