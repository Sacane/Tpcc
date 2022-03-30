#include "asm.h"



void init_global_asm(int size){
    FILE* f = fopen("_anonymous.asm", "w");

    fprintf(f, "section .bss\n");
    fprintf(f, "global_var: resp %d\n", size);


    fclose(f);
}


void write_global_eval(){
    FILE *f = fopen("_anonymous.asm", "a");
    
    

    fclose(f);
}