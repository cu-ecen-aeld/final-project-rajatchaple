

Single Board Computers (SBC) like BBB have capability to boot from external SD card. To automate the SD card preparation process for BBB and Buildroot Linux build environment, a bash script has been made.

Script link: https://github.com/cu-ecen-aeld/final-project-SundarKrishnakumar/blob/master/prepare_usd.sh


## Partition details

| Partition | File system type | Size | Label| Flags |
| ---------------- | -------------| ---------------- | ---------------- |---------------- |
| #1     | fat16  | 256 MB              |  BOOT| boot, lba|
| #2      | ext4      | Rest of the space | ROOTFS |  None |

BOOT partition contents: DTB, MLO, u-boot.img, uEnv.txt, zImage.

ROOTFS partition contents: extracted contents from the rootfs.tar file.

## Script execution flow

![script-execution-flow](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/usd-script-flow-img.jpg)


## Script usage

NOTE: If using a VM, enable the SD card device in the VM software (Oracle Vbox, VMware, etc). This will let the VM use the SD card device.

Use 'lsblk' command to find out the device node path of the microSD card device. The device path will be of the form /dev/sdX where X=a/b/c/d/e..

After buildroot builds the linux image successfully, run the prepare_usd.sh script with sudo permissions from the root directory of the buildroot repository. See command below:

```
sudo ./prepare_usd.sh -d /dev/sdX
```

This script prepares the microSD card successfully with contents from the latest buildroot linux build.

## Script in action

![script-execution-flow](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/sdcard_prepare_in_action.PNG)




