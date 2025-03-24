#!/bin/bash


gcc dproc.c -o dproc
gcc ./pagemap.c -o pagemap
gcc ./stat.c -o stat

pid=5682

./dproc $pid 
./pagemap $pid
./stat $pid
