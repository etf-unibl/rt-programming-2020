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

#define MAX 100
 
int32_t value = 0;
char stat_array[MAX]="1234\0";
char matrix[MAX]="1100001100110011011010110001000101010101100010010100010000000000001100001000000010100000111100100110";
unsigned int brojZivihCelija=0;
unsigned int brojRodjenihCelija=0;
unsigned int brojUmrlihCelija=0;
unsigned int brojGeneracija=0;


union converter
{
	unsigned int source;
	char tgt[sizeof(int)];
};

static union converter conv;

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
    return 0;
}
 
static int release_proc(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "statisticki proc file zatvoren.....\n");
    return 0;
}
 
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length,loff_t * offset)
{
    printk(KERN_INFO "citanje statistike\n");
    if(length == MAX)
    {
        int i;
        for(i=0; i<MAX; i++)
            stat_array[i]=matrix[i];
    }
    else if(length == 16)
    {
        conv.source=brojZivihCelija;
        stat_array[0]=conv.tgt[0];
        stat_array[1]=conv.tgt[1];
        stat_array[2]=conv.tgt[2];
        stat_array[3]=conv.tgt[3];
    
        conv.source=brojRodjenihCelija;
        stat_array[4]=conv.tgt[0];
        stat_array[5]=conv.tgt[1];
        stat_array[6]=conv.tgt[2];
        stat_array[7]=conv.tgt[3];
    
        conv.source=brojUmrlihCelija;
        stat_array[8]=conv.tgt[0];
        stat_array[9]=conv.tgt[1];
        stat_array[10]=conv.tgt[2];
        stat_array[11]=conv.tgt[3];
    
        conv.source=brojGeneracija;
        stat_array[12]=conv.tgt[0];
        stat_array[13]=conv.tgt[1];
        stat_array[14]=conv.tgt[2];
        stat_array[15]=conv.tgt[3];
    }
    
    copy_to_user(buffer,stat_array,length); 
    return length;;
}
 
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    printk(KERN_INFO "upisano u statistiku.....\n");
    copy_from_user(stat_array,buff,len);

    if(stat_array[0]=='0' && len==6)          //stanja celija
    {
        conv.tgt[0]=stat_array[1];
        conv.tgt[1]=stat_array[2];
        conv.tgt[2]=stat_array[3];
        conv.tgt[3]=stat_array[4];
        matrix[conv.source]=stat_array[5];
    }
    else if(len==16)       
    {
        conv.tgt[0]=stat_array[0];
        conv.tgt[1]=stat_array[1];
        conv.tgt[2]=stat_array[2];
        conv.tgt[3]=stat_array[3];
        brojZivihCelija=conv.source;
    
        conv.tgt[0]=stat_array[4];
        conv.tgt[1]=stat_array[5];
        conv.tgt[2]=stat_array[6];
        conv.tgt[3]=stat_array[7];
        brojRodjenihCelija=conv.source;

        conv.tgt[0]=stat_array[8];
        conv.tgt[1]=stat_array[9];
        conv.tgt[2]=stat_array[10];
        conv.tgt[3]=stat_array[11];
        brojUmrlihCelija=conv.source;

        conv.tgt[0]=stat_array[12];
        conv.tgt[1]=stat_array[13];
        conv.tgt[2]=stat_array[14];
        conv.tgt[3]=stat_array[15];
        brojGeneracija=conv.source;
    }
    else if(len==MAX)   //omogucavanje unosa pocetnog stanja matice kroz naredbu echo
    {
        int i;
        for(i=0; i<MAX; i++)
            matrix[i]=stat_array[i];
    }
    return len;
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
