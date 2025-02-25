#include "mystructs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define serv_ipaddr "127.0.0.1"

int sock_connect() {
    int sockfd;
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
    return sockfd;
}

int main(void) {
    int sockfd, type_req;
    char buf[len_buf + 1];
    int ind, can_write, femptycell;
    femptycell = 1;
    while (femptycell) {
        sockfd = sock_connect();
        type_req = READER;
        if (write(sockfd, &type_req, sizeof(type_req)) == -1) {
            perror("write1");
            exit(EXIT_FAILURE);
        }
        if (read(sockfd, buf, len_buf + 1) == -1) {
            perror("read1");
            exit(EXIT_FAILURE);
        }
        close(sockfd);
        printf("R: read %s\n", buf);
        ind = -1;
        for (size_t i = 0; ind == -1 && i < len_buf; ++i)
            if (buf[i] != '_')
                ind = i;
        if (ind == -1) {
            femptycell = 0;
        } else {
            sockfd = sock_connect();
            type_req = WRITER;
            if (write(sockfd, &type_req, sizeof(type_req)) == -1) {
                perror("write1");
                exit(EXIT_FAILURE);
            }
            sleep(rand() % 4);
            if (write(sockfd, &ind, sizeof(ind)) == -1) {
                perror("write2");
                exit(EXIT_FAILURE);
            }
            printf("W: ind %d\n", ind);
            if (read(sockfd, &can_write, sizeof(can_write)) == -1) {
                perror("read2");
                exit(EXIT_FAILURE);
            }
            if (can_write) 
                printf("W: write %c\n", buf[ind]);
            else 
                printf("W: can not write\n"); 
            close(sockfd);   
        }      
    }
    return 0;
}