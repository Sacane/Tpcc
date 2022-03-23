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

    List tmp = lst;
    if(!lst){
        return;
    }
    while(tmp != NULL){
        print_symbol_table(tmp->table);
        tmp = tmp->next;
    }
}

List build_function_tables(Node *root){

    List list;
    list = init_table_list(NULL);
    int i = 0;
    int is_void, nb_args = 0;
    Node* functions_root;
    PrimType function_t;
    functions_root = root->firstChild->nextSibling; //On DeclFoncts


    //parse of the DeclFonct
    for(Node* function_root = functions_root->firstChild; function_root; function_root = function_root->nextSibling){
        


        // =============== Management of the functions's header ==================

        Node* header_function = function_root->firstChild;
        PrimType param_types[MAX_ARGUMENT_FUNC];

        Node *function_type = header_function->firstChild;
        is_void = (!(strcmp(function_type->u.ident, "void"))) ? 1 : 0;  

        function_t = str_to_tpcType(function_type->u.ident);

        Symbol_table *table = create_symbol_table(function_type->firstChild->u.ident);

        Node *params = function_type->nextSibling;

        Symbol s;

        if(params->firstChild->label != Void){
            
            for (Node *paramType = params->firstChild; paramType; paramType = paramType->nextSibling){
                //parametres
                Kind k = PARAM;
                PrimType type = str_to_tpcType(paramType->u.ident);
                Node* id = paramType->firstChild;
                s = create_symbol(id->u.ident, k, type);
                insert_symbol_in_table(s, table);
                table->nb_parameter += 1;

                //symbol de structure de fonction
                param_types[nb_args] = type;
                nb_args += 1;
            }
        }


        Symbol params_sym = create_func_sym(function_type->firstChild->u.ident, function_t, param_types, nb_args);
        insert_symbol_in_table(params_sym, table);

   
        // ========================================================================


        Node* body = header_function->nextSibling;
        printf("%d\n", body->label);
        
        //Function's local variable :
        Node* global = body->firstChild;
        for(Node* global_types = global->firstChild; global_types; global_types = global_types->nextSibling){
            PrimType type = str_to_tpcType(global_types->u.ident); // type's variable
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



/**
 * @brief Get the table by name object
 * @warning may return null
 * @param name_table 
 * @param table_list 
 * @return Symbol_table* 
 */
Symbol_table *get_table_by_name(char *name_table, List table_list){
    List tmp = table_list;
    if(!table_list){
        return NULL;
    }
    while(strcmp(tmp->table->name_table, name_table) || tmp != NULL){
        tmp = tmp->next;
    }
    return tmp;
}

int function_call_sem_parser(Node *fc_node, List table){




    return 1;
}

int variable_call_sem_parser(Node *varcall_node, List table){
    
}

/**
 * @brief parse the tree and check if there is a sem error or not
 * 
 * @param n 
 * @param table 
 * @return int 1 if the parse returned no error, 0 if there is at least 1 sem error
 */
int parse_sem_error(Node *n, List table){
    if (!n){
        return 1;
    }
    switch(n->label){
        case FunctionCall:
            return parse_sem_error(n->nextSibling, table) && parse_sem_error(n->firstChild, table);
            break;
        case Variable:
            return parse_sem_error(n->nextSibling, table) && parse_sem_error(n->firstChild, table);
            break;
        case Assign:
            return parse_sem_error(n->nextSibling, table) && parse_sem_error(n->firstChild, table);
            break;
        case Eq:
            return parse_sem_error(n->nextSibling, table) && parse_sem_error(n->firstChild, table);
            break;
        default:
            return parse_sem_error(n->nextSibling, table) && parse_sem_error(n->firstChild, table);
    }
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


