#define _POSIX_C_SOURCE 200112L
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/sem.h>
#include <sys/epoll.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include "structs.h"

#define MAX_EVENTS 10

int sfd, semid;
char arr[ARR_SIZE];

#define READER_QUEUE 0
#define WRITER_QUEUE 1
#define READER 2
#define WRITER 3
#define SHADOW_WRITER 4

struct sembuf start_read[] = {{READER_QUEUE, 1, 0},
                            {WRITER_QUEUE, 0, 0},
                            {SHADOW_WRITER, 0, 0},
                            {READER, 1, 0},
                            {READER_QUEUE, -1, 0}};

struct sembuf stop_read[] = {{READER, -1, 0}};

struct sembuf start_write[] = {{WRITER_QUEUE, 1, 0},
                            {READER, 0, 0},
                            {WRITER, -1, 0},
                            {SHADOW_WRITER, 1, 0},
                            {WRITER_QUEUE, -1, 0}};

struct sembuf stop_write[] = {{SHADOW_WRITER, -1, 0},
                           {WRITER, 1, 0}};


void serv_client_request(int cl_fd) 
{
    char buf[BUF_SIZE];
    int status, is_full;
    unsigned index;

    printf("(%d) Got new connection!\n", getpid());

    while (1) 
    {
        ssize_t bytes_received = recv(cl_fd, buf, BUF_SIZE, 0);
        if (bytes_received <= 0) 
        {
            printf("(%d) Server finished\n", getpid());
            close(cl_fd);
            return;
        }

        switch (buf[0]) 
        {
            case 'r':
                if (semop(semid, start_read, 5) == -1)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }
                for (size_t i = 0; i < ARR_SIZE; i++)
                {
                    buf[i] = arr[i];
                }
                if (send(cl_fd, &buf, sizeof(buf), 0) == -1)
                {
                    perror("Can't send");
                    exit(EXIT_FAILURE);
                }
                if (semop(semid, stop_read, 1) == -1)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'w':
                {
                    index = (int) buf[1];
                    if (semop(semid, start_write, 5) == -1) 
                    {
                        perror("semop");
                        exit(EXIT_FAILURE);
                    }
                    if (index < ARR_SIZE && arr[index] != '-') 
                    {
                        arr[index] = '-';
                        status = OK;
                        printf("(%d) Client reserved letter %u\n", getpid(), index);
                    } else 
                    {
                        if (arr[index] == '-') 
                        {
                            status = ALREADY_RESERVED;
                            printf("(%d) Client failed to reserve letter %u\n", getpid(), index);
                        } 
                        else 
                        {
                            status = ERROR;
                            printf("(%d) Server received invalid letter number %u\n", getpid(), index);
                        }
                    }
                    is_full = 1;
                    for (size_t i = 0; is_full && i < ARR_SIZE; i++) 
                    {
                        if (arr[i] != '-')
                            is_full = 0;
                    }
                    if (is_full) 
                    {
                        printf("(%d) All letters reserved. Shutting down server.\n", getpid());
                        kill(getppid(), SIGINT); // Signal parent process to shutdown
                    }
                    if (send(cl_fd, &status, sizeof(status), 0) == -1) 
                    {
                        perror("Can't send");
                        exit(EXIT_FAILURE);
                    }
                    if (semop(semid, stop_write, 2) == -1) 
                    {
                        perror("semop");
                        exit(EXIT_FAILURE);
                    }
                }
                break;
                
        }
    }
    //NENNENENENENN
}

void sigint_handler() 
{
    close(sfd);
    semctl(semid, 5, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}


int main(void) 
{
    int listen_sock, conn_sock, nfds, epollfd;
    socklen_t client_len;
    struct sockaddr_in client_addr, serv_addr;
    struct epoll_event ev, events[MAX_EVENTS];

    if (signal(SIGINT, sigint_handler) == (void *)-1) 
    {
        perror("cannot set handler");
        exit(EXIT_FAILURE);
    }
    semid = semget(IPC_PRIVATE, 5, IPC_CREAT | 0666);
    if (semid == -1)
        perror("semget");
    if (semctl(semid, READER_QUEUE, SETVAL, 0) == -1)
        perror("semctl");
    if (semctl(semid, WRITER_QUEUE, SETVAL, 0) == -1)
        perror("semctl");
    if (semctl(semid, READER, SETVAL, 0) == -1)
        perror("semctl");
    if (semctl(semid, WRITER, SETVAL, 1) == -1)
        perror("semctl");
    if (semctl(semid, SHADOW_WRITER, SETVAL, 0) == -1)
        perror("semctl");

    char s = 'a';
    for (size_t i = 0; i < ARR_SIZE; i++)
    {
        arr[i] = s;
        s++;
        if (i % 26 == 0)
            s = 'a';
    }

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1) {
        perror("Can't socket");
        exit(EXIT_FAILURE);
    }


    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //Host To Network Long
    serv_addr.sin_port = htons(SERV_PORT); //Host To Network Short

    if (bind(listen_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) 
    {
        perror("Can't bind");
        exit(EXIT_FAILURE);
    }
    if (listen(listen_sock, 1024) == -1) 
    {
        perror("Can't listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", SERV_PORT);

    epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // Добавляем слушающий сокет в epoll
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    // struct timespec all_time_start, all_time_end;
    // long long elapsed_time_ns;
    // long long output_time_ns = 0;
    // struct timespec output_time_start, output_time_end;

    // long long  avg_time = 0;
    // int cnt;

    while (1) 
    {
        //ТУТ 1
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nfds; i++) 
        {
            if (events[i].data.fd == listen_sock) 
            {
                client_len = sizeof(client_addr);
                
                // clock_gettime(CLOCK_MONOTONIC, &all_time_start);
                //NENNNNNNN
                conn_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_len);
                //ТУТ 2

                if (conn_sock == -1) 
                {
                    perror("accept");
                }
                else{
                    // Добавляем новый сокет клиента в epoll
                    ev.events = EPOLLIN | EPOLLET;;
                    ev.data.fd = conn_sock;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev) == -1) {
                        perror("epoll_ctl: conn_sock");
                        exit(EXIT_FAILURE);
                    }
                }
            } 
            else 
            {
                serv_client_request(events[i].data.fd);

            }

        }
    }

    // Закрываем дескрипторы
    close(epollfd);
    close(listen_sock);
    
    if (semctl(semid, 5, IPC_RMID, NULL) == -1)
    {
        perror("semclt");
        exit(EXIT_FAILURE);
    }

    
    return 0;
}
