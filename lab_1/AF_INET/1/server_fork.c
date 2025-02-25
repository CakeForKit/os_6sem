#include "mystructs.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>



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

int f_sig = 1;

void sigint_handler(int sig_numb) {
    printf("signal=%d\n", sig_numb);
    f_sig = 0;
    exit(EXIT_FAILURE);
}

void sigchld_handler(int sig_numb) {
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {}
    return;
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

int init_shm(key_t key, char **bufout) {
    int shmid = shmget(key, 1, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (shmid == -1) {
        perror("shmid"); 
        exit(EXIT_FAILURE);
    }
    char *buf = shmat(shmid, NULL, 0);
    if (buf == (void *) -1){
        perror("shmat"); 
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
    *bufout = buf;
    return shmid;
}


int main(int argc, char *argv[]) {
    key_t key;
    char *buf;
    int shmid, semid;
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        perror("signal"); 
        exit(EXIT_FAILURE);
    }
    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        perror("signal"); 
        exit(EXIT_FAILURE);
    }
    key = ftok(argv[0], 2);
    if (key == -1){
        perror("ftok"); 
        exit(EXIT_FAILURE);
    }
    semid = init_sem(key);
    shmid = init_shm(key, &buf);
    int listenfd, connfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
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
    struct sockaddr cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    pid_t childpid;
    int type_req;
    while (f_sig) {
        if ((connfd = accept(listenfd, &cli_addr, &clilen)) == -1) {
            perror("accept");
            if (errno == EINTR)
                continue;
            else
                exit(EXIT_FAILURE);
        }
        if ((childpid = fork()) == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (childpid == 0) {
            close(listenfd);

            if (read(connfd, &type_req, sizeof(type_req)) <= 0) {
                perror("read1");
                exit(EXIT_FAILURE);
            }
            switch (type_req)
            {
            case READER:
                // start_read
                if (semop(semid, start_read, 5) == -1) {
                    perror("semop"); 
                    exit(EXIT_FAILURE);
                }
                if (write(connfd, buf, len_buf + 1) == -1) {
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
            case WRITER:
                int ind, can_write;
                if (read(connfd, &ind, sizeof(ind)) <= 0) {
                    perror("read");
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
                if (write(connfd, &can_write, sizeof(can_write)) == -1) {
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
            close(connfd);
            exit(EXIT_SUCCESS);
        }
        close(connfd);
    }
    close(listenfd);
    if (shmdt(buf) == -1) {
        perror("shmdt"); 
        exit(EXIT_FAILURE);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl"); 
        exit(EXIT_FAILURE);
    }
    return 0;
}