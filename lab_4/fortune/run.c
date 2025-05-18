#include <stdio.h>
#include <sys/syscall.h>

int main() {
    printf("pid = %d\n", getpid());
    printf("TID (task_struct->pid) = %ld\n", syscall(SYS_gettid));
    while (1);
}