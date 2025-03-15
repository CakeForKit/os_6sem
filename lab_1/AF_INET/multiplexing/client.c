#include "mystructs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

int sock_connect() {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if (connect(sockfd, &serv_addr, sizeof(serv_addr)) == -1) {
        printf("no server\n");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("ERR: use %s t/m\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char srvtype = argv[1][0], fname[40];
    int ftime;
    if (srvtype == 't') 
        sprintf(fname, "data/thr_cli_time_%d.txt", getpid());
    else if (srvtype == 'm')
        sprintf(fname, "data/m_cli_time_%d.txt", getpid());
    else {
        printf("ERR: use %s t/m\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    ftime = open(fname, O_CREAT | O_WRONLY, 00777);
    if (ftime == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    int sockfd;
    char buf[len_buf + 1], type_req, strtime[10];
    int ind, can_write, femptycell;
    clock_t beg, end, t = 0;
    femptycell = 1;
    while (femptycell) {
        beg = clock();
        sockfd = sock_connect();
        pid_t pid = getpid();
        if (srvtype == 't' && write(sockfd, &pid, sizeof(pid)) == -1) {
            perror("write3");
            exit(EXIT_FAILURE);
        }
        type_req = 'r';
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
            if (srvtype == 't' && write(sockfd, &pid, sizeof(pid)) == -1) {
                perror("write3");
                exit(EXIT_FAILURE);
            }
            type_req = 'w';
            if (write(sockfd, &type_req, sizeof(type_req)) == -1) {
                perror("write1");
                exit(EXIT_FAILURE);
            }
            sleep(rand() % 5);
            if (write(sockfd, &ind, sizeof(ind)) == -1) {
                perror("write2");
                exit(EXIT_FAILURE);
            }
            printf("W: send ind %d\n", ind);
            if (read(sockfd, &can_write, sizeof(can_write)) == -1) {
                perror("read2");
                exit(EXIT_FAILURE);
            }
            close(sockfd);   
            if (can_write) 
                printf("W: write %c\n", buf[ind]);
            else 
                printf("W: char is occupied\n"); 
            end = clock();
            t = end - beg;
            sprintf(strtime, "%ld\n", t);
            if (write(ftime, strtime, strlen(strtime)) == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }      
    }
    close(ftime);
    return 0;
}