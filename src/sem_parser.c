#include "sem_parser.h"

static PrimType getTypeOfNode(Node *node, Symbol_table *funTable, Symbol_table *globalTable){
    Symbol var;
    switch (node->label){
        case Int:  
            return INT;
        case Character:
            return CHAR;
        case Variable:
            if(isSymbolInTable(funTable, node->u.ident)){
                var = getSymbolInTableByName(funTable, node->u.ident);
            } else if (isSymbolInTable(globalTable, node->u.ident)){
                var = getSymbolInTableByName(globalTable, node->u.ident);
                
            } else {
                return NONE;
            }
            return var.u.p_type;
        default:
            return NONE;
    }
}


int isSymbolInGlobalAndFunc(char * symbol_name, Symbol_table *funTable, Symbol_table *globalTable)
{
    return isSymbolInTable(globalTable, symbol_name) && isSymbolInTable(funTable, symbol_name);
}

static int variableExistCheck(Node *varcall_node, List table, char *name){
    Symbol_table *globals = getTableInListByName(GLOBAL, table);
    Symbol_table *func = getTableInListByName(name, table);
    int ret_val = isSymbolInTable(globals, varcall_node->u.ident) || isSymbolInTable(func, varcall_node->u.ident);
    if(!ret_val) raiseError(varcall_node->lineno, "Variable '%s' undeclared as global or local\n", varcall_node->u.ident);
    
    return ret_val == 1;
}

static int functionCallParamCheck(Symbol_table *fun_caller_table, Symbol_table *function_table, Symbol_table *global_var_table, Node *fc_root){

    int i;
    i = 0;
    Symbol params = getSymbolInTableByName(function_table, function_table->name_table);
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
                raiseWarning(n->lineno, "When trying to call '%s' -> Expected type '%s' but the given type was '%s'\n", fc_root->u.ident, string_from_type(params.u.f_type.args_types[i]), string_from_type(labelToPrim(n->label)));
                i++;
                continue;
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
        if(isSymbolInTable(function_table, n->u.ident)){
            s = getSymbolInTableByName(function_table, n->u.ident);
        }  
        else if (isSymbolInTable(global_var_table, n->u.ident)){ //
            s = getSymbolInTableByName(global_var_table, n->u.ident);
        }  
        else if (isSymbolInTable(fun_caller_table, n->u.ident)){
            s = getSymbolInTableByName(fun_caller_table, n->u.ident);
        } 
        else {
            raiseError(fc_root->lineno, "Too many parameters given in function '%s'", fun_caller_table->name_table);
            return 0;
        }

        if(s.u.p_type != params.u.f_type.args_types[i]){
            if(params.u.f_type.args_types[i] == INT){
                if(s.u.p_type == CHAR){
                    raiseWarning(fc_root->lineno, "Variable '%s' is type char but function '%s' expected type int\n", s.symbol_name, params.symbol_name);
                }
            }
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

static int functionCallCheck(Node *fc_node, List table, char *callerFunctionName, char *calledFunctionName){

    Symbol_table *tableGlobal = getTableInListByName(GLOBAL, table);
    Symbol_table *tableCallerFunction = getTableInListByName(callerFunctionName, table);
    Symbol_table *tableCalledFunction = getTableInListByName(calledFunctionName, table);
    
    if(!tableCalledFunction){
        raiseError(fc_node->lineno, "Trying to call a non-existing function : '%s'\n", calledFunctionName);
        return 0;
    }
    if(!functionCallParamCheck(tableCallerFunction, tableCalledFunction, tableGlobal, fc_node)){
        return 0;
    }

    return 1;
}



static int equalCompareCheck(Node *eq, List tab, char *name_tab){

    Symbol_table* global_tab;
    Symbol_table* function_tab;
    Node *var1 = eq->firstChild;
    Node *var2 = eq->firstChild->nextSibling;


    global_tab = getTableInListByName(GLOBAL, tab);
    function_tab = getTableInListByName(name_tab, tab);

    if(var1->label == Variable){
        char id1[20];
        strcpy(id1, var1->u.ident);
        if(!(isSymbolInTable(global_tab,id1)) && !(isSymbolInTable(function_tab,id1))){
            raiseError(var1->lineno, "variable '%s' neither declared as local in function %s or as globals\n", id1, name_tab);
            return 0;
        }
    }

    if(var2->label == Variable){
        char* id2 = var2->u.ident;
        if(!(isSymbolInTable(global_tab,id2)) && !(isSymbolInTable(function_tab,id2))){
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


int assignCheck(Node *assign, List tab, char *nameTable){
    
    Symbol_table *global_tab;
    Symbol_table *function_tab;
    Symbol_table *calledTable;
    int check = 0;
    global_tab = getTableInListByName(GLOBAL, tab);
    function_tab = getTableInListByName(nameTable, tab);

    Node *lValue = FIRSTCHILD(assign);
    Node *rValue = SECONDCHILD(assign);

    PrimType lType = getTypeOfNode(lValue, function_tab, global_tab);
    PrimType rType = getTypeOfNode(rValue, function_tab, global_tab);

    if(lType == CHAR && rType == INT) {
        raiseWarning(lValue->lineno, "assigning char variable '%s' to integer %d\n", lValue->u.ident, rValue->u.num);
        check_warn = 1;
    }

    if(lValue->label == Variable){
        
        check += isSymbolInTable(global_tab, lValue->u.ident);
        if(isSymbolInTable(function_tab, lValue->u.ident)){
            Symbol s = getSymbolInTableByName(function_tab, lValue->u.ident);
            if(s.kind != FUNCTION){
                check += 1;
            } 
            if (rValue->label == FunctionCall){
                if(!functionCallCheck(rValue, tab, nameTable, rValue->u.ident)){
                    raiseError(rValue->lineno, "Function call failed : \n");
                    return 0;
                }
                calledTable = getTableInListByName(rValue->u.ident, tab);

                Symbol fun = getSymbolInTableByName(calledTable, rValue->u.ident);

                if(lType != fun.u.f_type.return_type){
                    raiseWarning(rValue->lineno, "Return type of function '%s' doesn't match with the assigned variable\n", rValue->u.ident);
                }
            }
        }
        if(!check){
            raiseError(lValue->lineno, "Left value : '%s' is neither a global or local variable\n", lValue->u.ident);
        }
        return check == 1 || check == 2;
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
static int parseSemErrorAux(Node *n, List table, char *name_table){
    if (!n){
        return 1;
    }
    if(n->label == EnTeteFonct) return parseSemErrorAux(n->nextSibling, table, name_table);
    if(n->label == DeclFonct) return parseSemErrorAux(n->firstChild->nextSibling, table, name_table);
    switch(n->label){
        case FunctionCall:
            return functionCallCheck(n, table, name_table, n->u.ident) && parseSemErrorAux(n->nextSibling, table, name_table) && parseSemErrorAux(n->firstChild, table, name_table);
        case Variable: 
            return variableExistCheck(n, table, name_table) && parseSemErrorAux(n->nextSibling, table, name_table);
        case Assign:
            return assignCheck(n, table, name_table) && parseSemErrorAux(n->nextSibling, table, name_table) && parseSemErrorAux(n->firstChild, table, name_table);
        case Eq:
            return equalCompareCheck(n, table, name_table) && parseSemErrorAux(n->nextSibling, table, name_table) && parseSemErrorAux(n->firstChild, table, name_table);
        default:
            return parseSemErrorAux(n->nextSibling, table, name_table) && parseSemErrorAux(n->firstChild, table, name_table);
    }
}

int parseSemError(Node *node, List table){
    if(!node){
        return 1;
    }
    if(!getTableInListByName("main", table)){
        raiseError(-1, "No main function in this program\n");
        check_sem_err = 1;
        return 0;
    }
    if(node->label != DeclFoncts){
        return parseSemError(node->nextSibling, table) && parseSemError(node->firstChild, table);
    }
    else{
        for(Node *n = node->firstChild; n; n = n->nextSibling){
            if(!parseSemErrorAux(n, table, getFuncNameFromDecl(n))){
                
                return 0;
            }
        }
    }
    return 1;
}










