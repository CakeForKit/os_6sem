14.04.25 
# Передача данных между пространством ядра и пространством пользователя

Начиная с ядра 2.6 было реализовано приложение -- **механизм виртуальных файлов**. Для этого была реализована ВФС proc.

Следует понимать - в ядре вся информация о всех ресурсах системы, в ядре мы можем получить всю информацию о процессах (в рамках системы)

ВФС proc 
- предоставляет информацию в режиме пользователя (stat, environ - не тербует прав root)
- mem - демонстрирует скрытую фрагментацию (тербует прав root)
+ предоставляет информацию в режиме ядра
+ можно создавать в ядре виртуальные файлы (директория, символическая ссылк и файл)

Для этого определена структура *struct_dir_entry* - дескриптор файловой системы proc

```c
v 5.16.8

#include <linux/proc_fs.h>

/*
 * This is not completely implemented yet. The idea is to
 * create an in-memory tree (like the actual /proc filesystem
 * tree) of these proc_dir_entries, so that we can dynamically
 * add new files to /proc.
 *
 * parent/subdir are used for the directory structure (every /proc file has a
 * parent, but "subdir" is empty for all non-directory entries).
 * subdir_node is used to build the rb tree "subdir" of the parent.
 */
struct proc_dir_entry {
	/*
	 * number of callers into module in progress;
	 * negative -> it's going away RSN
	 */
	atomic_t in_use;
	refcount_t refcnt;
	struct list_head pde_openers;	/* who did ->open, but not ->release */
	/* protects ->pde_openers and all struct pde_opener instances */
	spinlock_t pde_unload_lock;
    ...
    const struct inode_operations *proc_iops;
	union {
		const struct proc_ops *proc_ops;     // специальные операции на ВФС proc (новшество)
		const struct file_operations *proc_dir_ops; // операции определенные на файлах (в старых ядрах только это). Это часто вызываемые функции и чтобы сократить количество обращений к этим функциям введи proc_ops
	};
	const struct dentry_operations *proc_dops;
	union {
		const struct seq_operations *seq_ops;   // Средство передачи данных из ядра в режим пользователя (сиквенсы)
		int (*single_show)(struct seq_file *, void *);  // упрощенный интерфейс 
	};
    proc_write_t write; // наследник старой структуры (раньше было определено 2 функции read write)
	void *data;	// : непрозрачный указатель, который может использоваться обработчиками proc для передачи локальных данных, обычно NULL
    ...
	unsigned int low_ino; // номер inode для директории
    nlink_t nlink;
	kuid_t uid;	// идентификатор пользователя (uid), которому принадлежит файл (узел), обычно 0
	...
	loff_t size;	//  устанавливает размер узла, значение будет отображаться как размер inode в списках и будет возвращено stat. Е
    struct proc_dir_entry *parent;
	struct rb_root subdir;
	struct rb_node subdir_node; 
	char *name; // униакльное имя для виртуального файла для данного inode
	umode_t mode;   // права доступа
	u8 flags;
	u8 namelen;
	char inline_name[];
} __randomize_layout;
```
Существует большое кол-во версий ядра, при эток код ядра и функции не стандартизированы. Стандартизировано API - POSIX Си и не переписывается. (Это связано с драйверами, чтобы они не перестали работать)

ВФС proc - только предоставляет информацию и никак не влиает на работу. Ее переписывают. 

*struct proc_ops* - функции которые определеные нч структуре proc_dir_entry. Они аналогичны struct file_operations. Они были введены специально, чтобы освободить фунции file_operations, которые часто вызываются. 


```C
struct proc_ops {
	unsigned int proc_flags;
	int	(*proc_open)(struct inode *, struct file *); // аналогична open, открываем физический файл
	ssize_t	(*proc_read)(struct file *, char __user *, size_t, loff_t *);
	ssize_t (*proc_read_iter)(struct kiocb *, struct iov_iter *);
	ssize_t	(*proc_write)(struct file *, const char __user *, size_t, loff_t *);
	/* mandatory unless nonseekable_open() or equivalent is used */
	loff_t	(*proc_lseek)(struct file *, loff_t, int);
    ...
    long	(*proc_ioctl)(struct file *, unsigned int, unsigned long);
    ...
} __randomize_layout;
```

Для создания виртуального файла в ВФС proc на *struct proc_ops* определена функция:
```C
extern struct proc_dir_entry *proc_create_data(const char *, umode_t,
                                                struct proc_dir_entry *,
                                                const struct proc_ops *,
                                                void *);

// Но чаще вызывают обертку:    (fs/proc/generic.c)
struct proc_dir_entry *proc_create(const char *name, umode_t mode,
				   struct proc_dir_entry *parent,
				   const struct proc_ops *proc_ops)
{
	return proc_create_data(name, mode, parent, proc_ops, NULL);
}
EXPORT_SYMBOL(proc_create);

extern void remove_proc_entry(const char *, struct proc_dir_entry *);

static inline struct proc_dir_entry *proc_symlink(
    const char *name, struct proc_dir_entry *parent,const char *dest
) { return NULL;}

static inline struct proc_dir_entry *proc_mkdir(
    const char *name, struct proc_dir_entry *parent
) {return NULL;}
```

```C
include/linux/uaccess.h
static __always_inline __must_check unsigned long
    __copy_to_user(void __user *to, const void *from, unsigned long n);
static __always_inline __must_check unsigned long
    __copy_from_user(void *to, const void __user *from, unsigned long n);
```
Копирует n байт адресуемых указателем to 

Функция проверяет: доступен ли буфер to для записи. 

Если перед функцией стоит __ это указание что на эту функцию надо обратить пристальное внимание.

В ядре могут использоватья только библиотеки ядра. Код ядра находится в физической памяти и когда мы загружаем модуль в ядро он также загружается в физическую память.

А информацию мы передаем какому то процессу (терминал это тоже процесс), соответственно у процессов только виртуальные адресные пространства (исполняемый код должен находится в физической памяти, соответствующая страница долна находиться в физической памяти). Но поскольку на ВАП определена опреция выгрузки, то возмодна ситуация когда ярдо обращается к буферу, а буфер находится в странице которая выгружена. __copy_to_user и __copy_from_user написаны так чтобы проверять такую возможность.

Есть функция sprintf - может записывать строку в буфер, у нее есть аналог snprintf

# Лаба 4. 
В ВФС создать файл, поддиректорию и символическую ссылку 

Передача данных между пространством ядра и пространством пользователя. Рассмомтрим 2 способа из многих: 

proc_read, proc_write - в этих функциях будем вызывать функции ядра: copy_to_user(), copy_from_user()


## Программа фортунки
Демонстрирует передачу, сам процесс почти ведьзе описан правильно.

Из user передаем pid-процесса (выбираем сами). По этому pid используя task_struct передаем из ядра в user информацию (несколько полей)

Для того чтобы мы могли работать с этими функциями:

# Стандартные способы регистрации функций
Пишем загружаемый модуль ядра, в нем будет 2 обязательные точки входа init и exit + доп точки входа: read write. __copy_to_user и __copy_from_user вызываются из наших функций

# Условная компиляция
```C
// до init
#ifdef HAVE_PROC_OPS
static struct proc_ops file_ops = {
    proc_read = procfs_read,
    proc_write = procfs_write,
    proc_open = procfs_open,
    proc_release = procfs_close,
}
#else
static const struct file_operationc file_ops = {
    read = procfs_read,
    write = procfs_write,
    open = procfs_open,
    release = procfs_release,
}
#endif
```
```C
// в init
... my_init() {
    ...
    my_proc_file = proc_create("my_file", 0644, NULL, &file_ops);   // создаем файл, перед этим его определяем - указатель на struct proc_dir_entry
}
```




# seq_file seq_operations
21.04.25

Файловые последовательноси. Это буффер

В proc определено 2 интерфейса передвачи данных из пространство ядра в пространство пользователя: итератор и упрощенный

При этом сиквенс файлы - только для передачи данных из пространства ядра. Для передачиз из пространства пользователя исподьзуется *proc_write* 

На сиквенсах определена структура *seq_file*

```C
struct seq_operations;

struct seq_file {
	char *buf;
	size_t size;
	size_t from;
	size_t count;
	...
	loff_t index;
	loff_t read_pos;
	struct mutex lock;  // В любой структуре есть поле для реализации монопольного доступа
	const struct seq_operations *op;    // операции которые определены на seq_file
	int poll_event;
	const struct file *file;    // в unix все файл
	void *private;
};

struct seq_operations {
	void * (*start) (struct seq_file *m, loff_t *pos);
	void (*stop) (struct seq_file *m, void *v);
	void * (*next) (struct seq_file *m, void *v, loff_t *pos);
	int (*show) (struct seq_file *m, void *v);  // m - указатель работы с буфером
};
```

Помимо этих структур на сиквенсах определены функции:
```C
int seq_open(struct file *, const struct seq_operations *); 
ssize_t seq_read(struct file *, char __user *, size_t, loff_t *); // __user - передача данных в буфер пользователя
ssize_t seq_read_iter(struct kiocb *iocb, struct iov_iter *iter); // Выполняет основную работу - вызывается из seq_read
// loff_t seq_lseek(struct file *, loff_t, int);
int seq_release(struct inode *, struct file *);
int seq_write(struct seq_file *seq, const void *data, size_t len);

void seq_printf(struct seq_file *m, const char *fmt, ...);

``` 
Отличие параметров:
```C
int	(*proc_open)(struct inode *, struct file *); // аналогична open, открываем физический файл
int seq_open(struct file *, const struct seq_operations *);
```

### Для чего написали еще один интерфейс?
Более надежная передача данных, заключается в том что все данные из буффера будут последовательно переданы в режим пользователя. Поэтому часто действия связанные с сиквенсами реализуются как итератор


### Диаграмма перехода от одной функции к другой
proc_seq_file.pdf

3 квадратика Stop, это связано с тем что - 
- когда выполняется инерация по байтам записаным в буфер 
- - и последрвательность заканчивается (next()->0 => Stop => Start (потому что может начаться перекачка следующей порции данных))
- - и уже Start проверяет содержит ли буфер информацию 
- - буфер заполнен

![Диаграмма перехода от одной функции к другой](img\image1.png)


seq_file.pdf

здесь функция которая передает данные - show(). Целью является - проход по всем данным. 
show - записывает данные в буфер чтения для ядра

Важно - когда последовательность заканчивается, другая может начаться, это значит что в конце функции стоп может быть снова вызвана функция старт. *Заканчивается когда функция старт возвращает null*
![alt text](img\image2.png)
 

proc_seq_file.pdf
```C
/**
* Эта функция вызывается, когда этот модуль загружается в ядро
*/
static int __init ct_init(void) {
	proc_create("evens", 0, NULL, &ct_file_ops);
	return 0;
}
```
proc_create - является оберткой proc_create_data

proc_create_data возвращается указатель на struct proc_dir_entry - дескриптор файловой системы proc

Здесь init ничего не возвращает - это неправильно 

передает не struct file_operations а struct proc_ops

```C
/**
* Эта структура собирает функции для управления последовательным чтением
*/
static struct seq_operations ct_seq_ops = {
 .start = ct_seq_start,
 .next = ct_seq_next,
 .stop = ct_seq_stop,
 .show = ct_seq_show
};

/**
* Эта функция вызывается при открытии файла из /proc
*/
static int ct_open(struct inode *inode, struct file *file) {
 return seq_open(file, &ct_seq_ops);
};
```

## Задание сиквенс файлы интерфейс итератор
21.04.25 1:12:12
Здесь в буфер кладут очередное четное число
Нам во всех функциях инициализируем поля proc_write, proc_release стандартными функциями ядра. Но надо написать свои функции my_ и в них во всех вызывать printk, потому что одной из задач этой лабы является понимание какие в этой программе точки входа. Мы увидим что не смотря на то что мы не вызываем open напрямую, она вызывается. **Поскольку это основные точки входа при работе с файлами, не могут не вызываться**

В итераторе выводим все указатели, чтобы понять на что они указывают. Мы выводим из task_struct информацию но чуть шире чем в фортунках (не 2-3 поля а больше)

Вызов функции next() приводит к вызову функции show()

```C
/**
* next
*/
static void *ct_seq_next(struct seq_file *s, void *v, loff_t *pos) {
	printk(KERN_INFO "...")
	seq_printf(current value)
	return 0;
}

```



# Упрощенный интерфейс сиквенс файлов
В struct proc_dir_entry  - 2 union. Во втором union - seq_operations и single_show. - 2 интерфейса связанные с sequence file
```C
union {
		const struct seq_operations *seq_ops;   // Средство передачи данных из ядра в режим пользователя (сиквенсы)
		int (*single_show)(struct seq_file *, void *);  // упрощенный интерфейс 
	};
```

В bootlin перехода на single_show - нет. Таким образом они обозначили интерфейс. Там переход на single_open.

24.04.25 00:09:09




## Основной закон программирования
Ни одна переменная не может использоваться до обьявления и определения.