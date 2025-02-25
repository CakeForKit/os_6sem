#ifndef MYSTRUCTS__
#define MYSTRUCTS__

#define serv_socket "./serv_socket"
#define cli_socket "./cli_socket"
#define PLUS 0
#define MINUS 1
#define MUL 2
#define DIV 3

struct msg_t {
    int a;
    int b;
    int op;
};

#endif // MYSTRUCTS__