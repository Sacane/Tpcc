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
        list->table = table;
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

List build_list_table(Node *root){

    List list;
    list = init_table_list(NULL);
    Symbol_table *globals_table = create_global_variable_table(root);
    insert_table(list, globals_table);
    int i = 0;
    int is_void, nb_args = 0;
    Node* functions_root;
    PrimType function_t;
    int global_offset = 0;
    functions_root = root->firstChild->nextSibling; //On DeclFoncts


    //parse of the DeclFonct
    for(Node* function_root = functions_root->firstChild; function_root; function_root = function_root->nextSibling){
        
        
        nb_args = 0;
        // =============== Management of the functions's header ==================

        Node* header_function = function_root->firstChild;
        PrimType param_types[MAX_ARGUMENT_FUNC];

        Node *function_type = header_function->firstChild;

        is_void = (SECONDCHILD(header_function))->firstChild->label == Void ? 1 : 0; 

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
                s = create_symbol(id->u.ident, k, type, 0, id->lineno);
                insert_symbol_in_table(s, table);
                table->nb_parameter += 1;

                //symbol de structure de fonction
                param_types[nb_args] = type;
                nb_args += 1;
            }
        }


        Symbol params_sym = create_func_sym(function_type->firstChild->u.ident, function_t, param_types, nb_args, is_void);
        insert_symbol_in_table(params_sym, table);

   
        // ========================================================================


        Node* body = header_function->nextSibling;
        
        
        //Function's local variable :
        Node* local = body->firstChild;
        for(Node* local_node = local->firstChild; local_node; local_node = local_node->nextSibling){
            PrimType type = str_to_tpcType(local_node->u.ident); // type's variable
            Kind kind = VARIABLE;
            for(Node *id = local_node->firstChild; id; id = id->nextSibling){
                s = create_symbol(id->u.ident, kind, type, 0, id->lineno);
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
        if(!tmp) return NULL;
    }
    
    return (tmp) ? tmp->table : NULL;
}

//Sem errors management






