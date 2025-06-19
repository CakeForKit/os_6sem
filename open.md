28.04.25
# Буферизованный небуферизованный ввод вывод (открытые файлы)


Лабораторная выполняется в режиме пользователя и демонстрирует проблемы, которые могут у параллельный процессов(потоков) при работе с одними и теми же файлами.

Создать и открыть файл в Си может один и тот же системный вызов open(). В Си есть стандартная библиотека <stdio.h> - это библиотека буферизованного ввода/вывода, т е все функции этой библиотеки работают с буферами по умолчанию. 

open - системный вызов

fopen - библиотечная функция

```C
int open(const char *pathname, int flags);
int open(const char *pathname, int flags, mode_t mode);
```
Возвращает дескриптор открытого файла - он является дескриптором в таблице открытых файлов процесса.

Если указан флаг O_CREAT то создается новый дескриптор открытого файла (один и тот же файл может быть открыт много раз). Он остается открытым при системном вызове execve (В ядре существует тольок execve).

Если файл связан с процессов то о нем есть информация в дескрипторе процесса (task_struct)


```c
struct task_struct {
    ...
    struct fs_struct		*fs; // указатель на файловую систему к которой принадлежит процесс
    struct files_struct		*files;// указатель на таблицу открытых файлов
    ...
}
```

## struct fs_struct
Пока процесс не стал процессов он был файлом и принадлежал какой то файловой системе. 
```C
/* /include/linux/fs_struct.h */
struct fs_struct {
	int users;
	spinlock_t lock;    // средство взаимоисключения
	seqcount_spinlock_t seq;
	int umask;  // права доступа к файлу
	int in_exec;
	struct path root, pwd;  // корневой каталог и текущий рабочий каталог
} __randomize_layout;

/* /include/linux/path.h */
struct path {
	struct vfsmount *mnt;   // обьект монтировани 
	struct dentry *dentry;
} __randomize_layout;

/* /include/linux/mount.h */
struct vfsmount {
	struct dentry *mnt_root;	/* root of the mounted tree */
	struct super_block *mnt_sb;	/* pointer to superblock */
	int mnt_flags;
	struct user_namespace *mnt_userns;
} __randomize_layout;
```

Все структуры которые предназначены для работы с открытыми файлами связаны между собой.

struct task_struct -> struct fs_struct -> struct path -> struct vfsmount -> struct dentry

## struct files_struct
Каждый процесс имеет собствуенную таблицу открытых файлов. 
```C
/* /include/linux/fdtable.h */
/* Open file table structure */
struct files_struct {
  /* read mostly part */
	atomic_t count;  //счетчик ссылок на данную структуру 
	bool resize_in_progress;
	wait_queue_head_t resize_wait;

	struct fdtable __rcu *fdt;
	struct fdtable fdtab;
  /* written part on a separate cache line in SMP */
	spinlock_t file_lock ____cacheline_aligned_in_smp;
	unsigned int next_fd;   // (первый незанятый файловый дескриптор) для ускорения получения файлового дескриптора процесса когда он коткрывает файл
	unsigned long close_on_exec_init[1];
	unsigned long open_fds_init[1];
	unsigned long full_fds_bits_init[1];
	struct file __rcu * fd_array[NR_OPEN_DEFAULT];  // таб дескрипторов файлов процесса
};

/* /include/linux/fdtable.h */
struct fdtable {
	unsigned int max_fds;   // max count file descriptors
	struct file __rcu **fd;      /* current fd array */
	unsigned long *close_on_exec;   // fd которые должна закрываться при вызове exec
	unsigned long *open_fds;
	unsigned long *full_fds_bits;
	struct rcu_head rcu;
};
```

Когда мы создаем процесс, автоматически открывается 3 файла stdin, stdout, stderr (0, 1, 2) => next_fd = 3

Обьект - проинициализированная структура.

struct task_struct -> struct files_struct(обьект созданный для данного процесса) -> struct file __rcu * fd_array[NR_OPEN_DEFAULT]

В системе имется 1 таблица открытых файлов, в кот находятся дескрипторы всех открытых файлов, т е обьекты struct file


struct file -> struct path -> struct dentry

```C
struct file {
  struct path		f_path;
  struct inode		*f_inode;
  ...
  const struct file_operations	*f_op;
  ...
  atomic_long_t		f_count;
  unsigned int 		f_flags;
  fmode_t			f_mode;
  ...
  loff_t			f_pos;  // смезение в логическом файле
  ...
}
```

Файл считает что нач с нулевой позиции. Каждая операция чтения/записи смещает указатель на соответствующее количество байт.
В text файле - 1 байт. В bin файле - на соотв кол-во байт.

 
## Системный вызов open()
```C
#include <fcntl.h>
int open(const char *pathname, int flags, /* mode_t mode */ );
```
Флаги:
O_CREAT - If pathname does not exist, create it as a regular file. С ним рекомендуется использовать O_EXCL  - заставляет систему проверять, существует ли данный файл

O_APPEND The file is opened in append mode.  Before each write(2), the file offset is positioned at the end of the file, as if with lseek(2).

Если не создаем файл а открываем существующий то поле mode_t mode не нужно


## Буферизованный ввод вывод


```C
<stdio.h>
FILE *fopen( const char *filename, const char *mode );

/* /usr/include/x86_64-linux-gnu/bits/types/FILE.h */
struct _IO_FILE;
typedef struct _IO_FILE FILE;

/* /usr/include/x86_64-linux-gnu/bits/types/struct_FILE.h */
struct _IO_FILE
{
    int _flags;		/* High-order word is _IO_MAGIC; rest is flags. */

    /* The following pointers correspond to the C++ streambuf protocol. */
    char *_IO_read_ptr;	/* Current read pointer */
    char *_IO_read_end;	/* End of get area. */
    char *_IO_read_base;	/* Start of putback+get area. */
    char *_IO_write_base;	/* Start of put area. */
    char *_IO_write_ptr;	/* Current put pointer. */
    char *_IO_write_end;	/* End of put area. */
    char *_IO_buf_base;	/* Start of reserve area. */
    char *_IO_buf_end;	/* End of reserve area. */

    /* The following fields are used to support backing up and undo. */
    char *_IO_save_base; /* Pointer to start of non-current get area. */
    char *_IO_backup_base;  /* Pointer to first valid character of backup area */
    char *_IO_save_end; /* Pointer to end of non-current get area. */

    struct _IO_marker *_markers;

    struct _IO_FILE *_chain;

    int _fileno;    // дескриптор таблицы открытых файлов процесса
    int _flags2;
    __off_t _old_offset; /* This used to be _offset but it's too small.  */

    /* 1+column number of pbase(); 0 is unknown. */
    unsigned short _cur_column;
    signed char _vtable_offset;
    char _shortbuf[1];

    _IO_lock_t *_lock;
};
```

struct _IO_FILE - все поля структуры - указатели типа char* на буфер 

## Задание
методичка Лаб_работа5_open()_2022.pdf

Вызывается системный вызов open, который открывает файл "alphabet.txt" в режиме "только для чтения" и возвращает дескриптор в таблице открытых файлов процесса (fd). 

Вызывается 2 раза функция fdopen, которой передается файловый дескриптор fd.

Вызывается системный вызов open, который возвращает дескриптор fd дескриптор в таблице открытых файлов процесса. Вызывается 2 раза функция fdopen, которой передается файловый дескриптор fd. 

Функции возвращают указательна FILE: fs1, fs2. После того как получен указатель fs1, создается буфер функцией setvbuf, ей передается указатель fs1, буфер размером 20 байт.

В циклей while: если 

1:30:00