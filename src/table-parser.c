#include "table-parser.h"


List init_table_list(Symbol_table *table){
    List ls_table;
    
    if(!(ls_table = (List)malloc(sizeof(Table)))){
        fprintf(stderr, "Error while allocating list of symbol table");
        return NULL; 
    }

    ls_table->table = table;
    ls_table->next = NULL;

    return ls_table;

}

int insert_table(List list, Symbol_table *table){

    List tmp;
    tmp = list;

    if(tmp->table == NULL){
        tmp->table = table;
        return 1;
    }


    for(tmp; tmp->next; tmp = tmp->next){}
    tmp->next = init_table_list(table);

    if(!tmp->next){
        fprintf(stderr, "Error while insert the table\n");
        return 0;
    }

    return 1;
}

void print_chained_list(List lst){
    printf("PRINT\n");
    List tmp = lst;

    while(tmp != NULL){
        print_symbol_table(tmp->table);
        tmp = tmp->next;
    }
}

List build_function_tables(Node *root){
    List list;
    list = init_table_list(NULL);

    Node* functions_root;
    functions_root = root->firstChild->nextSibling; //On DeclFoncts

    for(Node* function_root = functions_root->firstChild; function_root; function_root = function_root->nextSibling){
        //parse of the DeclFonct


        // =============== Management of the functions's header ==================

        Node* header = function_root->firstChild;
        Node *function_type = header->firstChild;
        Symbol_table *table = create_symbol_table(function_type->firstChild->u.ident);
        
        Node *params = function_type->nextSibling;
        Symbol s;
        if(params->firstChild->label == Void){
            s = create_symbol("void", VOID_KIND, VOID_TYPE);
            insert_symbol_in_table(s, table);
        }else{
            for (Node *paramType = params->firstChild; paramType; paramType = paramType->nextSibling){
                Kind k = VARIABLE;
                Type type = str_to_tpcType(paramType->u.ident);
                Node* id = paramType->firstChild;
                s = create_symbol(id->u.ident, k, type);
                insert_symbol_in_table(s, table);
            }
        }

        // ========================================================================


        Node* body = header->nextSibling;


        //Function's Global variable :
        Node* global = body->firstChild;
        for(Node* global_types = global->firstChild; global_types; global_types = global_types->nextSibling){
            Type type = str_to_tpcType(global_types->u.ident); // type's variable
            Kind kind = VARIABLE;
            for(Node *id = global_types->firstChild; id; id = id->nextSibling){
                s = create_symbol(id->u.ident, kind, type);
                insert_symbol_in_table(s, table);
            }
        }
        insert_table(list, table);
        
    }
    return list;
}


/* Open/close asm file.
 *
 * int mode -> ouverture = 0 | fermeture != 0
 * int fd descripteur du fichier à fermer
*/
int open_close_asm_file(int mode, int fd){

    if(mode == OPEN){
        return open("_anonymous.asm",O_WRONLY|O_APPEND);
    }
    return close(fd);
}

/* Init asm file  with beginning expr or ending expr
 *
 * int mode -> 0 pour debut sinon fin
 * int fd -> descripteur fichier asm
*/
void init_asm_(int mode,FILE* fd){
    char* chaine;
    if(mode == OPEN) chaine = "global  _start\nsection  .text\n_start:\n";
    else chaine = "\tmov rax, 60\n\tmov rdi, 0\n\tsyscall\n";
    fprintf(fd, chaine);
}


/* We suppose node parameter in the body of the main */
void parse_and_apply_substraction(Node* node, FILE *out){
    char *buf;
    if(!node){
        return;
    }
    
    if(node && node->label == Addsub){
        if(node->u.byte == '-'){
            int a, b;
            a = node->firstChild->u.num;
            b = node->firstChild->nextSibling->u.num;
            fprintf(out, "\tpush %d\n\tpush %d\n", a, b);
            fprintf(out, "\tpop r14\n");
            fprintf(out, "\tpop r12\n");
            fprintf(out, "\tsub r12, r14\n");
            fprintf(out, "\tpush r12\n");
            
        }else{
            return;
        }
    }
    else {
        parse_and_apply_substraction(node->firstChild, out);
        parse_and_apply_substraction(node->nextSibling, out);
    }
    
    
}

int treat_simple_sub_in_main(Node *root){
    FILE* out;
    Node* functions = root->firstChild->nextSibling;
    
    out = fopen("_anonymous.asm", "wr");

    init_asm_(OPEN, out);

    for(Node *function = functions->firstChild; function; function = function->nextSibling){
        
        if (!(strcmp(function->firstChild->firstChild->firstChild->u.ident, "main"))){
            printf("MAIN\n");
            parse_and_apply_substraction(function->firstChild->nextSibling, out);
        }

    }

    init_asm_(CLOSE, out);
    fclose(out);


    



    return 1;
}


