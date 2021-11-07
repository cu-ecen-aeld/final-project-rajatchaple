## Some useful findings

***1. Reading a register right after writing to it. Why?*** <br>
It is to ensure that the processor has completed its operation before subsequent operations can be started. 

Code: https://android.googlesource.com/kernel/msm/+/android-6.0.1_r0.74/drivers/spi/spi-omap2-mcspi.c (line# 187)

Reference: https://devzone.nordicsemi.com/f/nordic-q-a/42437/reading-a-register-right-after-writing-to-it-why



