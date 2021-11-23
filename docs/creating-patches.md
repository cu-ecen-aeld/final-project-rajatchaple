## Why do you want to create a patch file?

You have made a few changes to the kernel and you want to share them with your team members, you can create something called a patch file that contains the just the changes you have made.

## How to create your own patch file?

spi_no_idle.patch is a patch for disabling idling for the SPI HW module. We will be creating this patch file in a step by step process below-

* Change directory to the repository that contains the linux source code.
* Before making our changes, we should checkout a new branch.

```
host$ cd bb-kernel/KERNEL
host$ git status
# On branch master
nothing to commit (working directory clean)
```
```
host$ git checkout -b patchdemo
host$ git status
# On branch patchdemo
nothing to commit (working directory clean)
```

(We have created a new branch called 'patchdemo' and have checked out to our new branch. Now we can start making changes to the kernel code.)

* We are making changes to the file /arch/arm/mach-omap2/omap_hwmod_33xx_43xx_ipblock_data.c
* We will be adding the following line to the variables 'am33xx_spi0_hwmod' and 'am33xx_spi1_hwmod'

```
.flags		= HWMOD_NO_IDLE // Do not idle the hwmod at all
```

* Add the files that were modified and commit them

```
host$ git add ./arch/arm/mach-omap2/omap_hwmod_33xx_43xx_ipblock_data.c
host$ git commit -m"idling disabled for spi0 and spi1 hwmod"
```

* Finally, we can create the patch file.

```
host$ git format-patch master --stdout > spi_no_idle.patch
```
(The spi_no_idle.patch file will be created in the current directory.)

* If we inspect the file, we will see the changes that we made.

Take a look at the spi_no_idle.patch file here - https://github.com/cu-ecen-aeld/final-project-SundarKrishnakumar/blob/master/base_external/patches/linux/spi_no_idle.patch


## Reference

 * Book:  BeagleBone Cookbook (Page 492)
