#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h>
#include<linux/kobject.h>
 
#define SUCCESS 0 

dev_t dev = 0;
static struct class *dev_class;
static struct cdev stat_cdev;
struct kobject *kobj_ref;
static int __init stat_driver_init(void);
static void __exit stat_driver_exit(void);
static int Device_Open  =   0;


/*************** Driver Fuctions **********************/
static int stat_open(struct inode *inode, struct file *file);
static int stat_release(struct inode *inode, struct file *file);
static ssize_t stat_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t stat_write(struct file *filp, const char *buf, size_t len, loff_t * off);
 
/*************** Sysfs Fuctions **********************/
static ssize_t sysfs_show_matrix(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store_matrix(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);
static ssize_t sysfs_show_alive(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store_alive(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);
static ssize_t sysfs_show_born(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store_born(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);
static ssize_t sysfs_show_died(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store_died(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);
static ssize_t sysfs_show_gen(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store_gen(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count);
                
//matrica trenutnih stanja celija, broj zivih, broj novorodjenih i broj novoumrlih
volatile int matrix[10][10];
volatile int num_alive, num_born, num_died, gen_num;
 
struct kobj_attribute stat_attr_matrix = __ATTR(matrix, 0660, sysfs_show_matrix, sysfs_store_matrix);
struct kobj_attribute stat_attr_alive = __ATTR(num_alive, 0660, sysfs_show_alive, sysfs_store_alive);
struct kobj_attribute stat_attr_born = __ATTR(num_born, 0660, sysfs_show_born, sysfs_store_born);
struct kobj_attribute stat_attr_died = __ATTR(num_died, 0660, sysfs_show_died, sysfs_store_died);
struct kobj_attribute stat_attr_gen = __ATTR(gen_num, 0660, sysfs_show_gen, sysfs_store_gen);

static struct file_operations fops =
{
    .owner          = THIS_MODULE,
    .read           = stat_read,
    .write          = stat_write,
    .open           = stat_open,
    .release        = stat_release,
};

static ssize_t sysfs_show_matrix(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int ret, i, j, k;
	printk(KERN_INFO "Sysfs - Read!!!\n");
	ret = 0;
	k = 0;
	for(i = 0; i < 10; i++){
		for(j = 0;j < 10; j++){
			ret += sprintf(&buf[k], "%d", matrix[i][j]);
			k++;
		}
	}
	return ret;
}
 
static ssize_t sysfs_store_matrix(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
    int i, j, k;
    printk(KERN_INFO "Sysfs - Write!!!\n");
	k= 0;
	for(i = 0; i < 10; i++){
		for(j = 0;j < 10; j++){
			sscanf(&buf[k],"%d",&matrix[i][j]);
			k++;
		}
	}
	return count;
}

static ssize_t sysfs_show_alive(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int res;
    printk(KERN_INFO "Sysfs - Read!!!");
    res = sprintf(buf, "%d", num_alive);
    printk(KERN_INFO "Alive - %d\n",num_alive);
    return res;
}
 
static ssize_t sysfs_store_alive(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
    printk(KERN_INFO "Sysfs - Write!!!");
    sscanf(buf,"%d",&num_alive);
    printk(KERN_INFO "Alive - %d\n",num_alive);
    return count;
}

static ssize_t sysfs_show_born(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int res;
    printk(KERN_INFO "Sysfs - Read!!!");
    res = sprintf(buf, "%d", num_born);
    printk(KERN_INFO "Born - %d\n",num_born);
    return res;
}
 
static ssize_t sysfs_store_born(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
    printk(KERN_INFO "Sysfs - Write!!!");
    sscanf(buf,"%d",&num_born);
    printk(KERN_INFO "Born - %d\n",num_born);
    return count;
}

static ssize_t sysfs_show_died(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int res;
    printk(KERN_INFO "Sysfs - Read!!!");
    res = sprintf(buf, "%d", num_died);
    printk(KERN_INFO "Died - %d\n",num_died);
    return res;
}
 
static ssize_t sysfs_store_died(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
    printk(KERN_INFO "Sysfs - Write!!!");
    sscanf(buf,"%d",&num_died);
    printk(KERN_INFO "Died - %d\n",num_died);
    return count;
}

static ssize_t sysfs_show_gen(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	int res;
    printk(KERN_INFO "Sysfs - Read!!!");
    res = sprintf(buf, "%d", gen_num);
    printk(KERN_INFO "Generation - %d\n",gen_num);
    return res;
}
 
static ssize_t sysfs_store_gen(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
    printk(KERN_INFO "Sysfs - Write!!!");
    sscanf(buf,"%d",&gen_num);
    printk(KERN_INFO "Generation - %d\n",gen_num);
    return count;
}

static int stat_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Rukovaoc otvoren...!!!\n");
	if (Device_Open)
	{
        return -EBUSY;
	}

	Device_Open++;
	try_module_get(THIS_MODULE);

	return SUCCESS;
}
 
static int stat_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Rukovaoc zatvoren...!!!\n");
    Device_Open--;  /* We're now ready for our next caller. */

	/*
	* Decrement the usage count, or else once you opened the file, you'll
	* never get get rid of the module.
	*/
	module_put(THIS_MODULE);

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
    
    /*Kreiranje direktorijuma stat_sysfs u /sys/kernel/ */
    kobj_ref = kobject_create_and_add("stat_sysfs",kernel_kobj);
    // kernel_kobj je parent koji odgovara folderu /sys/kernel
    // firmware_kobj je parent koji odgovara folderu /sys/firmware
    // fs_kobj je parent koji odgovara folderu /sys/fs
    // NULL je parent koji odgovara folderu /sys
    
    /*Kreiranje fajla za matrix*/
    if(sysfs_create_file(kobj_ref,&stat_attr_matrix.attr)){
		printk(KERN_INFO"Cannot create sysfs file for matrix......\n");
        goto r_sysfs_matrix;
    }
	
	/*Kreiranje fajla za num_alive*/
    if(sysfs_create_file(kobj_ref,&stat_attr_alive.attr)){
		printk(KERN_INFO"Cannot create sysfs file for num_alive......\n");
        goto r_sysfs_alive;
    }
	
	/*Kreiranje fajla za num_born*/
    if(sysfs_create_file(kobj_ref,&stat_attr_born.attr)){
		printk(KERN_INFO"Cannot create sysfs file for num_born......\n");
        goto r_sysfs_born;
    }
 	
	/*Kreiranje fajla za num_died*/
    if(sysfs_create_file(kobj_ref,&stat_attr_died.attr)){
		printk(KERN_INFO"Cannot create sysfs file for num_died......\n");
        goto r_sysfs_died;
    }
 
	/*Kreiranje fajla za gen_num*/
    if(sysfs_create_file(kobj_ref,&stat_attr_gen.attr)){
		printk(KERN_INFO"Cannot create sysfs file for gen_num......\n");
        goto r_sysfs_gen;
    }
 
    printk(KERN_INFO "Statistic Device Driver Insert...Done!!!\n");
    return 0;

r_sysfs_gen:
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &stat_attr_gen.attr);	
r_sysfs_died:
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &stat_attr_died.attr);
r_sysfs_born:
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &stat_attr_born.attr);
r_sysfs_alive:
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &stat_attr_alive.attr);
r_sysfs_matrix:
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &stat_attr_matrix.attr);
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    cdev_del(&stat_cdev);
    return -1;
}
void __exit stat_driver_exit(void)
{
	kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &stat_attr_gen.attr);
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &stat_attr_gen.attr);
	kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &stat_attr_born.attr);
	kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &stat_attr_alive.attr);
	kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &stat_attr_matrix.attr);
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
MODULE_DESCRIPTION("A simple device driver with statistic on sysfs");
MODULE_VERSION("1.1");
