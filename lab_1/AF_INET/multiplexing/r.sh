#!/bin/bash

rm client_thread, server_thread, server_epoll
gcc server_epoll.c -o server_epoll 
gcc client.c -o client
gcc server_thread.c -o server_thread -D_GNU_SOURCE
