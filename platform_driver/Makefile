obj-m=pcd_platform_device.o pcd_platform_driver.o

module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
rpimodule:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C /home/ashish/rpi/linux/ M=$(PWD) modules

