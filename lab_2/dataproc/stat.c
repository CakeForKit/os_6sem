#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#define PATH_LEN 1000
#define BUF_SIZE 1024

void get_stat_info(const char *proc, FILE *out)
{
    char path[PATH_LEN];
    snprintf(path, PATH_LEN, "/proc/%s/stat", proc);
    FILE *stat = fopen(path, "r");
    char *headers[] = {"pid", "comm", "state", "ppid", "pgrp", "session", "tty_nr", "tpgid", "flags", "minflt", "cminflt", "majflt", "cmajflt", "utime", "stime", "cutime", "cstime", "priority", "nice", "num_threads", "itrealvalue", "starttime", "vsize", "rss", "rsslim", "startcode", "endcode", "startstack", "kstkesp", "kstkeip", "signal", "blocked", "sigignore", "sigcatch", "wchan", "nswap", "cnswap", "exit_signal", "processor", "rt_priority", "policy", "delayacct_blkio_ticks", "guest_time", "cguest_time", "start_data", "end_data", "start_brk", "arg_start", "arg_end", "env_start", "env_end", "exit_code"};
    char buf[BUF_SIZE + 1] = "\0";
    int len;
    while ((len = fread(buf, 1, BUF_SIZE, stat)) > 0) {
        for (int i = 0; i < len; i++)
            if (buf[i] == 0)
                buf[i] = '\n';
        buf[len] = '\0';
        char *save_row;
        char *row = strtok_r(buf, " ", &save_row);
        int i = 0;
        while (row) {
            fprintf(out, "%s  %s\n", headers[i], row);
            row = strtok_r(NULL, " ", &save_row);
            i++;
        }
    }
    fclose(stat);
}

int main(int argc, char **argv) {
    if (argc != 2) {
       printf("ERR: need %s pid", argv[0]);
       exit(EXIT_FAILURE);
    }
    FILE *file = fopen("./savedata/stat.txt", "w");
    if (!file) {
       perror("fopen");
       exit(EXIT_FAILURE);
    }
    get_stat_info(argv[1], file);
    fclose(file);
}