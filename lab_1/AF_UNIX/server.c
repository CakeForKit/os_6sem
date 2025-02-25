#include "mystructs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

// int f_sigint = 1;
int fd;

void handler(int sig_numb) {
    printf("PID=%d signal=%d\n", getpid(),sig_numb);
    close(fd);
    unlink(serv_file);
    exit(EXIT_SUCCESS);
    // f_sigint = 0;
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
    struct sockaddr serv_addr;
    serv_addr.sa_family = AF_UNIX;
    strcpy(serv_addr.sa_data, serv_file);
    if (bind(fd, &serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    struct msg_t msg;
    while(1) {
        if (recv(fd, &msg, sizeof(msg), 0) == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        switch (msg.op)
        {
        case PLUS:
            printf("%d + %d\n", msg.a, msg.b);
            break;
        case MINUS:
            printf("%d - %d\n", msg.a, msg.b);
            break;
        case MUL:
            printf("%d * %d\n", msg.a, msg.b);
            break;
        case DIV:
            printf("%d / %d\n", msg.a, msg.b);
            break;
        default:
            printf("Error operation\n");
            break;
        }
    }
    // close(fd);
    // return 0;
}