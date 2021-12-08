To set up the HDMI display using X11 on a BeagleBoneBlack following are the steps-

NOTE: This was tested using the latest version of buildroot. defconfig used: beaglebone_qt5_defconfig

Buildroot source: https://github.com/buildroot/buildroot.git

```
git clone https://github.com/buildroot/buildroot.git
```


1. Change to the buildroot directory, first do **make beaglebone_qt5_defconfig** and do **make menuconfig**.
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

7. Save and exit the menuconfig and then do **make** from inside the buildroot directory, to perform the build.
8. Once the build is complete, copy the contents to the SD card and test the image.
9. Once the image boots, first login and then do **startx** from the BBB terminal to start the X server. 

The hdmi display may go to sleep if inactive for a longer time. To disable sleep, do the following-

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

* Buildroot - https://www.mcu.by/buildroot-bbb/
* X11 setup - https://agentoss.wordpress.com/2011/03/06/building-a-tiny-x-org-linux-system-using-buildroot/
* Sleep settings - https://forums.raspberrypi.com/viewtopic.php?t=232009

