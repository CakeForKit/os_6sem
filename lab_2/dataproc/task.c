#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#define buf_size 1024

void read_fnames_dir(char *dirname, FILE *fdout) {
    struct stat statbuf;
    if (lstat(dirname, &statbuf) == -1) {
        perror("lstat");
        return ;
    }
	if (S_ISDIR(statbuf.st_mode) == 0) {    /* не каталог */
        return;
    }

    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dirname)) == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0) {
            fprintf(fdout, "\t\t%s\n", dirp->d_name);
        }
    }
    closedir(dp);
}

void read_task(char *dirname, FILE *fdout) {
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dirname)) == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    int len;
    char buf[buf_size + 1] = "\0";
    char filename[buf_size + 1] = "\0";
    fprintf(fdout, "%s: \n", dirname);
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0) {
            sprintf(filename, "%s/%s", dirname, dirp->d_name);
            fprintf(fdout, "\t%s\n", dirp->d_name);
            read_fnames_dir(filename, fdout);
        }
    }
    fprintf(fdout, "\n\n");
    closedir(dp);
}



int main(int argc, char **argv) {
    if (argc != 2) {
        printf("ERR: need %s pid", argv[0]);
        exit(EXIT_FAILURE);
    }
    FILE *fout = fopen("./savedata/task.txt", "w");
    if (fout == NULL) {
        perror("fopen1");
        exit(EXIT_FAILURE);
    }

    char *strpid = argv[1];
    char filename[100];
    sprintf(filename, "/proc/%s/task", strpid);
    read_task(filename, fout);

    fclose(fout);
    
    return 0;
}