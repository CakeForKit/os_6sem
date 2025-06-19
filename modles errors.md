```bash
kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ make
...
kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ ls *.ko
md1.ko  md2.ko  md3.ko

kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ sudo insmod md2.ko 
[sudo] password for kathrine: 
insmod: ERROR: could not insert module md2.ko: Unknown symbol in module

kathrine@Viva:~/vuz/os_6sem/lab_3/merr$  dmesg | tail -n30 | grep md
[  767.085520] md2: loading out-of-tree module taints kernel.
[  767.085598] md2: Unknown symbol md1_data (err -2)
[  767.085604] md2: Unknown symbol md1_proc (err -2)


kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ sudo insmod md1.ko
kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ sudo insmod md2.ko
kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ lsmod | grep md
md2                    16384  0
md1                    16384  1 md2

kathrine@Viva:~/vuz/os_6sem/lab_3/merr$  dmesg | tail -n60 | grep +
[ 1332.442253] + module md1 start!
[ 1341.193562] + module md2 start!
[ 1341.193567] + data string exported from md1 : Привет мир!
[ 1341.193570] + string returned md1_proc() is : Привет мир!


// Попытка выгрузить модуль
kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ sudo rmmod md1
rmmod: ERROR: Module md1 is in use by: md2

// Счетчик ссылок на md1 не равен 0 -> он не может быть выгружен
kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ lsmod | grep md
md2                    16384  0
md1                    16384  1 md2

// выгружаем модули в правильном порядке -> операционная система вернулась в исходное состояние!
kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ sudo rmmod md2
kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ sudo rmmod md1
kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ lsmod | grep md


// загрузка модуля md3.ko
kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ sudo insmod md3.ko
insmod: ERROR: could not insert module md3.ko: Operation not permitted

kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ dmesg | tail -n60 | grep +
[ 3113.141802] + module md1 start!
[ 3121.058382] + module md2 start!
[ 3121.058384] + data string exported from md1 : Привет мир!
[ 3121.058386] + string returned md1_proc() is : Привет мир!
[ 3127.243788] + module md3 start!
[ 3127.243827] + data string exported from md1 : Привет мир!
[ 3127.243831] + string returned md1_proc() is : Привет мир!

kathrine@Viva:~/vuz/os_6sem/lab_3/merr$ lsmod | grep md
md2                    16384  0
md1                    16384  1 md2
```


### Ошибки на этапе компиляции 

```bash
$ make
make -w -C /lib/modules/5.19.0-45-generic/build M=/home/vladislav/OS/6sem/lab_05 modules 
make[1]: Entering directory '/usr/src/linux-headers-5.19.0-45-generic'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  You are using:           gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  CC [M]  /home/vladislav/OS/6sem/lab_05/md1.o
/home/vladislav/OS/6sem/lab_05/md1.c:11:14: warning: ‘md1_local_proc’ defined but not used [-Wunused-function]
   11 | static char* md1_local_proc( void ) {
      |              ^~~~~~~~~
  CC [M]  /home/vladislav/OS/6sem/lab_05/md2.o
In file included from ./include/linux/kernel.h:29,
                 from ./arch/x86/include/asm/percpu.h:27,
                 from ./arch/x86/include/asm/nospec-branch.h:14,
                 from ./arch/x86/include/asm/paravirt_types.h:40,
                 from ./arch/x86/include/asm/ptrace.h:97,
                 from ./arch/x86/include/asm/math_emu.h:5,
                 from ./arch/x86/include/asm/processor.h:13,
                 from ./arch/x86/include/asm/timex.h:5,
                 from ./include/linux/timex.h:67,
                 from ./include/linux/time32.h:13,
                 from ./include/linux/time.h:60,
                 from ./include/linux/stat.h:19,
                 from ./include/linux/module.h:13,
                 from /home/vladislav/OS/6sem/lab_05/md2.c:2:
/home/vladislav/OS/6sem/lab_05/md2.c: In function ‘md_init’:
/home/vladislav/OS/6sem/lab_05/md2.c:14:57: error: implicit declaration of function ‘md1_local_proc’ [-Werror=implicit-function-declaration]
   14 |      printk( "+ string returned md1_local_proc() : %s\n", md1_local_proc() );
cc1: some warnings being treated as errors
make[2]: *** [scripts/Makefile.build:257: /home/vladislav/OS/6sem/lab_05/md2.o] Error 1
make[1]: *** [Makefile:1857: /home/vladislav/OS/6sem/lab_05] Error 2
make[1]: Leaving directory '/usr/src/linux-headers-5.19.0-45-generic'
make: *** [Makefile:10: default] Error 2
```

```bash
$ make
make -w -C /lib/modules/5.19.0-45-generic/build M=/home/vladislav/OS/6sem/lab_05 modules 
make[1]: Entering directory '/usr/src/linux-headers-5.19.0-45-generic'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  You are using:           gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  CC [M]  /home/vladislav/OS/6sem/lab_05/md1.o
/home/vladislav/OS/6sem/lab_05/md1.c:11:14: error: static declaration of ‘md1_local_proc’ follows non-static declaration
   11 | static char* md1_local_proc( void ) {
      |              ^~~~~~~~~~~~~~
In file included from /home/vladislav/OS/6sem/lab_05/md1.c:3:
/home/vladislav/OS/6sem/lab_05/md.h:4:14: note: previous declaration of ‘md1_local_proc’ with type ‘char *(void)’
    4 | extern char* md1_local_proc(void);
      |              ^~~~~~~~~~~~~~
/home/vladislav/OS/6sem/lab_05/md1.c:11:14: warning: ‘md1_local_proc’ defined but not used [-Wunused-function]
   11 | static char* md1_local_proc( void ) {
      |              ^~~~~~~~~~~~~~
make[2]: *** [scripts/Makefile.build:257: /home/vladislav/OS/6sem/lab_05/md1.o] Error 1
make[1]: *** [Makefile:1857: /home/vladislav/OS/6sem/lab_05] Error 2
make[1]: Leaving directory '/usr/src/linux-headers-5.19.0-45-generic'
make: *** [Makefile:10: default] Error 2
```

```bash
$ make
make -w -C /lib/modules/5.19.0-45-generic/build M=/home/vladislav/OS/6sem/lab_05 modules 
make[1]: Entering directory '/usr/src/linux-headers-5.19.0-45-generic'
warning: the compiler differs from the one used to build the kernel
  The kernel was built by: x86_64-linux-gnu-gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  You are using:           gcc (Ubuntu 11.3.0-1ubuntu1~22.04.1) 11.3.0
  CC [M]  /home/vladislav/OS/6sem/lab_05/md1.o
  CC [M]  /home/vladislav/OS/6sem/lab_05/md2.o
  CC [M]  /home/vladislav/OS/6sem/lab_05/md3.o
  MODPOST /home/vladislav/OS/6sem/lab_05/Module.symvers
ERROR: modpost: "md1_noexport_proc" [/home/vladislav/OS/6sem/lab_05/md2.ko] undefined!
make[2]: *** [scripts/Makefile.modpost:128: /home/vladislav/OS/6sem/lab_05/Module.symvers] Error 1
make[1]: *** [Makefile:1771: modules] Error 2
make[1]: Leaving directory '/usr/src/linux-headers-5.19.0-45-generic'
make: *** [Makefile:10: default] Error 2