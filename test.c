#include <stdio.h>
#include <stdlib.h>
int r1, r2;

int useParam(char a){
    putchar(a);
    return 1;
}

void tmp(void){
    int b;
    b = 2;

    useParam(b);
}


int trinome(int a,int b, int c) {
    int d;
    int s;
    d=b*b-4*a;
        printf("%d\n", (d));

        r1=(b+s)/(2*a);
        r2=(b-s)/(2*a);
        return 1;
    

    return 0;
}

int switchret(void){
        int a;
        a = getint();
        switch(a){
                case 0:
                        return 1;
                case 1:
                        return 2;
                default:
                        return 2;
        }
}

int main(void){
        return 0;
}