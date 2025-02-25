#include "mystructs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Error. Usage: %s num1 operation num 2\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    struct msg_t msgsend;
    if (sscanf(argv[1], "%d", &msgsend.a) != 1) {
        perror("scanf");
        exit(EXIT_FAILURE);
    }
    if (strcmp(argv[2], "+") == 0)
        msgsend.op = PLUS;
    else if (strcmp(argv[2], "-") == 0)
        msgsend.op = MINUS;
    else if (strcmp(argv[2], "*") == 0)
        msgsend.op = MUL;
    else if (strcmp(argv[2], "/") == 0)
        msgsend.op = DIV;
    else {
        printf("Error. Operation: +, -, *, /\n");
        exit(EXIT_FAILURE);
    }
    if (sscanf(argv[3], "%d", &msgsend.b) != 1) {
        perror("scanf");
        exit(EXIT_FAILURE);
    }
    int fd;
    if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr serv_addr;
    serv_addr.sa_family = AF_UNIX;
    strcpy(serv_addr.sa_data, serv_file);
    if (sendto(fd, &msgsend, sizeof(msgsend), 0, &serv_addr, sizeof(serv_addr)) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }
    struct sockaddr cli_addr;
    cli_addr.sa_family = AF_UNIX;
    strcpy(cli_addr.sa_data, cli_socket);
    if (bind(fd, &cli_addr, sizeof(cli_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    int msgrecv;
    if (recv(fd, &msgrecv, sizeof(msgrecv), 0) == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    printf("Recive: %d\n", msgrecv);
    close(fd);
    return 0;
}



