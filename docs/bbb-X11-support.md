To set up the HDMI display using X11 on a BeagleBoneBlack following are the steps-

Follow instructions in the 'Setting up buildroot environment for BBB' section [here](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/docs/setting-up-the-environment.md) to setup the buildroot environment for BeagleBoneBlack. NOTE: Make sure to use the defconfig called **beaglebone_qt5_defconfig** in the shared.sh file.

1. From the buildroot directory, do **make menuconfig**.
2. Inside menuconfig, under Toolchain section, enable the following-

 (Press Spacebar to enable)
 
```
Toolchain ->
[X] Enable C++ support
```

3. Go to 'Target Packages' section -> Graphic libraries and applications -> X.org X Window systems and do the following-

```
X11R7 Servers ->
[X] Enable xorg-server -> (Modular X.org)
```
NOTE: We can enable any X11R7 Applications that we want, based on the project requirements.
```
X11R7 Applications ->
[X] Enable twm (It is like a window manager, useful to have.)
[X] Enable xinit (Required)

X11R7 Drivers —> 
For best compatibility, we can enable all the drivers in this category.

```
4. Under 'Target packages' section -> Fonts, cursors, icons, sounds and themes-
```
[X] Enable Liberation (free fonts)
```
5. Go to 'Target Packages' section -> Graphic libraries and applications-

```
[X] Enable xterm
```
6. Go to 'Target Packages' section -> Libraries -> Text and terminal handling-

```
[X] Enable ncurses (NOTE: ncurses is a needed dependency for xterm)
```

7. Save and exit the menuconfig and do ./saveconfig.sh from the buildroot directory. At this point, the external config file should like something like [this](https://github.com/cu-ecen-aeld/final-project-SundarKrishnakumar/blob/master/base_external/configs/aesd_bbb_qt5_defconfig).
8. Next do ./build.sh. Once the build is complete, copy the contents to the SD card and test the image.
9. Once the image boots, first login and then do **startx** from the BBB terminal to start the X server. 

The hdmi display may go to sleep if inactive for a long time. To disable sleep, do the following-

1. Go to /etc/X11/xorg.conf.d/ and create a new file named 10-time.conf 
 
(Sometimes xorg.conf.d directory might be present in /usr/share/X11/ directory.)

2. Paste the below contents in that file and re-run **startx**.
```
Section "ServerLayout"
    Identifier "ServerLayout0"
    Option "BlankTime"   "0"
    Option "StandbyTime" "0"
    Option "SuspendTime" "0"
    Option "OffTime"     "0"
EndSection
```
(The hdmi display will look similar to the image below.)

### Display

![bbb-X11-ui](https://github.com/cu-ecen-aeld/final-project-rajatchaple/blob/main/images/bbb-X11.PNG)


### References

* X11 setup - https://agentoss.wordpress.com/2011/03/06/building-a-tiny-x-org-linux-system-using-buildroot/
* Sleep settings - https://forums.raspberrypi.com/viewtopic.php?t=232009


