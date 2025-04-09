# компилятор тот же, что использует ядро (передаем параметром)
# /usr/bin/x86_64-linux-gnu-gcc-12

obj-m := infoproc.o

all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

test:
	sudo dmesg -C > resdata.txt
	sudo insmod infoproc.ko 
	sudo dmesg > resdata.txt

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	sudo rmmod infoproc