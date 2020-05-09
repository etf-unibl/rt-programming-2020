#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include "../head/stat.h"

 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev stat_cdev;
stat_t stat;

static int __init stat_driver_init(void);
static void __exit stat_driver_exit(void);
 
/*************** Driver Fuctions **********************/
static int stat_open(struct inode *inode, struct file *file);
static int stat_release(struct inode *inode, struct file *file);
static ssize_t stat_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t stat_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long  stat_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = stat_read,
        .write          = stat_write,
        .open           = stat_open,
	.unlocked_ioctl = stat_ioctl,
        .release        = stat_release,
	
};


 
static int stat_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Rukovaoc otvoren...!!!\n");
        return 0;
}
 
static int stat_release(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "Rukovaoc zatvoren...!!!\n");
        return 0;
}
 
static ssize_t stat_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Read function\n");
        return 0;
}
static ssize_t stat_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        printk(KERN_INFO "Write function\n");
        return 0;
}
static long stat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
		switch(cmd) {
    	case WR_CMD:
	    	if(copy_from_user(&stat ,(stat_t*) arg, sizeof(stat_t)))
			{
        		printk(KERN_INFO "Cannot copy from user");
				return -1;
			}
            break;
        case RD_CMD:
            if(copy_to_user((stat_t*) arg, &stat, sizeof(stat_t)))
			{
				printk(KERN_INFO "Cannot copy to user");        			
				return -1;
			}
             break;
        }
        return 0;
}
static int __init stat_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "stat_Dev")) <0){
                printk(KERN_INFO "Cannot allocate major number\n");
                return -1;
        }
        printk(KERN_INFO "Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&stat_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&stat_cdev,dev,1)) < 0){
            printk(KERN_INFO "Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"stat_class")) == NULL){
            printk(KERN_INFO "Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"stat_device")) == NULL){
            printk(KERN_INFO "Cannot create the Statistic Device \n");
            goto r_device;
        }
        
 
        printk(KERN_INFO "Statistic Device Driver Insert...Done!!!\n");
    return 0;
 

r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        cdev_del(&stat_cdev);
        return -1;
}
void __exit stat_driver_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&stat_cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}
module_init(stat_driver_init);
module_exit(stat_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PURV 2020");
MODULE_VERSION("1.1");
