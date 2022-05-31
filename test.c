#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int daioj;

void print(char c){
    printf("%c\n", c);
}

int main(void){
    char c;
    int i, j;
    i = 0;
    while(i < 10){
        c = getchar();
        c = getchar();
        printf("i : %d\n", i);
        i+= 1;
    }
    return 1;
}

