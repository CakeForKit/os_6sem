#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define buf_size 1024

void read_file(char *filename, FILE *fdout) {
    int fd = open(filename, O_RDONLY);  
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    char buf[buf_size + 1] = "\0";
    int len;
    fprintf(fdout, "%s: \n", filename);
    while ((len = read(fd, buf, buf_size)) > 0) {
        for (int i = 0; i < len; i++) 
            if (buf[i] == 0)
                buf[i] = '\n';
        buf[len] = '\0';
        fprintf(fdout, "%s\n", buf);
    }
    fprintf(fdout, "\n\n");
    close(fd);
}

void read_maps(char *filename, FILE *fdout) {
    int staddr, endaddr, page_size = 4096, cntpages = 0;
    char *buf = NULL;
    size_t sizebuf;
    ssize_t lenbuf;
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    fprintf(fdout, "%s: \n", filename);
    fprintf(fdout, "%s\t%s\n", "cnt_pages", "maps data");
    while ((lenbuf = getline(&buf, &sizebuf, file)), !feof(file)) {
        if (!feof(file) && lenbuf == -1) {
            perror("getline");
            fclose(file);
            free(buf);
            exit(EXIT_FAILURE);
        }
        sscanf(buf, "%x-%x", &staddr, &endaddr);
        fprintf(fdout, "%d\t\t\t%s", (endaddr - staddr) / page_size, buf);
        cntpages += (endaddr - staddr) / page_size;
    }
    fprintf(fdout, "count pages = %d\n", cntpages);
    fclose(file);
}

void read_symlink(char *filename, FILE *fdout) {
    char buf[buf_size + 1] = "\0";
    int len = readlink(filename, buf, buf_size);
    if (len == -1) {
        perror("readlink");
        fprintf(fdout, "%s\n", filename);
        exit(EXIT_FAILURE);
    }
    buf[len] = '\0';
    fprintf(fdout, "%s: \n%s\n", filename, buf);
    fprintf(fdout, "\n\n");
}

void read_dir(char *dirname, FILE *fdout) {
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
            len = readlink(filename, buf, buf_size);
            buf[len] = '\0';
            fprintf(fdout, "\t%s [ %s ]\n", dirp->d_name, buf);
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
    FILE *fall_out = fopen("./savedata/savedatas.txt", "w");
    if (fall_out == NULL) {
        perror("fopen1");
        exit(EXIT_FAILURE);
    }
    fprintf(fall_out, "here\n");

    char *strpid = argv[1];
    char filename[100];
    sprintf(filename, "/proc/%s/cmdline", strpid);
    read_file(filename, fall_out);

    sprintf(filename, "/proc/%s/cwd", strpid);
    read_symlink(filename, fall_out);

    sprintf(filename, "/proc/%s/environ", strpid);
    read_file(filename, fall_out);

    sprintf(filename, "/proc/%s/exe", strpid);
    read_symlink(filename, fall_out);

    sprintf(filename, "/proc/%s/fd", strpid);
    read_dir(filename, fall_out);

    sprintf(filename, "/proc/%s/io", strpid);
    read_file(filename, fall_out);

    sprintf(filename, "/proc/%s/comm", strpid);
    read_file(filename, fall_out);

    sprintf(filename, "/proc/%s/root", strpid);
    read_symlink(filename, fall_out);

    // sprintf(filename, "/proc/%s/statm", strpid);
    // read_file(filename, fall_out);

    sprintf(filename, "/proc/%s/task", strpid);
    read_dir(filename, fall_out);

    FILE *fsmaps_out = fopen("./savedata/smaps.txt", "w");
    if (fsmaps_out == NULL) {
        perror("fopen2");
        exit(EXIT_FAILURE);
    }
    sprintf(filename, "/proc/%s/smaps", strpid);
    fprintf(fsmaps_out, "size resident shared text lib data dt\n");
    read_file(filename, fsmaps_out);
    fclose(fsmaps_out);

    // sprintf(filename, "/proc/%s/pagemap", strpid);
    // read_file(filename, fall_out);

    FILE *fmaps_out = fopen("./savedata/maps.txt", "w");
    if (fmaps_out == NULL) {
        perror("fopen2");
        exit(EXIT_FAILURE);
    }
    sprintf(filename, "/proc/%s/maps", strpid);
    read_maps(filename, fmaps_out);
    fclose(fmaps_out);

    fclose(fall_out);
    
    return 0;
}