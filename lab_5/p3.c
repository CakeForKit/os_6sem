#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() 
{
    FILE *f1, *f2;
    f1 = fopen("f.txt","w");
    f2 = fopen("f.txt","w");
    for(char c = 'a'; c <= 'z'; c++) {
        if (c%2)
            fprintf(f1, "%c", c);
        else
            fprintf(f2, "%c", c);
    }
    printf("fclose(f1); fclose(f2);\n");
    fclose(f1);
    fclose(f2);
    return 0;
}