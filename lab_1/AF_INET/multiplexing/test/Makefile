# CC := gcc

# .PHONY: build
# build: client server

# %.exe: %.c structs.h
# 	gcc -o $@ $^ -Wall -Werror


CC := gcc

.PHONY: build
build: client_t client server_m server_t

%.exe: %.c structs.h
	gcc -o $@ $^ -Wall -Werror
