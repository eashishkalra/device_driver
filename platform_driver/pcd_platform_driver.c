#include<linux/module.h>
#include<linux/platform_device.h>
#include<linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include "platform.h"

#define NUMBER_OF_DEVICES 1
loff_t pcd_llseek (struct file *filep, loff_t offset , int wherein)
{
#if 0
        loff_t temp;
        pr_crit("llseek called with whernin %d and offset %lld\n",wherein, offset);
        switch(wherein){
                case SEEK_SET:
                        if(offset>=DEVICE_MEM_SIZE)
                                return EINVAL;
                        filep->f_pos=offset;
                        break;
                case SEEK_CUR:
                        temp=filep->f_pos+offset;
                        if(temp>=DEVICE_MEM_SIZE)
                                return EINVAL;
                        filep->f_pos=temp;
                        break;
                case SEEK_END:
                        temp=DEVICE_MEM_SIZE+offset;
                        if(temp>=DEVICE_MEM_SIZE)
                                return EINVAL;
                        filep->f_pos=temp;
                        break;
        }
#endif	
        return 0;
}
ssize_t pcd_read (struct file *filep, char __user *buff, size_t size, loff_t *offset)
{
#if 0

        pr_crit("pcd_read called with size %ld and offset %lld\n",size, *offset);
        if(*offset+size>DEVICE_MEM_SIZE)
                size=DEVICE_MEM_SIZE-*offset;

        if(copy_to_user(buff,&device_memory[*offset],size))
        {
                return EINVAL;
        }
        *offset+=size;
        pr_crit("pcd_read end with offset %lld\n and read bytes%ld ",*offset,size);

        return size;
#endif
	return 0;	
}
ssize_t pcd_write (struct file *filep, const char __user *buff, size_t size, loff_t *offset)
{
#if 0
        pr_crit("pcd_write called with size %ld and offset %lld\n",size, *offset);
        if(*offset+size>DEVICE_MEM_SIZE)
                size=DEVICE_MEM_SIZE-*offset;
        if(!size)
                return ENOMEM;

        if(copy_from_user(&device_memory[*offset],buff,size))
        {
                return EINVAL;
        }
        *offset+=size;
        pr_crit("pcd_write end with offset %lld\n and write bytes%ld ",*offset,size);

        return size;
#endif
	return 0;	
}
int pcd_open (struct inode *inode, struct file *filep)
{
#if 0
        pr_crit("pcd_open called");
        int ret;
        int minor_n;
        minor_n=MINOR(inode->i_rdev);
#endif
        return 0;
}
int pcd_release (struct inode *inode, struct file *filep)
{
        pr_crit("pcd_release called");
        return 0;
}

struct file_operations pcd_fops=
{
        .open=pcd_open,
        .release=pcd_release,
        .read=pcd_read,
        .write=pcd_write
};
struct pcdev_private_data
{
	struct pcdev_platform_data pdata;
	char *buffer;
	dev_t dev_num;
	struct cdev cdev;
};

struct pcdrv_private_data
{
	int total_devices;
	dev_t device_num_base;
	struct class *pcd_class;
	struct device *pcd_device;

};

struct pcdrv_private_data pcdev_data;

int pcd_probe(struct platform_device *pcd_dev)
{
	int ret=0;
	struct pcdev_private_data *dev_data;
	struct pcdev_platform_data *pdata;
	pr_info("pcd_probe_called\n");
	pdata=(struct pcdev_platform_data*)dev_get_platdata(&pcd_dev->dev);
	if(!pdata){
		pr_info("No Platform data\n");
		goto out;
	}
	dev_data=kzalloc(sizeof(*dev_data),GFP_KERNEL);
	if(!dev_data)
	{
		pr_info("Memory allocation failed");
		ret=-ENOMEM;
		goto out;
	}
	dev_data->pdata.size=pdata->size;
	dev_data->pdata.perm=pdata->perm;
	dev_data->pdata.serial_number=pdata->serial_number;

	dev_data->buffer=kzalloc(dev_data->pdata.size,GFP_KERNEL);
        if(!dev_data)
        {
                pr_info("Memory allocation failed");
                ret=-ENOMEM;
                goto devdata_free;
        }
	dev_data->dev_num=pcdev_data.device_num_base+pcd_dev->id;

        cdev_init(&(dev_data->cdev), &pcd_fops);
        dev_data->cdev.owner=THIS_MODULE;
        ret=cdev_add(&dev_data->cdev, dev_data->dev_num,0);
        if(ret<0)
	        goto cleanup1;
	pcdev_data.pcd_device=device_create(pcdev_data.pcd_class, NULL,dev_data->dev_num,NULL,"pcd-%d",pcd_dev->id);
	if(pcdev_data.pcd_device<0)
                        goto cleanup2;

	pr_info("pcd_probe_finished\n");
	pcd_dev->dev.driver_data=dev_data;
	return ret;
cleanup2:
	pr_info("cleanup2\n");
	device_destroy(pcdev_data.pcd_class,dev_data->dev_num);
        cdev_del(&(dev_data->cdev));
cleanup1:
	pr_info("cleanup1\n");
	kfree(dev_data->buffer);
devdata_free:
	pr_info("devdata_free\n");
	kfree(dev_data);
out:
	pr_info("device probe failed\n");

	return ret;
}
int pcd_remove(struct platform_device *pcd_dev)
{
	struct pcdev_private_data *dev_data;
	dev_data=(struct pcdev_private_data *)(pcd_dev->dev.driver_data);
	pr_info("pcd_removed_called\n");
	device_destroy(pcdev_data.pcd_class,dev_data->dev_num);
        cdev_del(&(dev_data->cdev));
	kfree(dev_data->buffer);
	kfree(dev_data);
	return 0;
}

int pcdev_release(struct inode *inode, struct file *flip)
{
	return 0;
}
struct platform_driver pcd_drv={
	.probe=pcd_probe,
	.remove=pcd_remove,
	.driver={
		.name="Ashish",
	}
};




int __init pcd_platform_driver_init(void)
{
	int ret;
	pr_info("pcd_platform_driver_int_called\n");
	platform_driver_register(&pcd_drv);
        pr_crit("pcd_driver_init called");
        ret=alloc_chrdev_region(&pcdev_data.device_num_base,0,NUMBER_OF_DEVICES,"pcdev");
        if(ret<0)
                goto end;
        pr_crit("device number allocated to driver is %d",pcdev_data.device_num_base);

        pcdev_data.pcd_class=class_create(THIS_MODULE,"pcd_class");
        if(pcdev_data.pcd_class<0)
                goto cleanup2;

cleanup2:
        unregister_chrdev_region(pcdev_data.device_num_base,NUMBER_OF_DEVICES);
end:

	return 0;
}
void __exit pcd_platform_driver_cleanup(void)
{
	pr_info("pcd_platform_driver_cleanup_called\n");
	platform_driver_unregister(&pcd_drv);
        class_destroy(pcdev_data.pcd_class);
        unregister_chrdev_region(pcdev_data.device_num_base,NUMBER_OF_DEVICES);
}

module_init(pcd_platform_driver_init);
module_exit(pcd_platform_driver_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MODULE for PLATFORM DRIVER");
MODULE_AUTHOR("ASHISH");
