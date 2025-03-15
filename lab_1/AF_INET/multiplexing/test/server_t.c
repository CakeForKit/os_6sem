#define _POSIX_C_SOURCE 200112L
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <time.h>

#include "structs.h"

int sfd, semid;
char arr[ARR_SIZE];

//long long  avg_time = 0;

#define READER_QUEUE 0
#define WRITER_QUEUE 1
#define READER 2
#define WRITER 3
#define SHADOW_WRITER 4

// struct my_arg_type
// {
//     int connfd;
//     int timestart;
// };

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


void* serv_client_request(void *args)
{
    unsigned index;
    int connfd = *((int *)args);
    char buf[BUF_SIZE];
    int status, is_full;

    printf("(%d)Got new connection!\n", getpid());

    while (1)
    {
        if (recv(connfd, buf, BUF_SIZE, 0) <= 0)
        {
            printf("(%d)Server finished\n", getpid());
            break;
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
                if (send(connfd, &buf, sizeof(buf), 0) == -1)
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
                    printf("(%d)Client reserved letter %u\n", getpid(), index);
                }
                else
                {
                    if (arr[index] == '-')
                    {
                        status = ALREADY_RESERVED;
                        printf("(%d)Client failed to reserve letter %u\n", getpid(), index);
                    }
                    else
                    {
                        status = ERROR;
                        printf("(%d)Server received invalid letter number %u\n", getpid(), index);
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
                    
                    printf("(%d)All letters reserved. Shutting down server.\n", getpid());
                    kill(getppid(), SIGINT); // Signal parent process to shutdown
                }
                if (send(connfd, &status, sizeof(status), 0) == -1)
                {
                    perror("Can't send");
                    exit(EXIT_FAILURE);
                }
                if (semop(semid, stop_write, 2) == -1)
                {
                    perror("semop");
                    exit(EXIT_FAILURE);
                }
                break;
        }
    }
    

    //clock_gettime(CLOCK_MONOTONIC, &all_time_end);

    //clock()
    return NULL;
}

void sigint_handler()
{
    close(sfd);
    semctl(semid, 5, IPC_RMID, NULL);
    //avg_time/=ARR_SIZE;
    //printf("\nSERVER: Average time elapsed = %lld ns\n\n",avg_time);

    exit(EXIT_SUCCESS);
}

int main(void)
{
    int listenfd, connfd;
    socklen_t client_len;
    struct sockaddr_in client_addr, serv_addr;
    pthread_t th;

    struct timespec all_time_start, all_time_end;
    // long long elapsed_time_ns;
    // long long output_time_ns = 0;
    // struct timespec output_time_start, output_time_end;

    int cnt;

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

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("Can't socket");
        exit(EXIT_FAILURE);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  //Host To Network Long
    serv_addr.sin_port = htons(SERV_PORT);  //Host To Network Short

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("Can't bind");
        exit(EXIT_FAILURE);
    }
    if (listen(listenfd, 1024) == -1)
    {
        perror("Can't listen");
        exit(EXIT_FAILURE);
    }
    printf("listening on port %d\n", SERV_PORT);
    while (1)
    {
        client_len = sizeof(client_addr);
        //struct my_arg_type args;

        clock_gettime(CLOCK_MONOTONIC, &all_time_start);

        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_len);

        //args.connfd = connfd;
        if (pthread_create(&th, NULL, serv_client_request, &connfd) != 0)
        {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        if (pthread_detach(th))
        {
            perror("pthread_detach");
            exit(EXIT_FAILURE);
        }
        

        //avg_time += all_time_end.tv_nsec - all_time_start.tv_nsec;
        //cnt++;
    }
    if (semctl(semid, 5, IPC_RMID, NULL) == -1)
    {
        perror("semclt");
        exit(EXIT_FAILURE);
    }
}
