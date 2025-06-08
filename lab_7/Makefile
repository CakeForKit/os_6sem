NAME_MODULE = wq

# Если KERNELRELEASE определён, значит вызов сделан из системы сборки ядра.
ifneq ($(KERNELRELEASE),)
	obj-m := $(NAME_MODULE).o
# Иначе вызов сделан прямо из командной строки - вызвать систему сборки ядра.
# KDIR - дерево исходных кодов ядра
# -C - смена своего каталога на указанный опцией (т.е на каталог исходных кодов ядра)
# там находит Makefile верхнего уровня ядра
# M= заставляет вернуться обратно в директорию исходников модуля, прежде чем попытаться построить целевой модуль
# ссылается на список модулей в obj-m

# sudo insmod $(NAME_MODULE).ko 
# sudo rmmod $(NAME_MODULE).ko
else
	CURRENT = $(shell uname -r)
	KDIR = /lib/modules/$(CURRENT)/build 
	PWD = $(shell pwd)

default:
	echo $(MAKE) -C $(KDIR) M=$(PWD) modules
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	make clean

load:
	sudo insmod $(NAME_MODULE).ko 
	
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
	@sudo rmmod $(NAME_MODULE).ko
	@rm *.ko *.symvers
endif 