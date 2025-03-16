#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stddef.h>
#include <string.h>



/* тип функции, которая будет вызываться для каждого встреченного файла */
typedef int Myfunc(const char *, const struct stat *, int);

static Myfunc myfunc;
static int myftw(char *, Myfunc *);
static int dopath(Myfunc *);

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;


int main(int argc, char *argv[]) {
    int ret;
    if (argc != 2) {
        printf("Использование: ftw <начальный каталог>\n");
        exit(EXIT_FAILURE);
    }
    ret = myftw(argv[1], myfunc); /* выполняет всю работу */
    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if (ntot == 0)
        ntot = 1;   /* во избежание деления на 0; вывести 0 для всех счетчиков */
    printf("обычные файлы =                             %7ld, %5.2f %%\n", nreg, nreg*100.0/ntot);
    printf("каталоги =                                  %7ld, %5.2f %%\n", ndir, ndir*100.0/ntot);
    printf("специальные файлы блочных устройств =       %7ld, %5.2f %%\n", nblk, nblk*100.0/ntot);
    printf("специальные файлы символьных устройств =    %7ld, %5.2f %%\n", nchr, nchr*100.0/ntot);
    printf("FIFO =                                      %7ld, %5.2f %%\n", nfifo, nfifo*100.0/ntot);
    printf("символические ссылки =                      %7ld, %5.2f %%\n", nslink, nslink*100.0/ntot);
    printf("сокеты =                                    %7ld, %5.2f %%\n", nsock, nsock*100.0/ntot);
    exit(ret);
}

/*
 * Обойти дерево каталогов, начиная с каталога "pathname".
 * Пользовательская функция func() вызывается для каждого встреченного файла.
 */
#define FTW_F 1     /* файл, не являющийся каталогом */
#define FTW_D 2     /* каталог */
#define FTW_DNR 3   /* каталог, который не доступен для чтения */
#define FTW_NS 4    /* файл, информацию о котором невозможно получить с помощью stat */

static char *filename; /* полный путь к каждому из файлов */

/* возвращаем то, что вернула функция func() */
static int myftw(char *pathname, Myfunc *func)
{
    int len = 300;
    // filename = path_alloc(&len); /* выделить память для PATH_MAX+1 байт (листинг 2.3) */
    filename = malloc(len);
    if (filename == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    strncpy(filename, pathname, len);   /* защита от */
    filename[len-1] = 0;                /* переполнения буфера */
    return(dopath(func));
}

/* Обход дерева каталогов, начиная с "filename". Если "filename" не является
 * каталогом, для него вызывается lstat(), func() и затем выполняется возврат.
 * Для каталогов производится рекурсивный вызов функции. */
static int dopath(Myfunc* func)
{
	struct stat     statbuf;
	struct dirent   *dirp;
	DIR             *dp;
	int             ret = 0;
    char            *ptr;

	if (lstat(filename, &statbuf) < 0) /* ошибка вызова функции stat */
		return(func(filename, &statbuf, FTW_NS));
	if (S_ISDIR(statbuf.st_mode) == 0)  /* не каталог */
		return(func(filename, &statbuf, FTW_F)); // отобразить в дереве 

    /* Это каталог. Сначала вызовем функцию func(),
    * а затем обработаем все файлы в этом каталоге. */
	if ((ret = func(filename, &statbuf, FTW_D)) != 0)
		return(ret);
    ptr = filename + strlen(filename); /* установить указатель в конец filename */
    *ptr++ = '/';
    *ptr = 0;

	if ((dp = opendir(filename)) == NULL) // каталог недоступен
		return(func(filename, &statbuf, FTW_DNR));
    
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 || 
            strcmp(dirp->d_name, "..") == 0)
            continue;           /* пропустить каталоги "." и ".." */
        strcpy(ptr, dirp->d_name); /* добавить имя после слэша */
        if ((ret = dopath(func)) != 0) /* рекурсия */
            break; /* выход по ошибке */
    }
    ptr[-1] = 0; /* стереть часть строки от слэша и до конца */

	if (closedir(dp) < 0)
		perror("невозможно закрыть каталог");

	return(ret);    
}

static int myfunc(const char *pathname, const struct stat *statptr, int type) {
    switch (type)
    {
    case FTW_F:
        switch (statptr->st_mode & S_IFMT)
        {
            case S_IFREG: nreg++; break;
            case S_IFBLK: nblk++; break;
            case S_IFCHR: nchr++; break;
            case S_IFIFO: nfifo++; break;
            case S_IFLNK: nslink++; break;
            case S_IFSOCK: nsock++; break;
            case S_IFDIR:
                printf("признак S_IFDIR для %s", pathname);
                exit(EXIT_FAILURE);
        }
        break;
    case FTW_D:
        ndir++;
        break;
    case FTW_DNR:
        printf("закрыт доступ к каталогу %s", pathname);
        break;
    case FTW_NS:
        printf("ошибка вызова функции stat для %s", pathname);
        break;
    default:
        printf("неизвестный тип %d для файла %s", type, pathname);
        exit(EXIT_FAILURE);
    }
    return(0);
}