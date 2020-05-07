PWD := $(shell pwd)
obj-m += ioctl_modul.o

all:
	make  -C $(LINUX_SRC) SUBDIRS=$(PWD) modules #ARCH=arm CROSS_COMPILE=$(TOOL_PATH)/$(CROSS_COMPILE)
clean:
	rm -rf *.ko *.o *.symvers *.order *.mod.c *.mod.o
