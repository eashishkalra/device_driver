#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform.h"



void pcdev_release(struct device *dev)
{
	pr_info("device released\n");
}
struct pcdev_platform_data pcdev_data={
	.size=512,.perm=0x01,.serial_number=12
};
struct platform_device pcd_dev={
	.name="Ashish",
	.id=0,
	.dev={
		.platform_data=&pcdev_data,
		.release=pcdev_release
	}
};

int __init pcd_platform_device_init(void)
{
	pr_info("pcd_platform_device_int_called\n");
	platform_device_register(&pcd_dev);
	return 0;
}

void __exit pcd_platform_device_cleanup(void)
{
	pr_info("pcd_platform_device_cleanup_called\n");
	platform_device_unregister(&pcd_dev);
}

module_init(pcd_platform_device_init);
module_exit(pcd_platform_device_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ASHISH KALRA");
MODULE_DESCRIPTION("Platform driver for pcd");
