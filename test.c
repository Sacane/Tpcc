#include <stdio.h>
#include <stdlib.h>

int daioj;

void print(char c){
    printf("%c\n", c);
}

int main(void){
    char df;
    int a;
    a = 45;
    
    switch(a){

        default:
            print('p');
            

        case 10:
            print('a');
            print('b');
            break;
        case 20:
            print('A');
    }
    return 0;
}

