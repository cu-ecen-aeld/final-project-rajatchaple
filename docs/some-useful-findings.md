## Some useful findings

***1. Reading a register right after writing to it. Why?*** <br>
It is to ensure that the processor has completed its operation before subsequent operations can be started. 

Code: https://android.googlesource.com/kernel/msm/+/android-6.0.1_r0.74/drivers/spi/spi-omap2-mcspi.c (line# 187)

Reference: https://devzone.nordicsemi.com/f/nordic-q-a/42437/reading-a-register-right-after-writing-to-it-why

***2. void * devm_kzalloc (	struct device * dev, size_t size, gfp_t gfp)*** <br>
devm_kzalloc() is resource-managed kzalloc(). If we use devm_kzalloc() then no need to free this memory. Memory allocated with this function is automatically freed on driver detach.

Reference: https://docs.huihoo.com/linux/kernel/2.6.26/kernel-api/re121.html



