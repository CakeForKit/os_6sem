#ifndef MYSTRUCTS__
#define MYSTRUCTS__

#define serv_file "./serv_file"
#define PLUS 0
#define MINUS 1
#define MUL 2
#define DIV 3
char ops[] = {'+', '-', '*', '/'};

struct msg_t {
    double a;
    double b;
    int op;
};

#endif // MYSTRUCTS__