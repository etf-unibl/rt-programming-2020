#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Milos Stanivukovic");
MODULE_DESCRIPTION("...");

#define DEVICE_NAME "IOCTL_GOL_channel"
#define NUM 10

//Definisanje kodova za ioctl
#define RD_STATES _IOWR('a','a',unsigned char*)
#define RD_STATE _IOWR('a','b',unsigned char*)
#define RD_ALIVE _IOR('a','c',unsigned char*)
#define RD_BORN _IOR('a','d',unsigned char*)
#define RD_DIED _IOR('a','e',unsigned char*)
#define RD_GEN _IOR('a','f',unsigned long*)
#define WR_STATE _IOWR('a','g',unsigned char*)
#define WR_ALIVE _IOW('a','h',unsigned char*)
#define WR_BORN _IOW('a','i',unsigned char*)
#define WR_DIED _IOW('a','j',unsigned char*)
#define INC_GEN _IO('a','k')

static int __init start(void);
static void __exit end(void);

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t,loff_t*);
static long device_ioctl(struct file*, unsigned int, unsigned long);

static struct file_operations fops={
	//U dokumentaciji pise da postavljanjem vlasnika na THIS_MODULE nece dopustiti uklanjanje modula ako se koristi, tako da nema potrebe
	//u device_open i device_release dodavati posebno try_module_get i module_put
	.owner=THIS_MODULE,
	.read=device_read,
	.write=device_write,
	.open=device_open,
	.unlocked_ioctl=device_ioctl,
	.release=device_release
};

static dev_t dev;
static struct cdev cd;
static struct class* dev_class;

static int __init start(void){
	if(alloc_chrdev_region(&dev,0,1,DEVICE_NAME)<0){
		printk(KERN_INFO "Major broj za drajver se nije mogao alocirat!\n");
		return -1;
	}
	printk(KERN_INFO "Major=%d\nMinor=%d\n",MAJOR(dev),MINOR(dev));
	cdev_init(&cd,&fops);
	
	if(cdev_add(&cd,dev,1)<0){
		printk(KERN_INFO "Dodavanje uredjaja nije uspjelo!");
		unregister_chrdev_region(dev,1);
		return -1;
	}
	
	if((dev_class=class_create(THIS_MODULE,DEVICE_NAME))==NULL){
		printk(KERN_INFO "Stvaranje klase za uredjaj nije uspjelo!\n");
		unregister_chrdev_region(dev,1);
		return -1;
	}
	if(device_create(dev_class,NULL,dev,NULL,DEVICE_NAME)==NULL){
		printk(KERN_INFO "Stvaranje uredjaja nije uspjelo!\n");
		class_destroy(dev_class);
		unregister_chrdev_region(dev,1);
		return -1;
	}
	printk(KERN_INFO "Dodavanje uredjaja i stvaranje datoteke je zavrseno!\n");
	return 0;
}

static void __exit end(void){
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	unregister_chrdev_region(dev,1);
	printk(KERN_INFO "Uklanjanje uredjaja zavrseno!\n");
}

module_init(start);
module_exit(end);

static char field[NUM][NUM];
static unsigned char alive;
static unsigned char born;
static unsigned char died;
static unsigned long generation;
static int is_open=0;

static int device_open(struct inode* in, struct file* filp){
	/*
	 * Svaka instanca gol koja hoce da pocne sa izvrsavanjem mora da pokusa da prvo otvori datoteku koja odgovara ovom drajveru.
	 * Da bi se sprijecio slucaj da vise procesa pristupi datoteci mora da postoji neka kontrola.
	 * if(is_open!=0){
	 * 		return -1;
	 * }
	 * is_open=1;
	 * ...
	 * Gornji kod nije dovoljno siguran jer postoji situacija kada bi 2 procesa mogla da odjednom dobiju pristup datoteci.
	 * U slucaju da jedan prodje provjeru, ali izgubi vrijeme i ne stigne da uveca is_open. Drugi proces u tom slucaju moze
	 * da prodje provjeru i izmijeni promjenljivu.
	 * Posto nisam siguran kako se koristi sinhronizacija na kernelskom nivou koristi se donji kod.
	 * Prvo se is_open uvecava i onda se vrsi provjera da li je vrijednost veca od 1. Grub nacin zato sto moze da prouzrokuje da neki
	 * proces iako je prvi stigao opet ne dobije pravo da otvori datoteku, ali situacija da 2 dvije instance programa za igru zivota
	 * citaju i pisu u datoteku istovremeno sada ne postoji. Da bi proces dobio pravo da udje u datoteku mora da prodje provjeru uslova
	 * prije nego sto neki drugi proces pokusa uci i uveca promjenljivu. Moze se desiti slucaj cak i da proces koji izlazi iz datoteke
	 * privremeno blokira one koji pokusavaju da je otvore dok on ne umanji vrijednost is_open pri izlasku, ali to je samo u situaciji ako
	 * su 2 procesa pokusala pristupiti u relativno bliskom vremenskom intervalu i medjusobno se blokirala pa ih je drajver oba izbacio.
	 * I prvi metod bi mozda bio dovoljan za ovaj slucaj jer vrijeme koje je potrebno korisniku da pokrene nesto 2 puta je relativno
	 * veliko u odnosu na procesor, ali sa donjim metodom je sigurno. A sansa da se desi situacija gdje ce oba procesa medjusobno
	 * izbaciti nije velika niti problematicna, dovoljno je opet pokrenuti igru zivota. 
	 * */
	is_open++;
	if(is_open>1){
		is_open--;
		printk(KERN_INFO "Datoteka je vec otvorena ili ju je drugi proces u isto vrijeme pokusao otvoriti!\n");
		return -1;
	}
	generation=0;
	alive=0;
	born=0;
	died=0;
	printk(KERN_INFO "Datoteka za GOL uspjesno otvorena!\n");
	return 0;
}

static int device_release(struct inode* in, struct file* filp){
	is_open--;
	printk(KERN_INFO "Datoteka za GOL zatvorena!\n");
	return 0;
}

static ssize_t device_read(struct file* filp, char* buffer, size_t length, loff_t* offset){
	printk(KERN_INFO "Koristiti ioctl za citanje statistike!\n");
	return 0;
}

static ssize_t device_write(struct file* filp, const char* buffer, size_t length, loff_t* offset){
	printk(KERN_INFO "Korsititi ioctl za pisanje!\n");
	return length;
}

static long device_ioctl(struct file* filp, unsigned int cmd, unsigned long arg){
	//Buffer je lokalna promjenljiva u funkciji kako ne bi imali problema kada vise niti poziva ovu funkciju.
	char buffer[9];
	unsigned char x,y,r,ret=0;
	int i,j;
	switch(cmd){
		case RD_STATES:
			ret=copy_from_user(&x,(char*)arg,1);
			ret=copy_from_user(&y,((char*)arg)+1,1);
			r=0;
			for(i=x-1;i<=x+1;i++){
				for(j=y-1;j<=y+1;j++){
					if(i<0 || i>=NUM || j<0 || j>=NUM){
						buffer[r]=0;
					}
					else{
						if(field[i][j]==1 || field[i][j]==2){
							buffer[r]=1;
						}
						else{
							buffer[r]=0;
						}
					}
					r++;
				}
			}
			ret=copy_to_user((char*)arg,buffer,9);
			break;
		case RD_STATE:
			ret=copy_from_user(&x,(char*)arg,1);
			ret=copy_from_user(&y,((char*)arg)+1,1);
			ret=copy_to_user(((char*)arg)+2,&(field[x][y]),1);
			break;
		case RD_ALIVE:
			ret=copy_to_user((char*)arg,&alive,1);
			break;
		case RD_BORN:
			ret=copy_to_user((char*)arg,&born,1);
			break;
		case RD_DIED:
			ret=copy_to_user((char*)arg,&died,1);
			break;
		case RD_GEN:
			ret=copy_to_user((unsigned long*)arg,&generation,sizeof(unsigned long));
			break;
		case WR_STATE:
			ret=copy_from_user(&x,(char*)arg,1);
			ret=copy_from_user(&y,((char*)arg)+1,1);
			ret=copy_from_user(&(field[x][y]),((char*)arg)+2,1);
			break;
		case WR_ALIVE:
			ret=copy_from_user(&alive,(char*)arg,1);
			break;
		case WR_BORN:
			ret=copy_from_user(&born,(char*)arg,1);
			break;
		case WR_DIED:
			ret=copy_from_user(&died,(char*)arg,1);
			break;
		case INC_GEN:
			generation++;
			break;
		default:
			return -1;
	}
	return 0;
}
