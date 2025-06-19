/*
Пример: файл открывается два раза системным вызовом open() для записи и 
в него последовательно записывается строка «аааааааааааа» по первому дескриптору и 
затем строка «вввв» по второму дескриптору, затем файл закрывается два раза. 
Показать, что будет записано в файл и пояснить результат. (71-78)*/
#include <fcntl.h>
#include <unistd.h>

int main()  {
    int fd1 = open("f.txt", O_CREAT | O_RDWR);
    int fd2 = open("f.txt",O_RDWR);
    char *a = "aaaaaaaaaaa", *b = "bbbbb";
    write(fd1, a, strlen(a)); write(fd2, b, strlen(b));
    close(fd1); close(fd2); return 0;
}

void *thread_func(void *arg) {
    int fd = *(int *)arg;
    for (int i = 0; i < 5; ++i) {
        write(fd, "b", 1); }
    return NULL; }
int main() {
    int fd1 = open("f.txt", O_TRUNC | O_APPEND | O_RDWR);
    int fd2 = open("f.txt", O_APPEND | O_RDWR);
    pthread_t t;
    if (pthread_create(&t, NULL,thread_func,&fd1)) {
        perror("pthread_create");
        exit(1); }
    for (int i = 0; i < 10; ++i) {
        write(fd1, "a", 1); }
    pthread_join(t, NULL);
    close(fd1);
    return 0; }