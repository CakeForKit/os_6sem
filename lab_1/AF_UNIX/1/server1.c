#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define serv_file "./serv_file"
#define sec 7

int fd;

void handler(int sig_numb) {
    printf("signal=%d\n", sig_numb);
    alarm(0);
    close(fd);
    unlink(serv_file);
}

int main(void) {
    if (signal(SIGALRM, handler) == SIG_ERR) {
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
    alarm(sec);
    char buf[10];
    while(1) {
        if (recvfrom(fd, &buf, sizeof(buf), 0, NULL, NULL) == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        printf("%s\n", buf);
    }
    return 0;
}