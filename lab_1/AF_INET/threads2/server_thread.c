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
#include <pthread.h>
#include <sched.h>

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

struct thrdata_t {
    int sockfd;
    int cpuID;
};

void *func_thread(void *arg) {
    struct thrdata_t *thrdata = (struct thrdata_t *)arg;
    int sockfd = thrdata->sockfd;
    int cpuID = thrdata->cpuID;
    free(arg);

    int num = sysconf(_SC_NPROCESSORS_CONF);
    if (cpuID < 0 || cpuID >= num) {
        fprintf(stderr, "cpuID %d is out of range (0 - %d). Using 0 instead.\n", cpuID, num - 1);
        cpuID = 0;
    }
    cpu_set_t mask;
    cpu_set_t get;
    CPU_ZERO(&mask);
    CPU_SET(cpuID, &mask);
    int s = pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
    if (s != 0)
        fprintf(stderr, "pthread_setaffinity_np error: %s\n", strerror(s));
    CPU_ZERO(&get);
    s = pthread_getaffinity_np(pthread_self(), sizeof(get), &get);
    if (s != 0)
        fprintf(stderr, "pthread_getaffinity_np error: %s\n", strerror(s));
    printf("thread: %lu, ", (unsigned long)pthread_self());
    struct sockaddr_in peer;
    socklen_t len = sizeof(peer);
    if (getpeername(sockfd, (struct sockaddr *)&peer, &len) != -1)
        printf("cli_port: %d, ", ntohs(peer.sin_port));
    else
        perror("getpeername");
    for (int i = 0; i < num; i++)
        if (CPU_ISSET(i, &get))
            printf("CPU: %d ", i);
    printf("\n");




    if (pthread_detach(pthread_self()) != 0) {
        perror("pthread_detach");
        return NULL;
    }
    char type_req;
    if (read(sockfd, &type_req, sizeof(type_req)) <= 0) {
        perror("read1");
        pthread_exit(0);
    }
    switch (type_req)
    {
    case 'r':
        // start_read
        if (semop(semid, start_read, 5) == -1) {
            perror("semop"); 
            pthread_exit(0);
        }
        if (write(sockfd, buf, len_buf + 1) == -1) {
            perror("write");
            pthread_exit(0);
        }
        // printf("%s\n", buf);
        if (semop(semid, stop_read, 1) == -1) {
            perror("semop"); 
            pthread_exit(0);
        }
        // stop_read
        break;
    case 'w':
        int ind, can_write;
        if (read(sockfd, &ind, sizeof(ind)) <= 0) {
            perror("read");
            pthread_exit(0);
        }
        // start_write
        if (semop(semid, start_write, 5) == -1) {
            perror("semop"); 
            pthread_exit(0);
        }
        if (buf[ind] == '_') 
            can_write = 0;
        else {
            can_write = 1;
            buf[ind] = '_';
        }
        if (write(sockfd, &can_write, sizeof(can_write)) == -1) {
            perror("write");
            pthread_exit(0);
        }
        if (semop(semid, stop_write, 2) == -1){
            perror("semop"); 
            pthread_exit(0);
        }
        // stop_write
        break;
    default:
        break;
    }
    close(sockfd);
    return NULL;
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

    pthread_t tid;
    struct thr_data_t *thr_data;
    struct sockaddr cli_addr;
    socklen_t clilen = sizeof(struct sockaddr);
    int num = sysconf(_SC_NPROCESSORS_CONF);
    int i = 0;
    while (f_sig) {
        if ((connfd = accept(listenfd, &cli_addr, &clilen)) == -1) {
            perror("accept");
            if (errno == EINTR)
                continue;
            else
                exit(EXIT_FAILURE);
        }
        struct thrdata_t *thrdata = malloc(sizeof(struct thrdata_t));
        if (thrdata == NULL) {
            perror("malloc");
            exit(EXIT_SUCCESS);
        }
        thrdata->sockfd = connfd;
        thrdata->cpuID = i;
        int rc;
        if ((rc = pthread_create(&tid, NULL, func_thread, (void *) thrdata)) != 0) {
            printf("rc = %d\n", rc);
            perror("pthread_create");
            sleep(4);
            exit(EXIT_FAILURE);
        }
        i = rand() % num;
    }
}
