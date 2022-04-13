#include "sem_parser.h"

int isSymbolInGlobalAndFunc(char * symbol_name, Symbol_table *funTable, Symbol_table *globalTable)
{
    return is_symbol_in_table(globalTable, symbol_name) && is_symbol_in_table(funTable, symbol_name);
}

static int variable_call_sem_parser(Node *varcall_node, List table, char *name){
    Symbol_table *globals = get_table_by_name("global_vars", table);
    Symbol_table *func = get_table_by_name(name, table);
    int ret_val = is_symbol_in_table(globals, varcall_node->u.ident) || is_symbol_in_table(func, varcall_node->u.ident);
    if(!ret_val) raiseError(varcall_node->lineno, "Variable '%s' undeclared as global or local\n", varcall_node->u.ident);
    
    return ret_val == 1;
}

static int check_param_function_call(Symbol_table *fun_caller_table, Symbol_table *function_table, Symbol_table *global_var_table, Node *fc_root){

    int i;
    i = 0;
    Symbol params = get_symbol_by_name(function_table, function_table->name_table);
    Symbol s;
    //D'abord on check si la fonction ne retourne pas void
    if(params.u.f_type.is_void && FIRSTCHILD(fc_root)->label != Void){
        raiseError(fc_root->lineno, "Function '%s' does not expect parameters\n", params.symbol_name);
        return 0;
    } 
    if (params.u.f_type.is_void && FIRSTCHILD(fc_root)->label == Void){
        return 1;
    }
    

    if(!fc_root->firstChild){
        raiseError(fc_root->lineno, "This function '%s' expect %d arguments but no argument were find\n", function_table->name_table, function_table->nb_parameter);
        return 0;
    }

    for(Node *n = fc_root->firstChild; n; n = n->nextSibling){
        if(isPrimLabelNode(n)){
            if (labelToPrim(n->label) != params.u.f_type.args_types[i]){
                raiseError(n->lineno, "When trying to call '%s' -> Expected type '%s' but the given type was '%s'\n", fc_root->u.ident, string_from_type(params.u.f_type.args_types[i]), string_from_type(labelToPrim(n->label)));
                return 0;
            }
            else {
                i++;
                continue;
            }
        }   
        if(n->label == Addsub){
            
            i++;
            continue;
        }
        //On récupère le symbol de la fonction pour vérifier ses paramètres
        if(is_symbol_in_table(function_table, n->u.ident)){
            s = get_symbol_by_name(function_table, n->u.ident);
        }  
        else if (is_symbol_in_table(global_var_table, n->u.ident)){ //
            s = get_symbol_by_name(global_var_table, n->u.ident);
        }  
        else if (is_symbol_in_table(fun_caller_table, n->u.ident)){
            s = get_symbol_by_name(fun_caller_table, n->u.ident);
        } 
        else {
            raiseError(fc_root->lineno, "Too many parameters given in function '%s'", fun_caller_table->name_table);
            return 0;
        }
        
        if(s.u.p_type != params.u.f_type.args_types[i]){
            raiseError(fc_root->lineno, "symbol name parameters n°%d : %s type : %s -> type expected : %s \n", i, s.symbol_name, string_from_type(s.u.p_type), string_from_type(params.u.f_type.args_types[i]));
            return 0;
        }
        i++;
    }
    if(i != params.u.f_type.nb_args){
        raiseError(fc_root->lineno, "Expected %d argument but %d were given\n", params.u.f_type.nb_args, i);
        return 0;
    }
    return 1;

}

static int function_call_sem_parser(Node *fc_node, List table, char *name_fun_caller, char *name_fun_called){

    Symbol_table *global_table = get_table_by_name("global_vars", table);
    Symbol_table *fun_caller_table = get_table_by_name(name_fun_caller, table);
    Symbol_table *fun_called_table = get_table_by_name(name_fun_called, table);
    
    if(!fun_called_table){
        raiseError(fc_node->lineno, "Trying to call a non-existing function : '%s'\n", name_fun_called);
        return 0;
    }
    if(!check_param_function_call(fun_caller_table, fun_called_table, global_table, fc_node)){
        return 0;
    }

    return 1;
}



static int equal_check(Node *eq, List tab, char *name_tab){

    Symbol_table* global_tab;
    Symbol_table* function_tab;
    Node *var1 = eq->firstChild;
    Node *var2 = eq->firstChild->nextSibling;


    global_tab = get_table_by_name("global_vars", tab);
    function_tab = get_table_by_name(name_tab, tab);

    if(var1->label == Variable){
        char id1[20];
        strcpy(id1, var1->u.ident);
        if(!(is_symbol_in_table(global_tab,id1)) && !(is_symbol_in_table(function_tab,id1))){
            raiseError(var1->lineno, "variable '%s' neither declared as local in function %s or as globals\n", id1, name_tab);
            return 0;
        }
        if(isSymbolInGlobalAndFunc(id1, function_tab, global_tab)){
            raiseError(var1->lineno, "symbol '%s' declared as function and global variable\n", var1->u.ident);
            return 0;
        }
    }

    if(var2->label == Variable){
        char* id2 = var2->u.ident;
        if(!(is_symbol_in_table(global_tab,id2)) && !(is_symbol_in_table(function_tab,id2))){

            raiseError(var2->lineno, "Variable '%s' undeclared neither as global or local\n", id2);
            return 0;
        }
        if(isSymbolInGlobalAndFunc(id2, function_tab, global_tab)){
            raiseError(var2->lineno, "symbol %s in both table function and global var\n", id2);
            return 0;
        }
    }

    return 1;
}

static PrimType getTypeOfNode(Node *node, Symbol_table *funTable, Symbol_table *globalTable){
    Symbol var;
    switch (node->label){
        case Int:  
            return INT;
        case Character:
            return CHAR;
        case Variable:
            if(is_symbol_in_table(funTable, node->u.ident)){
                var = get_symbol_by_name(funTable, node->u.ident);
            } else if (is_symbol_in_table(globalTable, node->u.ident)){
                var = get_symbol_by_name(globalTable, node->u.ident);
                
            } else {
                return NONE;
            }
            return var.u.p_type;
        default:
            return NONE;
    }
}

int assign_check(Node *assign, List tab, char *name_tab){
    
    Symbol_table *global_tab;
    Symbol_table *function_tab;
    Symbol_table *fun_called;
    int check = 0;
    global_tab = get_table_by_name("global_vars", tab);
    function_tab = get_table_by_name(name_tab, tab);

    Node *lValue = FIRSTCHILD(assign);
    Node *rValue = SECONDCHILD(assign);

    PrimType lType = getTypeOfNode(lValue, function_tab, global_tab);
    PrimType rType = getTypeOfNode(rValue, function_tab, global_tab);

    if(lType == CHAR && rType == INT) {
        raiseWarning(lValue->lineno, "assigning char variable '%s' to integer %d\n", lValue->u.ident, rValue->u.num);
        check_warn = 1;
    }

    if(lValue->label == Variable){
        
        check += is_symbol_in_table(global_tab, lValue->u.ident);
        if(is_symbol_in_table(function_tab, lValue->u.ident)){
            Symbol s = get_symbol_by_name(function_tab, lValue->u.ident);
            if(s.kind != FUNCTION){
                check += 1;
            } 
            if (rValue->label == FunctionCall){
                if(!function_call_sem_parser(rValue, tab, name_tab, rValue->u.ident)){
                    return 0;
                }
                fun_called = get_table_by_name(rValue->u.ident, tab);

                Symbol fun = get_symbol_by_name(fun_called, rValue->u.ident);

                if(lType != fun.u.f_type.return_type){
                    raiseWarning(rValue->lineno, "Return type of function '%s' doesn't match with the assigned variable\n", rValue->u.ident);
                }
            }
        }
        switch(check){
            case 0:
                raiseError(lValue->lineno, "Left value : '%s' is neither a global or local variable\n", lValue->u.ident);
                break;
            case 2:
                raiseError(lValue->lineno, "variable %s already been defined as global\n", lValue->u.ident);
                break;
            default:
                break;
        }

        return check == 1;
    }
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
    if(n->label == EnTeteFonct) return parse_sem_error(n->nextSibling, table, name_table);
    if(n->label == DeclFonct) return parse_sem_error(n->firstChild->nextSibling, table, name_table);
    switch(n->label){
        case FunctionCall:
            return function_call_sem_parser(n, table, name_table, n->u.ident) && parse_sem_error(n->nextSibling, table, name_table) && parse_sem_error(n->firstChild, table, name_table);
        case Variable: 
            return variable_call_sem_parser(n, table, name_table) && parse_sem_error(n->nextSibling, table, name_table);
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
    if(!get_table_by_name("main", table)){
        raiseError(-1, "No main function in this program\n");
        check_sem_err = 1;
        return 0;
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










