#include "mystructs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <time.h>

#define active_writer       0
#define numb_of_readers  1
#define writers_queue    2
#define readers_queue   3
#define bin_sem 4

struct sembuf start_read[] = {
    {readers_queue, 1, 0},
    {active_writer, 0, 0},
    {writers_queue, 0, 0},
    {numb_of_readers, 1, 0},
    {readers_queue, -1, 0}
};
struct sembuf stop_read[] = { {numb_of_readers, -1, 0} };

struct sembuf start_write[] = {
    {writers_queue, 1, 0},
    {numb_of_readers, 0, 0},
    {bin_sem, -1, 0},
    {active_writer, 1, 0},
    {writers_queue, -1, 0}
};
struct sembuf stop_write[] = { {active_writer, -1, 0}, {bin_sem, 1, 0} };

int semid;
char buf[len_buf + 1];
int f_sig = 1;
#define max_events 10

void sigint_handler(int sig_numb) {
    printf("signal=%d\n", sig_numb);
    f_sig = 0;
    exit(EXIT_FAILURE);
}

int init_sem(key_t key) {
    int semid = semget(key, 5, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (semid == -1) {
        perror("semget"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, active_writer, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, numb_of_readers, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, writers_queue, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, readers_queue, SETVAL, 0) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    if (semctl(semid, bin_sem, SETVAL, 1) == -1) {
        perror("semctl"); 
        exit(EXIT_FAILURE);
    }
    return semid;
}

int main(int argc, char *argv[]) {
    key_t key;
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal"); 
        exit(EXIT_FAILURE);
    }
    key = ftok(argv[0], 2);
    if (key == -1){
        perror("ftok"); 
        exit(EXIT_FAILURE);
    }
    semid = init_sem(key);
    int listenfd, connfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    char alpha = 'a';
    for (size_t i = 0; i < len_buf; ++i) {
        buf[i] = alpha;
        if (alpha == 'z')
            alpha = 'a';
        else
            alpha++;
    }
    buf[len_buf] = 0;
    // printf("bufger: %s\n", buf);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(serv_port);
    if (bind(listenfd, &serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    if (listen(listenfd, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev, events[max_events];
    int nfds, epollfd;
    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &ev) == -1) {
        perror("epoll_ctl: listenfd");
        exit(EXIT_FAILURE);
    }
    int ftime = open("data/m_serv_time.txt", O_WRONLY);
    if (ftime == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    clock_t beg, end, t;
    char strtime[10];
    struct sockaddr cli_addr;
    socklen_t clilen = sizeof(struct sockaddr);
    while (f_sig) {
        nfds = epoll_wait(epollfd, events, max_events, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }
        beg = clock();
        for (size_t i = 0; i < nfds; ++i) {
            if (events[i].data.fd == listenfd) {
                connfd = accept(listenfd, (struct sockaddr *) &cli_addr, &clilen);
                if (connfd == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = connfd;
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd,
                            &ev) == -1) {
                    perror("epoll_ctl: connfd");
                    exit(EXIT_FAILURE);
                }
            } else {
                int sockfd = events[i].data.fd;
                char type_req;
                if (read(sockfd, &type_req, sizeof(type_req)) <= 0) {
                    perror("read1");
                    exit(EXIT_FAILURE);
                }
                switch (type_req)
                {
                case 'r':
                    // start_read
                    if (semop(semid, start_read, 5) == -1) {
                        perror("semop"); 
                        exit(EXIT_FAILURE);
                    }
                    if (write(sockfd, buf, len_buf + 1) == -1) {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                    // printf("%s\n", buf);
                    if (semop(semid, stop_read, 1) == -1) {
                        perror("semop"); 
                        exit(EXIT_FAILURE);
                    }
                    // stop_read
                    break;
                case 'w':
                    int ind, can_write;
                    if (read(sockfd, &ind, sizeof(ind)) <= 0) {
                        // printf("rc = %d\n", errno);
                        perror("read2");
                        exit(EXIT_FAILURE);
                    }
                    // start_write
                    if (semop(semid, start_write, 5) == -1) {
                        perror("semop"); 
                        exit(EXIT_FAILURE);
                    }
                    if (buf[ind] == '_') 
                        can_write = 0;
                    else {
                        can_write = 1;
                        buf[ind] = '_';
                    }
                    if (write(sockfd, &can_write, sizeof(can_write)) == -1) {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }
                    if (semop(semid, stop_write, 2) == -1){
                        perror("semop"); 
                        exit(EXIT_FAILURE);
                    }
                    // stop_write
                    break;
                default:
                    break;
                }
                close(sockfd);
                end = clock();
                t = end - beg;
                sprintf(strtime, "%ld\n", t);
                if (write(ftime, strtime, strlen(strtime)) == -1) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}
