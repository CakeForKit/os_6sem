#!/bin/bash

rm client_thread, server_thread
gcc client_thread.c -o client_thread && gcc server_thread.c -o server_thread