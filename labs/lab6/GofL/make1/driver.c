#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>   
#include <linux/kdev_t.h>
#include <linux/fs.h>             
#include<linux/uaccess.h>              

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Aleksandar L");
MODULE_VERSION ("1.0");
MODULE_DESCRIPTION ("driver");
 
#define DEVICE_NAME "ioctl_dev"
#define WR_COM _IOW('a','a',cell_info*)
#define RD_COM _IOR('a','b',cell_info*)
  
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev cdevice;

typedef struct{
    int num_liv_cells;
    int num_born_cells;
    int num_dead_cells;
    int gen_counter;
} cell_info;

cell_info cell_stat;

static int __init  Driver_init(void);
static void __exit driver_exit (void);

static int device_open (struct inode *inode, struct file *file);
static ssize_t device_read (struct file *fp, char __user * buf, size_t len,loff_t * off);
static ssize_t device_write (struct file *fp, const char *buf, size_t len,loff_t * off);
static long device_ioctl (struct file *file, unsigned int num,unsigned long arg);
static int device_release (struct inode *inode, struct file *file);

static struct file_operations fops = { 
    .owner = THIS_MODULE, 
    .read = device_read, 
    .write = device_write, 
    .open = device_open, 
    .unlocked_ioctl = device_ioctl, 
    .release = device_release, 
};

static int device_open (struct inode *inode, struct file *file){
    printk (KERN_INFO "Open handler\n");
    return 0;
}

static ssize_t device_read (struct file *fp, char __user * buf, size_t len, loff_t * off){
    printk (KERN_INFO "Read function\n");
    return 0;
}

static ssize_t device_write (struct file *fp, const char __user * buf, size_t len,loff_t * off){
    printk (KERN_INFO "Write function\n");
    return 0;
}

static int device_release (struct inode *inode, struct file *file){
    printk (KERN_INFO "Close handler\n");
    return 0;
}

static long device_ioctl (struct file *file, unsigned int num, unsigned long arg){
    switch (num){
        case WR_COM:
            if (copy_from_user (&cell_stat, (cell_info *) arg, sizeof (cell_info))){
                printk (KERN_INFO "Cannot copy from user\n");
                return -1;
            }
        break;
        
        case RD_COM:
            if (copy_to_user ((cell_info *) arg, &cell_stat, sizeof (cell_info))){
                printk (KERN_INFO "Cannot copy to user\n");
                return -1;
            }
        break;
    }
    return 0;
}


static int __init Driver_init (void){
    if ((alloc_chrdev_region (&dev, 0, 1, DEVICE_NAME)) < 0){
        printk (KERN_INFO "Cannot allocate major number\n");
        return -1;
    }
    printk (KERN_INFO "Major = %d Minor = %d \n", MAJOR (dev), MINOR (dev));
    cdev_init (&cdevice, &fops);
    if ((cdev_add (&cdevice, dev, 1)) < 0){
        printk (KERN_INFO "Cannot add the device to the system\n");
        unregister_chrdev_region (dev, 1);
        cdev_del (&cdevice);
        return -1;
    }
    if ((dev_class = class_create (THIS_MODULE, DEVICE_NAME)) == NULL){
        printk (KERN_INFO "Cannot create the struct class\n");
        unregister_chrdev_region (dev, 1);
        cdev_del (&cdevice);
        return -1;
    }
    if ((device_create (dev_class, NULL, dev, NULL, DEVICE_NAME)) == NULL){
        printk (KERN_INFO "Cannot create the Statistic Device \n");
        class_destroy (dev_class);
        unregister_chrdev_region (dev, 1);
        cdev_del (&cdevice);
        return -1;
    }
    printk (KERN_INFO "Device driver insert compleat\n");
    return 0;
}

void __exit driver_exit (void){
    device_destroy (dev_class, dev);
    class_destroy (dev_class);
    cdev_del (&cdevice);
    unregister_chrdev_region (dev, 1);
    printk (KERN_INFO "Driver removed\n");

} 

module_init (Driver_init);
module_exit (driver_exit);

