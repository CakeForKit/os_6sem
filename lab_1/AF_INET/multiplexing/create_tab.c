#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#define N 100

char *mcli_str = "m_cli_time.txt";
char *mserv_str = "m_serv_time.txt";
char *thrcli_str = "thr_cli_time.txt";
char *thrserv_str = "thr_serv_time.txt";


int main(void) {
    int mcli_fd, mserv_fd, thrcli_fd, thrserv_fd;
    int mcli_data[N], mserv_data[N], thrcli_data[N], thrserv_data[N];
    if ((mcli_fd = open(mcli_str, O_RDONLY)) == -1) {
        perror("open mcli_fd");
        exit(EXIT_FAILURE);
    }
    if ((mserv_fd = open(mserv_str, O_RDONLY)) == -1) {
        perror("open mserv_fd");
        exit(EXIT_FAILURE);
    }
    if ((thrcli_fd = open(thrcli_str, O_RDONLY)) == -1) {
        perror("open thrcli_fd");
        exit(EXIT_FAILURE);
    }
    if ((thrserv_fd = open(thrserv_str, O_RDONLY)) == -1) {
        perror("open thrserv_fd");
        exit(EXIT_FAILURE);
    }


    close(mcli_fd);
    close(mserv_fd);
    close(thrcli_fd);
    close(thrserv_fd);
}