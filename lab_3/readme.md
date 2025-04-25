# Загружаемый модули ядра
1. Вывод информации о процессах
2. Взаимодействие модудей ядра (по материалу О Цирюльника)


Unix Linux - монолитные минимизированные ядра.

**Монолитные ядра** - единая программа, состоящая из подсистем (модулей) - это все структуры и функции ядра

Разработчики монолитных ядер для того чтобы пользователи могли включать в ядро нужную им *функциональность*, которая обеспечивает выполение опреленнных действий в ядре. Unix Linux предоставляют возможность написания загружаемых модулей ядра, т е программировать в ядре без его перекомпиляции.

Windows тоже предоставляет возможность внесения дополнительной функциональности. Этя этого разарботаны многоуровневые драйверы, которые можно зарегистрировать в реестре (двухтомних по Windows + MSDN - базовая информация для Windows)

Для работы с кодами ядра надо:
- права root
- специальная компиляция


Загружаемые модули ядра имеют определенную структуру - 2 макрома (init, exit), в которые передаются 2 обязательные точки входа

```c
// __ сообщает ядру что на этот код надо обратить пристальное внимание
static int __init my_init(void) {
    printk(KERN_INFO, "my module load.\n");
    return 0;
}

static void __exit my_exit(void) {
    printk(KERN_INFO, "my module unload.\n");
}

module_init(my_init);
module_exit(my_exit);
```

В виде загружаемых модулей ядра пишутся самые разные коды ядра, вплоть до драйверов. Любой модуль ядра это многовходовая программа. Кода модуль загружается в ядра он становится частью ОС.

После того как резидентная программа остается резидентно в памяти он становится частью DOS. 

В ядре существуют свои библиотеки. Ядра написано на СИ. В ядре нельзя использовать библиотеки usermod.

printk - не входит ни в какую библиотеку и пишет в syslog.

8 уровней протоколирования. Чем меньше номер, тем больше приоритет. (KERN_INFO) Это можно псмотреть в lxr

## Макросы
Модули входят в библиотеку 
```c
#include <linux/module.h>
```

Ядро предоставляет нам функциии на для работы со списком процессов. Список не кольцевой. Это очередь. В голове списка процессов процесс pid=0.

## task_struct:
```c
struct list_head tasks; // структура двусвязный список
```

SMP-архитектуры (Symmetric multiprocessing) - равноправные процессоры с общей памятью. Но один процессор условно обозначается как главный, он может выполнять прерывание от системного таймера. 

Сейчас архитектуры NUMA

В компах нет единаго модуля, который бы определял загруженной каждго ядра и перераспределял между ними работу. Это слишом большой обьем анализируемой информации, причем выполняемый постоянно - слишком затратно.

Поэтому существует **миграция**. Каждое ядро скорее всего имеет ограничение на очередь выполняемых процессов. Если очередь переполняется происходит миграция, т е процесс переходит на выполнение на другое ядро.


```c
// page 87
/*  
 * Task state bitmask. NOTE! These bits are also
 * encoded in fs/proc/array.c: get_task_state().
 *
 * We have two separate sets of flags: task->__state
 * is about runnability, while task->exit_state are
 * about the task exiting. Confusing, but this way
 * modifying one set can't modify the other one by
 * mistake.
 */
/* Used in tsk->__state: */
#define TASK_RUNNING			0x00000000              
#define TASK_INTERRUPTIBLE		0x00000001              
#define TASK_UNINTERRUPTIBLE		0x00000002          
#define __TASK_STOPPED			0x00000004              
#define __TASK_TRACED			0x00000008              
/* Used in tsk->exit_state: */
#define EXIT_DEAD			0x00000010                  
#define EXIT_ZOMBIE			0x00000020                  
#define EXIT_TRACE			(EXIT_ZOMBIE | EXIT_DEAD)   

page 1672
/*
 * Per process flags
 */
#define PF_VCPU			0x00000001	/* I'm a virtual CPU */
#define PF_IDLE			0x00000002	/* I am an IDLE thread */
#define PF_EXITING		0x00000004	/* Getting shut down */
#define PF_POSTCOREDUMP		0x00000008	/* Coredumps should ignore this task */
#define PF_IO_WORKER		0x00000010	/* Task is an IO worker */
#define PF_WQ_WORKER		0x00000020	/* I'm a workqueue worker */
#define PF_FORKNOEXEC		0x00000040	/* Forked but didn't exec */
#define PF_MCE_PROCESS		0x00000080      /* Process policy on mce errors */
#define PF_SUPERPRIV		0x00000100	/* Used super-user privileges */
#define PF_DUMPCORE		0x00000200	/* Dumped core */
#define PF_SIGNALED		0x00000400	/* Killed by a signal */
#define PF_MEMALLOC		0x00000800	/* Allocating memory to free memory. See memalloc_noreclaim_save() */
#define PF_NPROC_EXCEEDED	0x00001000	/* set_user() noticed that RLIMIT_NPROC was exceeded */
#define PF_USED_MATH		0x00002000	/* If unset the fpu must be initialized before use */
#define PF_USER_WORKER		0x00004000	/* Kernel thread cloned from userspace thread */
#define PF_NOFREEZE		0x00008000	/* This thread should not be frozen */
#define PF_KCOMPACTD		0x00010000	/* I am kcompactd */
#define PF_KSWAPD		0x00020000	/* I am kswapd */
#define PF_MEMALLOC_NOFS	0x00040000	/* All allocations inherit GFP_NOFS. See memalloc_nfs_save() */
#define PF_MEMALLOC_NOIO	0x00080000	/* All allocations inherit GFP_NOIO. See memalloc_noio_save() */
#define PF_LOCAL_THROTTLE	0x00100000	/* Throttle writes only against the bdi I write to,
						 * I am cleaning dirty pages from some other bdi. */
#define PF_KTHREAD		0x00200000	/* I am a kernel thread */
#define PF_RANDOMIZE		0x00400000	/* Randomize virtual address space */
#define PF__HOLE__00800000	0x00800000
#define PF__HOLE__01000000	0x01000000
#define PF__HOLE__02000000	0x02000000
#define PF_NO_SETAFFINITY	0x04000000	/* Userland is not allowed to meddle with cpus_mask */
#define PF_MCE_EARLY		0x08000000      /* Early kill for mce process policy */
#define PF_MEMALLOC_PIN		0x10000000	/* Allocations constrained to zones which allow long term pinning.
						 * See memalloc_pin_save() */
#define PF_BLOCK_TS		0x20000000	/* plug has ts that needs updating */
#define PF__HOLE__40000000	0x40000000
#define PF_SUSPEND_TASK		0x80000000      /* This thread called freeze_processes() and should not be frozen */
```

```c
#ifdef CONFIG_SMP
    unsigned int __state;
    //  информация о процессоре на котором выполняется соответствующий процесс
	int on_cpu; 
#endif

    int				prio;   // приоритет // TODO
	int				static_prio;
	int				normal_prio;
    unsigned int	rt_priority; // real time
    ...
    unsigned int			policy; // алгоритм планирования
    ...
//     void				*migration_pending;
#ifdef CONFIG_SMP
	unsigned short			migration_disabled;
#endif
	unsigned short			migration_flags;
    ...
//     struct list_head tasks; // связный список процессов
//     struct mm_struct *mm;		// указатель на таблицы страниц - адресс таблицы страниц 4 уровня
//     struct mm_struct *active_mm;

    int				exit_state; // код завершения
	int				exit_code;
	int				exit_signal;
    ...
    // /* Children/sibling form the list of natural children: */ // Дерево процессов
	// struct list_head		children;
	// struct list_head		sibling;
	// struct task_struct		*group_leader;
    ...
    pid_t				pid; // + ppid
	pid_t				tgid;
    ...
    u64				utime;  // user time Время выполнения процесса в режиме ядра и в режиме пользователя
	u64				stime;  // system time
    ...
    /*
	 * executable name, excluding path. исполняемый файл
	 *
	 * - normally initialized setup_new_exec()
	 * - access it with [gs]et_task_comm()
	 * - lock it with task_lock()
	 */
	char				comm[TASK_COMM_LEN];
    ...
    // #ifdef CONFIG_SYSVIPC   // семафоры и разделяемыя память
    // struct sysv_sem			sysvsem;
    // struct sysv_shm			sysvshm;
    // #endif
    // ...
    // /* Filesystem information: */ // указатель на файловую систему к которой принадлежит процесс
	// struct fs_struct		*fs;
	// /* Open file information: */ // указатель на таблицу открытых файлов (до 512 файлов)
	// struct files_struct		*files;
```

## 1 часть лабы


```c
#include <linux/module.h>
#include <linux/kernel.h>

static int __md_init(void) {
    struct task_struct *task = &init_task;
    do {
        printk(KERN_INFO "***pid=%d, ppid=%d, comm=%s\n", task->pid, task->parant->pid, task->comm);
    } while ((task=next_task(task)) != &init_task);
    return 0;
}
```
Дополнительо: 
Есть символ CURRENT (symbol_current)

После цикла все тоже самое только для current


Анализируем kthread daemon, migration, ksoftuql
поток у которого prio < 120
policy = 2 ищем поток
Выводим state и flages это битовые маски характеризующие особенности процесса 
В lxr есть расписанные state 99 строка в библиотеке fd.h
1673 строка - flags


## 2 часть лабы
07.04.25

взаимодействие модулей ядра - через обращение к данным

чтобы прошла компиляция модулей которые обращаются в внешним данным (они могут не обратиться) данные обьявляются как внешние

взаимодействие модулей ядра выполняется по физическим адресам
т к ядро загружено в физическую память (резидентно), в отличие от процессов (она в ВАП)

Ядро прежде чем загружается в память, 

Чтобы ядро можно было адресовать для него создаются такие же таблицы страниц (4 уровня)

зубков - защищенный режим
лучше рудаков-финогенов - ассемблер - защищенный режим

Когда md2 сможет получить физический адрес данных md1 (когда md1 будет загружен в память). Т е **имеет значение парядок загрузки модулей**
Пока модуль используется его нельзя выгрузить

При сдаче лабы
создать историю ошибок и демонстрировать ее
также разобраться с порядком загрузки и выгрузки модулей из памяти

модуль 3 эквивалентен md2

