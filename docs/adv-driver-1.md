## Platform bus and Platform driver

Every device has its configuration data and resources, which need to be reported to the OS. Enumeration is the process through which the OS can inquire and receive information, such as type of device, manufacturer, device configuration, and all the devices connected to a given bus. Busses like PCI are enumeration-capable; any device sitting on a PCI bus can tell the system what sort of device it is and where its resources are. So the kernel can, at boot time, enumerate the devices available. Enumeration is also called discoverability.


On embedded system platforms, there are I2C, UART, SPI, and other devices. They are a part of SoC and can't be removed, unlike PCI based devices like network cards and graphic cards which can be swapped in or out as needed. Since these devices are permanent, they are not connected to enumeration-capable busses. Such devices are called non-discoverable/platform devices.

But the kernel still needs to provide ways to be told about the hardware that is actually present. That is where the pseudo platform bus comes in. The pseudo platform bus, also called the platform bus, is a kernel virtual bus for devices that are non-discoverable.

The drivers associated with platform devices are called platform drivers.

The platform device information can be fed to the kernel in multiple ways. The latest and recommended approach is writing Device Tree files. These files are compiled into Device Tree Blobs (DTB). At boot-time, u-boot feeds the DTBs to the kernel. The kernel's platform bus logic reads the Device Tree information and initiates the matching platform drivers.



## Device Tree

Patch Link: @@ patch @@

Follow steps 3 and 4 at [link](https://github.com/cu-ecen-aeld/buildroot-assignments-base/wiki/Beagle-Bone-Black-Devicetree-Hardware-Support) to add the patch successfuly.

Parent node entry for the BBB SPI0 hardware module: spi@48030000 | Label: aesd_spi0 | Comaptible string: "ti-omap,spi0"

Child node entry for the MCP3002 ADC IC: spi_mcp3002@0 | Compatible string: "adc3002"

## Code organization

@@ image @@

@@ problem - solution @@

## Probing

@@ image @@


## References
 - Linux Device Drivers Development (Book) by John Madieu.

 - https://lwn.net/Articles/448499/

 - How to create a patch? 
    Book:  BeagleBone Cookbook (Page 492)
