#include "mystructs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

int fd;
char cli_file[20]; 

int f_sig = 1;
void handler(int sig_numb) {
    printf("signal=%d\n", sig_numb);
    f_sig = 0;
    close(fd);
    unlink(cli_file);
    exit(EXIT_FAILURE);
}

int main(void) {
    if (signal(SIGINT, handler) == SIG_ERR) {
        perror("signal"); 
        exit(EXIT_FAILURE);
    }
    
    if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    sprintf(cli_file, "%d.cli", getpid());
    struct sockaddr cli_addr;
    cli_addr.sa_family = AF_UNIX;
    strcpy(cli_addr.sa_data, cli_file);
    if (bind(fd, &cli_addr, sizeof(cli_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    struct sockaddr serv_addr;
    serv_addr.sa_family = AF_UNIX;
    strcpy(serv_addr.sa_data, serv_file);
    srand(time(NULL));
    double res;
    struct msg_t msg;
    while (f_sig) {
        msg.a = rand() % 10;
        msg.b = rand() % 10;
        msg.op = rand() % 4;
        if (sendto(fd, &msg, sizeof(msg), 0, &serv_addr, sizeof(serv_addr)) == -1) {
            printf("no server\n");
            close(fd), unlink(cli_file);
            exit(EXIT_FAILURE);
        }
        if (recvfrom(fd, &res, sizeof(res), 0, NULL, NULL) == -1) {
            perror("recv");
            close(fd), unlink(cli_file);
            exit(EXIT_FAILURE);
        }
        printf("%.1f %c %.1f = %.1f\n", msg.a, ops[msg.op], msg.b, res);
        usleep(rand() % 1000 + 2e6);
    }
    close(fd);
    unlink(cli_file);
    return 0;
}



