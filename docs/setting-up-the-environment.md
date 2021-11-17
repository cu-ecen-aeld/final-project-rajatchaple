## Setting up buildroot environment for BBB

Follow AESD Assignment 4 instructions for setting up buildroot environment for the final project and adding dropbear package for SSH support.

Buildroot source: https://git.busybox.net/buildroot/ (Branch: 2021.02.x)

Linux kernel version: 4.19.79

NOTE: In shared.sh file specify the correct defconfig file for the target hardware - BeagleBoneBlack.

## Setting SPI hardware module on BBB

If kernel identifies that there are no users for the on-board SPI hardware module, it cuts off the clock for the module. As a result, device drivers cannot read/write to the module registers. So, make sure to add the spi_no_idle patch at [link](https://github.com/cu-ecen-aeld/final-project-SundarKrishnakumar/blob/master/base_external/patches/linux/spi_no_idle.patch) to the buildroot linux kernel source code.

Follow steps 3, 4 and 5 at [link](https://github.com/cu-ecen-aeld/buildroot-assignments-base/wiki/Beagle-Bone-Black-Devicetree-Hardware-Support) to add the patch to the buildroot linux kernel source code and make a clean linux kernel build.







