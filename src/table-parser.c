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
    printf("List \n");
    while(tmp != NULL){
        print_symbol_table(tmp->table);
        tmp = tmp->next;
    }
}

List build_list_table(Node *root){

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
    
    while(1){

        if(!strcmp(name_table, tmp->table->name_table)){
            return tmp->table;
        }
        if(!tmp) return NULL;
        tmp = tmp->next;
    }
    
    return (tmp) ? tmp->table : NULL;
}

static int check_param_function_call(Symbol_table *function_table, Node *fc_root){

    int i;
    Symbol params = get_symbol_by_name(function_table, function_table->name_table);

    for(Node *n = fc_root->firstChild; n; n = n->nextSibling){
        
        Symbol s = get_symbol_by_name(function_table, n->u.ident);
        if(s.u.p_type != params.u.f_type.args_types[i]){
            return 0;
        }
        i++;
    }
    return 1;
}

static int function_call_sem_parser(Node *fc_node, List table, char *name){

    Symbol_table *global_table = get_table_by_name("global_vars", table);
    Symbol_table *fun_table = get_table_by_name(name, table);
    if(!fun_table){
        return 0;
    }
    if(fun_table->nb_parameter != 0){
        for(Node *n = fc_node->firstChild; n; n = n->nextSibling){
            if(
                n->label == Variable && 
                (!is_symbol_in_table(global_table, n->u.ident) || !is_symbol_in_table(fun_table, n->u.ident)) &&
                !(check_param_function_call(fun_table, fc_node))                
            ){
                return 0;
            }
        }
    }
    
    return 1;
}

static int variable_call_sem_parser(Node *varcall_node, List table, char *name){
    Symbol_table *globals = get_table_by_name("global_vars", table);
    Symbol_table *func = get_table_by_name(name, table);
    int ret_val = is_symbol_in_table(globals, varcall_node->u.ident) || is_symbol_in_table(func, varcall_node->u.ident);
    return ret_val == 1;
}

static int equal_check(Node *eq, List tab, char *name_tab){

    Symbol_table* global_tab;
    Symbol_table* function_tab;
    Node *var1 = eq->firstChild;
    Node *var2 = eq->firstChild->nextSibling;


    global_tab = get_table_by_name("global_vars", tab);
    function_tab = get_table_by_name(name_tab, tab);

    if(var1->label == Variable){
        char* id1 = var1->u.ident;
        if(!(is_symbol_in_table(global_tab,id1)) || !(is_symbol_in_table(function_tab,id1))){
            DEBUG("Error while equals check\n");
            return 0;
        }
            

    }

    if(var2->label == Variable){
        char* id2 = var2->u.ident;
        if(!(is_symbol_in_table(global_tab,id2)) || !(is_symbol_in_table(function_tab,id2))){
            DEBUG("Error while equals check\n");
            return 0;
        }
            
    }

    return 1;
}

int assign_check(Node *assign, List tab, char *name_tab){
    
    Symbol_table *global_tab;
    Symbol_table *function_tab;
    Node *var1 = assign->firstChild;
    int check = 0;
    
    global_tab = get_table_by_name("global_vars", tab);
    function_tab = get_table_by_name(name_tab, tab);
    if(var1->label == Variable){
        check += is_symbol_in_table(global_tab, var1->u.ident);
        check += is_symbol_in_table(function_tab, var1->u.ident);
        switch(check){
            case 0:
                fprintf(stderr, "%s unregistered in globals or local variables\n", var1->u.ident);
                break;
            case 2:
                fprintf(stderr, "value in both local and global variable : %s\n", var1->u.ident);
                break;
            default:
                break;
        }

        return check == 1;
    }
    DEBUG("Error while assigning values\n");
    return 0;


}

/**
 * @brief parse the tree and check if there is a sem error or not
 * 
 * @param n 
 * @param table 
 * @return int 1 if the parse returned no error, 0 if there is at least 1 sem error
 */
int parse_sem_error(Node *n, List table, char *name_table){
    if (!n){
        return 1;
    }

    switch(n->label){
        case FunctionCall:

            return function_call_sem_parser(n, table, name_table) && parse_sem_error(n->nextSibling, table, name_table) && parse_sem_error(n->firstChild, table, name_table);
        case Variable:
            return variable_call_sem_parser(n, table, name_table) && parse_sem_error(n->nextSibling, table, name_table) && parse_sem_error(n->firstChild, table, name_table);
        case Assign:
            return assign_check(n, table, name_table) && parse_sem_error(n->nextSibling, table, name_table) && parse_sem_error(n->firstChild, table, name_table);
        case Eq:
            return equal_check(n, table, name_table) && parse_sem_error(n->nextSibling, table, name_table) && parse_sem_error(n->firstChild, table, name_table);
        default:
            return parse_sem_error(n->nextSibling, table, name_table) && parse_sem_error(n->firstChild, table, name_table);
    }
}

int parse_sem_function_error(Node *node, List table){
    if(!node){
        return 1;
    }


    if(node->label != DeclFoncts){
        return parse_sem_function_error(node->nextSibling, table) && parse_sem_function_error(node->firstChild, table);
    }
    else{
        //We suppose in declfoncts
        for(Node *n = node->firstChild; n; n = n->nextSibling){

            if(!parse_sem_error(n, table, getFuncNameFromDecl(n))){
                return 0;
            }

        }
    }
    return 1;
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


