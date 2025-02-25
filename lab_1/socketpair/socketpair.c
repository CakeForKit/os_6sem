#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define N 20

int main(void) {
    pid_t pid;
    int fdsock[2], wstatus;
    char buf[N], text[N];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fdsock) == -1) {
        perror("socketpair");
        exit(-1);
    }
    if ((pid = fork()) == -1) {
        perror("fork");
        exit(-1);
    } else if (pid == 0) {
        sprintf(text, "Msg from child");
        printf("Child send: %s\n", text);
        write(fdsock[0], text, strlen(text));
        usleep(200);
        read(fdsock[0], &buf, N);
        printf("Child recieve: %s\n", buf);
        exit(0);
    } else {
        sprintf(text, "Msg from parent");
        printf("Parent send: %s\n", text);
        write(fdsock[1], text, strlen(text));
        usleep(200);
        read(fdsock[1], &buf, N);
        printf("Parent recieve: %s\n", buf);
    }
    if (wait(&wstatus) == -1) {
        perror("wait");
        exit(-1);
    }
    return 0;
}




