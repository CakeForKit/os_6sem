#include "mystructs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

int fd;
int f_sig = 1;

void handler(int sig_numb) {
    printf("signal=%d\n", sig_numb);
    f_sig = 0;
    close(fd);
    unlink(serv_file);
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
    struct sockaddr serv_addr;
    serv_addr.sa_family = AF_UNIX;
    strcpy(serv_addr.sa_data, serv_file);
    if (bind(fd, &serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    struct msg_t msg;
    double res;
    int rc = 0;
    struct sockaddr cli_addr;
    socklen_t socklen = sizeof(cli_addr);
    while(f_sig) {
        if (recvfrom(fd, &msg, sizeof(msg), 0, &cli_addr, &socklen) == -1) {
            perror("recv");
            f_sig = 0, rc = 1;
            continue;
        }
        switch (msg.op)
        {
        case PLUS:
            res = msg.a + msg.b;
            break;
        case MINUS:
            res = msg.a - msg.b;
            break;
        case MUL:
            res = msg.a * msg.b;
            break;
        case DIV:
            if (msg.b == 0)
                res = 0;
            else
                res = msg.a / msg.b;    
            break;
        default:
            res = 0;
            printf("Error operation\n");
            break;
        }
        printf("%.1f %c %.1f = %.1f\n", msg.a, ops[msg.op], msg.b, res);
        if (sendto(fd, &res, sizeof(res), 0, &cli_addr, socklen) == -1) {
            printf("no server\n");
            f_sig = 0, rc = 1;
            continue;
        }
    }
    close(fd);
    unlink(serv_file);
    return rc;
}