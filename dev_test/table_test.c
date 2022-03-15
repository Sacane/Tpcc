#include "../src/symbols-table.h"


    


int main(int argc, char const *argv[]){
    Symbol_table *table = create_symbol_table("First table");

    Symbol s1 = create_symbol("count", VARIABLE, INT_TYPE);
    
    print_symbol(s1);

    int res = insert_symbol_in_table(s1, table);
    (res == 1) ? printf("insert success\n") : printf("insert failed\n");
    res = insert_symbol_in_table(s1, table);
    (res == 1) ? printf("insert success\n") : printf("insert failed\n");

    

    return 0;
}