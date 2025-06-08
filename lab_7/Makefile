TARGET1 = tasklet
TARGET2 = wq

CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(CURDIR)
obj-m := $(TARGET1).o $(TARGET2).o
default: 
	$(MAKE) -C $(KDIR) M=$(PWD) modules 
	make clean

load:
	sudo insmod $(TARGET1).ko 
	sudo insmod $(TARGET2).ko 
	
res:
	sudo dmesg |  tail -n300 | grep + > res.txt

run:
	echo 1 > /proc/mydir/myf
	echo 2 > /proc/mydir/myf
	echo 3 > /proc/mydir/myf
	cat /proc/mydir/myf

clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order
	@rm -f .*.*.cmd *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions
	
disclean: clean
	@sudo rmmod *.ko
	@rm *.ko *.symvers
