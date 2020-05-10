#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/proc_fs.h>
 
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

#define BR_CELIJA 100

int32_t value = 0;
char stat_array[300];
int br_iteracija = 0;
 
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev stat_cdev;
 
static int __init stat_driver_init(void);
static void __exit stat_driver_exit(void);
/*************** Driver Functions **********************/
static int stat_open(struct inode *inode, struct file *file);
static int stat_release(struct inode *inode, struct file *file);
static ssize_t stat_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t stat_write(struct file *filp, const char *buf, size_t len, loff_t * off);
 
/***************** Procfs Functions *******************/
static int open_proc(struct inode *inode, struct file *file);
static int release_proc(struct inode *inode, struct file *file);
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset);
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t * off);
 
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = stat_read,
        .write          = stat_write,
        .open           = stat_open,
        .release        = stat_release,
};
 
static struct file_operations proc_fops = {
        .open = open_proc,
        .read = read_proc,
        .write = write_proc,
        .release = release_proc
};
 
static int open_proc(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "statisticki proc file otvoren.....\t");
    br_iteracija = 0;
    return 0;
}
 
static int release_proc(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "statisticki proc file zatvoren.....\n");
    return 0;
}
 
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset)
{
	int i,br=0,len = 0;
	char temp[20];
    printk(KERN_INFO "citanje statistike\n");
    
    /* Evoluirano stanje postaje trenutno stanje. */ 
    for( i = 0; i < BR_CELIJA; ++i )
	{
		stat_array[i] = stat_array[ i + BR_CELIJA ];
	}
	/* Ispis statirtike. */ 
	for( i = 2 * BR_CELIJA; i < 2 * BR_CELIJA +20 ; ++i )
	{
		
		if( stat_array[i] != '/' )
		{
			temp[ len++ ] = stat_array[i];
		}
		else
		{
			temp[ len ] = '\0';
			switch(br)
			{
				case 0 : 
					printk(KERN_INFO "Broj zivih:     %s\n",temp);
					break;
				case 1 : 
					printk(KERN_INFO "Broj rodjenih:  %s\n",temp);
					break;
				case 2 : 
					printk(KERN_INFO "Broj umrlih:    %s\n",temp);
					break;
				case 3 : 
					printk(KERN_INFO "Broj iteracija: %s\n",temp);
					break;
				default: {}
			}
			len=0;
			br++;
			if( br == 4 ) break;
		}
		
	}	
    if ( copy_to_user(buffer,stat_array,2*BR_CELIJA) != 0 )
    {
		return -EFAULT;
	}
 
    return length;
}
 
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	int i,br_zivi = 0,br_rodjeni = 0,br_umrlih = 0;
	char temp[20];
	
    printk(KERN_INFO "upisano u statistiku.....\n");
    
    if( copy_from_user(stat_array,buff,len) != 0 )
    {
		return -EFAULT;
	}
    
    /* Racunanje statistike*/ 
    
    br_iteracija++;
    for( i = 0; i < BR_CELIJA; ++i )
	{
		
		if  ( stat_array[ i + BR_CELIJA ]  == 'o' ) 
		{
			br_zivi++;
			if (stat_array[ i ] == 'x')
				br_rodjeni++;
		}
		else if( stat_array[ i ] == 'o' )
			br_umrlih++;
	}
	
	sprintf(temp,"%d/%d/%d/%d/",br_zivi,br_rodjeni,br_umrlih,br_iteracija-1);
	for( i = 0; i < 20; ++i )
		stat_array[2*BR_CELIJA + i]= temp[i];
		
    return len+20;
}
 
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
 
        /*Creating Proc entry*/
        proc_create("stat_proc",0666,NULL,&proc_fops);
 
        printk(KERN_INFO "Statistic Device Driver Insert...Done!!!\n");
    return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}
 
void __exit stat_driver_exit(void)
{
        remove_proc_entry("stat_proc",NULL);
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
MODULE_DESCRIPTION("A simple device driver with statistic on procfs");
MODULE_VERSION("1.0");
