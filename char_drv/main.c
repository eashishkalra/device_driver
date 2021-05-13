#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#define DEVICE_MEM_SIZE 512

dev_t device_number;
char device_memory[DEVICE_MEM_SIZE];

loff_t pcd_llseek (struct file *filep, loff_t offset , int wherein)
{
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
	return 0;
}
ssize_t pcd_read (struct file *filep, char __user *buff, size_t size, loff_t *offset)
{

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
}
ssize_t pcd_write (struct file *filep, const char __user *buff, size_t size, loff_t *offset)
{
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
}
int pcd_open (struct inode *inode, struct file *filep)
{
	pr_crit("pcd_open called");
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
struct cdev pcd_cdev;
struct class *pcd_class;
struct device *pcd_device;
int pcd_driver_init(void)
{
	int ret;
	pr_crit("pcd_driver_init called");
	ret=alloc_chrdev_region(&device_number,0,1,"pcd_ashish");
	if(ret<0)
		goto end;
	pr_crit("device number allocated to driver is %d",device_number);
	cdev_init(&pcd_cdev, &pcd_fops);
	ret=cdev_add(&pcd_cdev, device_number,0);
	if(ret<0)
		goto cleanup1;
	pcd_class=class_create(THIS_MODULE,"pcd_class");
	if(pcd_class<0)
		goto cleanup2;
	pcd_device=device_create(pcd_class, NULL,device_number,NULL,"pcd");
	if(pcd_device<0)
		goto cleanup3;
	pr_crit("pcd_driver_init finished");
cleanup3:
	class_destroy(pcd_class);
cleanup2:
	cdev_del(&pcd_cdev);
cleanup1:
	unregister_chrdev_region(device_number,0);
end:
	return 0;
}
void pcd_driver_cleanup(void)
{
	pr_crit("pcd_driver_cleanup called");
	device_destroy(pcd_class,device_number);
	class_destroy(pcd_class);
	cdev_del(&pcd_cdev);
	unregister_chrdev_region(device_number,0);
	pr_crit("pcd_driver_cleanup finished");
}
module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ashish Kalra");
MODULE_DESCRIPTION("First char driver");




