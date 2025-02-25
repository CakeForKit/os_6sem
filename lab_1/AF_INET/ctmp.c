#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define serv_port 9877
#define len_buf 10
#define serv_ipaddr "127.0.0.1"

int main(void) {
    int sockfd;
    char buf[len_buf + 1];
    int ind, can_write, emptycell = 1;
    char alpha = 'a', getch;
    
    while (emptycell) {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(serv_port);
        if (inet_pton(AF_INET, serv_ipaddr, &serv_addr.sin_addr) != 1) {
            perror("inet_pton");
            exit(EXIT_FAILURE);
        }
        if (connect(sockfd, &serv_addr, sizeof(serv_addr)) == -1) {
            printf("no server\n");
            exit(EXIT_FAILURE);
        }

        if (read(sockfd, buf, len_buf + 1) <= 0) {
            perror("read1");
            exit(EXIT_FAILURE);
        }
        printf("R: read %s\n", buf);
        ind = -1;
        for (size_t i = 0; ind == -1 && i < len_buf; ++i)
            if (buf[i] != '_')
                ind = i;
        if (ind == -1) {
            emptycell = 0;
            ind = len_buf - 1;
        }
        sleep(rand() % 4);
        // usleep(rand() % 2000 + 1e6);
        if (write(sockfd, &ind, sizeof(ind)) == -1) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        printf("W: ind %d\n", ind);
        if (read(sockfd, &can_write, sizeof(can_write)) <= 0) {
            perror("read2");
            exit(EXIT_FAILURE);
        }
        if (can_write) {
            if (read(sockfd, &getch, 1) <= 0) {
                perror("read2");
                exit(EXIT_FAILURE);
            }
            printf("W: write %c\n", getch);
        } else {
            printf("W: can not write\n");
        }
            
    }
    close(sockfd);
    return 0;
}