#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "structs.h"

int main(int argc, char **argv)
{
    int sockfd, ind;
    char buf[BUF_SIZE];
    struct sockaddr_in servaddr;

    if (argc != 2)
    {
        perror("usage: tcpcli <IPaddress>");
        exit(EXIT_FAILURE);
    }
    while(1)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
        {
            perror("Can't socket");
            exit(EXIT_FAILURE);
        }
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(SERV_PORT); //Host To Network Short
        //преобразование строкового представления IP-адреса в его бинарный формат
        if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) == -1) 
        {
            perror("inet pton");
            exit(EXIT_FAILURE);
        }
        if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
        {
            perror("connection Can't");
            exit(EXIT_FAILURE);
        }
        else
            printf("Connected\n");

        buf[0] = 'r';
        if (send(sockfd, &buf, sizeof(buf), 0) == -1)
        {
            perror("Can't send");
            exit(EXIT_FAILURE);
        }
        if (recv(sockfd, &buf, sizeof(buf), 0) == -1)
        {
            perror("Can't recieve");
            exit(EXIT_FAILURE);
        }
        printf("Recieved letters: ");
        for (size_t i = 0; i < ARR_SIZE; i++)
            printf("%c", buf[i]);
        puts("");
        ind = -1;
        for (int i = 0; i < ARR_SIZE; i++)
        {
            if (buf[i] != '-')
            {
                ind = i;
                break;
            }
        }

        if (ind == -1)
        {
            printf("No availible letters left! Disconnecting from server\n");
            exit(EXIT_FAILURE);
        }

        usleep(500000 + rand() % 500000);

        buf[0] = 'w';
        buf[1] = (char) ind;

        printf("Sending ind %d to server\n", ind);

        if (send(sockfd, &buf, sizeof(buf), 0) == -1)
        {
            perror("Can't send");
            exit(EXIT_FAILURE);
        }

        if (recv(sockfd, &buf, sizeof(buf), 0) == -1)
        {
            perror("Can't recieve");
            exit(EXIT_FAILURE);
        }

        switch ((int) buf[0])
        {
            case OK:
                puts("Response from server: OK\n");
                break;
            case ALREADY_RESERVED:
                puts("Response from server: already reserved\n");
                break;
            default:
                perror("Unexpected response from server");
                exit(EXIT_FAILURE);
        }
        usleep(2000000 + rand() % 1500000);
        close(sockfd);
    }

    exit(EXIT_SUCCESS);
}
