## Platform bus and Platform driver

Every device has its configuration data and resources, which need to be reported to the OS. Enumeration is the process through which the OS can inquire and receive information, such as type of device, manufacturer, device configuration, and all the devices connected to a given bus. Busses like PCI are enumeration-capable; any device sitting on a PCI bus can tell the system what sort of device it is and where its resources are. So the kernel can, at boot time, enumerate the devices available. Enumeration is also called discoverability.


On embedded system platforms, there are I2C, UART, SPI, and other devices. They are a part of SoC and can't be removed, unlike PCI based devices like network cards and graphic cards which can be swapped in or out as needed. Since these devices are permanent, they are not connected to enumeration-capable busses. Such devices are called non-discoverable/platform devices.

But the kernel still needs to provide ways to be told about the hardware that is actually present. That is where the pseudo platform bus comes in. The pseudo platform bus, also called the platform bus, is a kernel virtual bus for devices that are non-discoverable.

The drivers associated with platform devices are called platform drivers.

The platform device information can be fed to the kernel in multiple ways. The latest and recommended approach is writing Device Tree files. These files are compiled into Device Tree Blobs (DTB). At boot-time, u-boot feeds the DTBs to the kernel. The kernel's platform bus logic reads the Device Tree information and initiates the matching platform drivers.



## Device Tree

Patch Link: https://github.com/cu-ecen-aeld/final-project-SundarKrishnakumar/blob/master/base_external/patches/linux/devtree.patch

Follow steps 3 and 4 at [link](https://github.com/cu-ecen-aeld/buildroot-assignments-base/wiki/Beagle-Bone-Black-Devicetree-Hardware-Support) to add the patch successfuly.

Parent node entry for the BBB SPI0 hardware module: spi@48030000 | Label: aesd_spi0 | Comaptible property: "ti-omap,spi0"

Child node entry for the MCP3002 ADC IC: spi_mcp3002@0 | Name property: "adc3002"

NOTE: The compatible property, in case of multiple strings, each of them are separated by double quotes. The name property, in case of mutiple strings, each of them are separated by commas.

## Code organization

![adv-driver-1](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/adv-driver-1.jpg)

 - The problem of code re-usablity has been solved using Device Tree (DT) and platform bus integration. By moving the hardware register addresses and gpio pin configuration information to the DT, now we have a more generic code that is re-usable across other SPI hardware controllers.

 - The Problem of tight coupling between char driver and low level driver has been solved by integrating Linux SPI framework into the design. The SPI framework acts as a middle-ware and mediates the communication between both the drivers. As a result, now both the drivers are independent of each other; the drivers have to register themselves with the framework in order to use the framework's APIs.

## Probing


![probing_in_action](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/arch.JPG)

The image shows how probing happens in kernel drivers. When there a match between the comaptible strings in the DT and the driver code, the binding is successful and the associated probe() function gets invoked.


## References
 - Linux Device Drivers Development (Book) by John Madieu.

 - https://lwn.net/Articles/448499/

 - How to create a patch? 
    Book:  BeagleBone Cookbook (Page 492)
