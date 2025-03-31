#!/bin/bash


gcc dproc.c -o dproc
gcc ./pagemap.c -o pagemap
gcc ./stat.c -o stat
gcc ./task.c -o task
gcc ./mem.c -o mem

pid=9701

./dproc $pid
./task $pid
./stat $pid
./mem $pid
./pagemap $pid 
