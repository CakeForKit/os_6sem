#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define serv_file "./serv_file"

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Error. Usage: %s num1 operation num 2\n", argv[0]);
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
    char buf[20];
    strcpy(buf, argv[1]);
    if (sendto(fd, &buf, strlen(buf) + 1, 0, &serv_addr, sizeof(serv_addr)) == -1) {
        printf("no server\n");
        exit(EXIT_FAILURE);
    }
    close(fd);
    return 0;
}



