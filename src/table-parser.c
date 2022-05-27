#include "table-parser.h"


List newSymbolTableList(Symbol_table *table){
    List ls_table;
    
    if(!(ls_table = (List)malloc(sizeof(Table)))){
        fprintf(stderr, "Error while allocating list of symbol table");
        return NULL; 
    }

    ls_table->table = table;
    ls_table->next = NULL;

    return ls_table;

}

int insertSymbolTableInList(List list, Symbol_table *table){

    List tmp;
    tmp = list;

    if(tmp->table == NULL){
        list->table = table;
        return 1;
    }
    for(tmp; tmp->next; tmp = tmp->next){}
    tmp->next = newSymbolTableList(table);
    if(!tmp->next){
        raiseError(-1, "Error while insert the table\n");
        return 0;
    }

    return 1;
}

void printSymbolTableList(List lst){

    List tmp = lst;
    if(!lst){
        return;
    }
    while(tmp != NULL){
        printSymbolTable(tmp->table);
        tmp = tmp->next;
    }
}

List buildSymbolTableListFromRoot(Node *root){

    List list;
    Node* functions_root;
    PrimType function_t;
    int i = 0, is_void, nb_args = 0, global_offset = 0;
    int totalFuncOffset = 0, totalLocalVariable = 0;
    Symbol_table *globals_table = buildGlobalVariableSymbolTable(root);
    list = newSymbolTableList(NULL);
    insertSymbolTableInList(list, globals_table);
    functions_root = root->firstChild->nextSibling; //On DeclFoncts
    //parse of the DeclFonct
    for(Node* function_root = functions_root->firstChild; function_root; function_root = function_root->nextSibling){
    
        nb_args = 0;
        totalLocalVariable = 0;
        // =============== Management of the functions's header ==================

        Node* header_function = function_root->firstChild;
        PrimType param_types[MAX_ARGUMENT_FUNC];
        Node *function_type = header_function->firstChild;

        is_void = header_function->firstChild->label == Void ? 1 : 0; 
        function_t = stringOfTpcType(function_type->u.ident);
        Symbol_table *table = newSymbolTable(function_type->firstChild->u.ident);
        Node *params = function_type->nextSibling;
        Symbol s;

        if(params->firstChild->label != Void){
            
            for (Node *paramType = params->firstChild; paramType; paramType = paramType->nextSibling){
                //parametres
                Kind k = PARAM;
                PrimType type = stringOfTpcType(paramType->u.ident);
                Node* id = paramType->firstChild;
                s = newSymbol(id->u.ident, k, type, 0, id->lineno);
                insertSymbol(s, table);
                table->nb_parameter += 1;

                //symbol de structure de fonction
                param_types[nb_args] = type;
                nb_args += 1;
            }
        }
        //=========================== Function's body ===========================

        Node* body = header_function->nextSibling;
        //Function's local variable :
        Node* local = body->firstChild;
        totalFuncOffset = -8;
        for(Node* localVarNode = local->firstChild; localVarNode; localVarNode = localVarNode->nextSibling){
            totalLocalVariable += 1;
            PrimType type = stringOfTpcType(localVarNode->u.ident); // type's variable
            Kind kind = VARIABLE;
            for(Node *id = localVarNode->firstChild; id; id = id->nextSibling){
                s = newSymbol(id->u.ident, kind, type, totalFuncOffset, id->lineno);
                insertSymbol(s, table);
                totalFuncOffset -= 8;
            }
        }
        table->total_size = totalLocalVariable;
        Symbol params_sym = newFunctionSymbol(function_type->firstChild->u.ident, function_t, param_types, nb_args, is_void, totalLocalVariable);
        insertSymbol(params_sym, table);
        insertSymbolTableInList(list, table);
    }
    return list;
}

/**
 * @brief To get the symbol table stored in the StList
 * @warning may return null if the table doesn't exist
 * @param name_table 
 * @param SymbolTableList 
 * @return Symbol_table* 
 */
Symbol_table *getTableInListByName(char *name_table, List SymbolTableList){
    
    List tmp = SymbolTableList;
    if(!SymbolTableList){
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
