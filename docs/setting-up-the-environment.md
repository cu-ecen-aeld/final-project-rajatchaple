## Setting up buildroot environment for BBB

Follow AESD Assignment 4 instructions for setting up buildroot environemnt and adding dropbear package for SSH support.

NOTE: In shared.sh file specify the correct defconfig file for the target hardware - BeagleBoneBlack.


## Setting SPI hardware module on BBB

If kernel identifies that there are no users for the on-board SPI hardware module, it cuts off the clock for the module. As a result, device drivers cannot read/write to the module registers. So, make sure to add the patch at [link](https://github.com/cu-ecen-aeld/final-project-SundarKrishnakumar/blob/master/base_external/patches/linux/spi_no_idle.patch) to the buildroot image.

Follow steps 3 and 4 at [link](https://github.com/cu-ecen-aeld/buildroot-assignments-base/wiki/Beagle-Bone-Black-Devicetree-Hardware-Support) to add the patch successfuly.

## Reference

 - How to create a patch? 
    Book:  BeagleBone Cookbook (Page 492)





