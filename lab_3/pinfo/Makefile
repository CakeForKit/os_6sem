# Если KERNELRELEASE определён, значит вызов сделан из системы сборки ядра.
ifneq ($(KERNELRELEASE),)
	obj-m := infoproc.o
# Иначе вызов сделан прямо из командной строки - вызвать систему сборки ядра.
# KDIR - дерево исходных кодов ядра
# -C - смена своего каталога на указанный опцией (т.е на каталог исходных кодов ядра)
# там находит Makefile верхнего уровня ядра
# M= заставляет вернуться обратно в директорию исходников модуля, прежде чем попытаться построить целевой модуль
# ссылается на список модулей в obj-m

# sudo insmod infoproc.ko 
# sudo rmmod infoproc.ko
else
	CURRENT = $(shell uname -r)
	KDIR = /lib/modules/$(CURRENT)/build 
	PWD = $(shell pwd)

default:
	echo $(MAKE) -C $(KDIR) M=$(PWD) modules
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	make clean

load:
	sudo insmod infoproc.ko 
	
res:
	sudo dmesg |  tail -n500 | grep + > resdata.txt

clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order
	@rm -f .*.*.cmd *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions
	
disclean: clean
	@sudo rmmod infoproc.ko
	@rm *.ko *.symvers
endif 