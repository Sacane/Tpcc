#include "../src/symbols-table.h"


void fun(int c){
    c = 10;
}

int main(int argc, char const *argv[]){
    fun('o');

    return 0;
}