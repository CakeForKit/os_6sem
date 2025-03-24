#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

typedef int Myfunc_t(const char *, const struct stat *, size_t);
static Myfunc_t myfunc;

static int myfunc(const char *filename, const struct stat *statptr, size_t level_depth) {
    printf("%*s%s\n", level_depth * 2, "", filename);
    return 0;
}

static int dopath(Myfunc_t* func, const char *filename, size_t level_depth) {
	struct stat     statbuf;
	struct dirent   *dirp;
	DIR             *dp;
	int             ret = 0;
    char            *ptr;
	if (lstat(filename, &statbuf) == -1) {
        perror("lstat");
        return 1;
    }
	if (S_ISDIR(statbuf.st_mode) == 0) {    /* не каталог */
        return(func(filename, &statbuf, level_depth)); 
    }

    /* каталог */
	if ((ret = func(filename, &statbuf, level_depth)) != 0)
		return ret;
    if ((dp = opendir(filename)) == NULL) {
        perror("opendir");
        return 1;
    }
    if (chdir(filename) == -1) {
        perror("chdir");
        return 1;
    }
    ret = 0;
    while (ret == 0 && (dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0)
            ret = dopath(func, dirp->d_name, level_depth + 1);
    }
    if (chdir("..") == -1) {
        perror("chdir");
        return 1;
    }
	if (closedir(dp) == -1) {
        perror("closedir");
        return 1;
    }
	return ret;    
}

static int myftw(char *filename, Myfunc_t *func) {
    if (filename == NULL)
        return 1;
    return(dopath(func, filename, 0));
}

int main(int argc, char *argv[]) {
    int ret;
    if (argc != 2) {
        printf("Использование: %s <начальный каталог>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    ret = myftw(argv[1], myfunc); 
    return ret;
}
