## About

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

![script-execution-flow](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/usd-script-flow.jpg)
## Script usage


