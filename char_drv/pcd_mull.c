#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#define DEVICE_MEM_SIZE 512

#define NUMBER_OF_DEVICES 4

char device_memory[DEVICE_MEM_SIZE];
char device_memory1[DEVICE_MEM_SIZE];
char device_memory2[DEVICE_MEM_SIZE];
char device_memory3[DEVICE_MEM_SIZE];
char device_memory4[DEVICE_MEM_SIZE];

struct dev_data
{
	char *buffer;
	unsigned int size;
	const char *serial_number;
	int perm;
	struct cdev cdec;
};

struct drv_data
{
	dev_t device_number;
	struct class *pcd_class;
	struct device *pcd_device;
	int total_devices;
	struct dev_data pcd_dev[NUMBER_OF_DEVICES];
};

struct drv_data pcdrv_data =
{
	.total_devices=NUMBER_OF_DEVICES,
	.pcd_dev={
		[0]={
			.buffer=device_memory1,
			.size=DEVICE_MEM_SIZE,
			.serial_number="PCDEV1XYZ123",
			.perm=0x1/*RDONLY*/
		},
       		[1]={
                        .buffer=device_memory2,
                        .size=DEVICE_MEM_SIZE,
                        .serial_number="PCDEV2XYZ123",
                        .perm=0x10/*WRONLY*/
                },
		[2]={
                        .buffer=device_memory3,
                        .size=DEVICE_MEM_SIZE,
                        .serial_number="PCDEV3XYZ123",
                        .perm=0x11/*RDWR*/
                },
		[3]={
                        .buffer=device_memory4,
                        .size=DEVICE_MEM_SIZE,
                        .serial_number="PCDEV4XYZ123",
                        .perm=0x11/*RDWR*/
                }
	}
};

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
int pcd_driver_init(void)
{
	int ret,i;
	pr_crit("pcd_driver_init called");
	ret=alloc_chrdev_region(&pcdrv_data.device_number,0,NUMBER_OF_DEVICES,"pcd_mul");
	if(ret<0)
		goto end;
	pr_crit("device number allocated to driver is %d",pcdrv_data.device_number);

	pcdrv_data.pcd_class=class_create(THIS_MODULE,"pcd_class");
	if(pcdrv_data.pcd_class<0)
		goto cleanup2;

	for(i=0;i<NUMBER_OF_DEVICES;i++){
		cdev_init(&(pcdrv_data.pcd_dev[i].cdec), &pcd_fops);
		pcdrv_data.pcd_dev[i].cdec.owner=THIS_MODULE;
		ret=cdev_add(&pcdrv_data.pcd_dev[i].cdec, pcdrv_data.device_number,0);
		if(ret<0)
			goto cleanup1;
		pcdrv_data.pcd_device=device_create(pcdrv_data.pcd_class, NULL,pcdrv_data.device_number,NULL,"pcd");
		if(pcdrv_data.pcd_device<0)
			goto cleanup3;
	pr_crit("pcd_driver_init finished");
	}
cleanup3:
	for(i=NUMBER_OF_DEVICES-1;i>=0;i--){
		device_destroy(pcdrv_data.pcd_class,pcdrv_data.device_number+i);
		cdev_del(&pcdrv_data.pcd_dev[i].cdec);
	}
cleanup2:
	class_destroy(pcdrv_data.pcd_class);
cleanup1:
	unregister_chrdev_region(pcdrv_data.device_number,NUMBER_OF_DEVICES);
end:
	return 0;
}
void pcd_driver_cleanup(void)
{
	int i;
	for(i=NUMBER_OF_DEVICES-1;i>=0;i--){
		device_destroy(pcdrv_data.pcd_class,pcdrv_data.device_number+i);
		cdev_del(&pcdrv_data.pcd_dev[i].cdec);
	}
	class_destroy(pcdrv_data.pcd_class);
	unregister_chrdev_region(pcdrv_data.device_number,NUMBER_OF_DEVICES);
	pr_crit("pcd_driver_cleanup finished");
}
module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ashish Kalra");
MODULE_DESCRIPTION("First char driver");




