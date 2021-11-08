/*#include <stdio.h>
#include <stdlib.h>*/

//test

int ppcm(int a, int b){
    int c, d;
	c=a;
	d=b;

    while (a!=b){   
        if(a>b)
          b=d+b;
        else if(a<b)
            a=c+a;
    }
    
    return a;
}