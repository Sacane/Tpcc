#include "sem_parser.h"


/**
 * @brief To translate the operation 
 * 
 * @param opNode 
 * @return int 
 */
int computeOpNode(Node *opNode){
    switch(opNode->label){
        case Addsub:
            switch(opNode->u.byte){
                case '+':
                    return computeOpNode(FIRSTCHILD(opNode)) + computeOpNode(SECONDCHILD(opNode));
                case '-':
                    return computeOpNode(FIRSTCHILD(opNode)) + computeOpNode(SECONDCHILD(opNode));
                    break;
            }
        case divstar:
            switch(opNode->u.byte){
                case '*':
                    return computeOpNode(FIRSTCHILD(opNode)) * computeOpNode(SECONDCHILD(opNode));
                case '/':
                    return computeOpNode(FIRSTCHILD(opNode)) / computeOpNode(SECONDCHILD(opNode));
                case '%':
                    return computeOpNode(FIRSTCHILD(opNode)) % computeOpNode(SECONDCHILD(opNode));
            }
            break;
        case Int:
            return opNode->u.num;
        case Character:
            
            return opNode->u.byte;
    }
}

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
        //TODO case functionCall
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
    if(!ret_val) {
        check_sem_err = 1;
    }
    
    return ret_val == 1;
}

static int functionCallParamCheck(Symbol_table *fun_caller_table, Symbol_table *function_table, Symbol_table *global_var_table, Node *fc_root){

    int i;
    i = 0;
    Symbol params = getSymbolInTableByName(function_table, function_table->name_table);
    Symbol s;
    //D'abord on check si la fonction ne retourne pas void
    if(function_table->nb_parameter == 0 && fc_root->firstChild->label != Void){
        DEBUG("Label fc_root : %d\n", fc_root->firstChild->label);
        raiseError(fc_root->lineno, "Function '%s' does not expect parameters\n", params.symbol_name);
        check_sem_err = 1;
        return 0;
    } 
    if (function_table->nb_parameter == 0 && fc_root->firstChild->label == Void){
        return 1;
    }
    

    if(!fc_root->firstChild){
        raiseError(fc_root->lineno, "This function '%s' expect %d arguments but no argument were find\n", function_table->name_table, function_table->nb_parameter);
        check_sem_err = 1;
        return 0;
    }

    for(Node *n = fc_root->firstChild; n; n = n->nextSibling){
        if(i >= params.u.f_type.nb_args){
            raiseError(n->lineno, "Too much parameters while trying to call function '%s'\n", fc_root->u.ident);
            check_sem_err = 1;
            break;
        }
        if(isPrimLabelNode(n)){
            if (labelToPrim(n->label) != params.u.f_type.args_types[i]){
                raiseWarning(n->lineno, "When trying to call '%s' -> Expected type '%s' but the given type was '%s'\n", fc_root->u.ident, stringOfType(params.u.f_type.args_types[i]), stringOfType(labelToPrim(n->label)));
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
            check_sem_err = 1;
            return 0;
        }

        if(s.u.p_type != params.u.f_type.args_types[i]){
            if(params.u.f_type.args_types[i] == INT){
                if(s.u.p_type == CHAR){
                    raiseWarning(fc_root->lineno, "Variable '%s' is type char but function '%s' expected type int\n", s.symbol_name, params.symbol_name);
                }
            }
            raiseError(fc_root->lineno, "symbol name parameters n°%d : %s type : %s -> type expected : %s \n", i, s.symbol_name, stringOfType(s.u.p_type), stringOfType(params.u.f_type.args_types[i]));
            return 0;
        }
        i++;
    }
    if(i != params.u.f_type.nb_args){
        raiseError(fc_root->lineno, "Expected %d argument but %d were given\n", params.u.f_type.nb_args, i);
        check_sem_err = 1;
        return 0;
    }
    return 1;

}

static int isPrimaryFunctions(char *calledFunctionName){
    return strcmp(calledFunctionName, "getint") == 0 || strcmp(calledFunctionName, "getchar") == 0 ||
    strcmp(calledFunctionName, "putint") == 0 || strcmp(calledFunctionName, "putchar") == 0;
}

static int functionCallCheck(Node *fc_node, List table, char *callerFunctionName, char *calledFunctionName){

    Symbol_table *tableGlobal = getTableInListByName(GLOBAL, table);
    Symbol_table *tableCallerFunction = getTableInListByName(callerFunctionName, table);
    Symbol_table *tableCalledFunction = getTableInListByName(calledFunctionName, table);

    if(isPrimaryFunctions(calledFunctionName)){
        return 1;
    }

    if(!tableCalledFunction){
        raiseError(fc_node->lineno, "Trying to call a non-existing function : '%s'\n", calledFunctionName);
        check_sem_err = 1;
        return 0;
    }

    return functionCallParamCheck(tableCallerFunction, tableCalledFunction, tableGlobal, fc_node);
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
            check_sem_err = 1;
            return 0;
        }
    }

    if(var2->label == Variable){
        char* id2 = var2->u.ident;
        if(!(isSymbolInTable(global_tab,id2)) && !(isSymbolInTable(function_tab,id2))){
            raiseError(var2->lineno, "Variable '%s' undeclared neither as global or local\n", id2);
            check_sem_err = 1;
            return 0;
        }
        if(isSymbolInGlobalAndFunc(id2, function_tab, global_tab)){
            raiseError(var2->lineno, "symbol %s in both table function and global var\n", id2);
            check_sem_err = 1;
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
        //La variable est dans la table des globaux
        check += isSymbolInTable(global_tab, lValue->u.ident);
        if(isSymbolInTable(function_tab, lValue->u.ident)){
            Symbol s = getSymbolInTableByName(function_tab, lValue->u.ident);
            if(s.kind != FUNCTION){
                check += 1;
            } 
            if (rValue->label == FunctionCall){
                if(!functionCallCheck(rValue, tab, nameTable, rValue->u.ident)){
                    raiseError(rValue->lineno, "Function call failed : \n");
                    check_sem_err = 1;
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
            raiseError(lValue->lineno, "Trying to assign a non-existing value : '%s'.\n", lValue->u.ident);
            check_sem_err = 1;
            return 0;
        }
        return check == 1 || check == 2;
    }
    return 1;
}


void checkBinaryCaseAux(List listTable, char *tableName, Node *node, int *tab, int *index){
    int value;
    if(!node){
        return;
    }
    
    
    if(node->label == Case){
        Node *inCase = FIRSTCHILD(node);
        switch(inCase->label){
            case Int:
                tab[*index] = inCase->u.num;
                break;
            case Character:
                printf("%d\n", inCase->u.byte);
                tab[*index] = inCase->u.byte;
                break;
            case Addsub:
            case divstar:
                tab[*index] = computeOpNode(inCase);
                break;
            default:
                break;
        }
        (*index)++;
    }
    checkBinaryCaseAux(listTable, tableName, node->nextSibling, tab, index);
}


void checkBinaryCase(List listTable, char *tableName, Node *node){
    int tab[BUFSIZ];
    int index = 0;
    int checkCaseContent = 0;

    checkBinaryCaseAux(listTable, tableName, node->firstChild->nextSibling->nextSibling, tab, &index);
    for(int i = 0; i < index - 1; i++){
        for(int j = i+1; j < index; j++){

            if(tab[i] == tab[j]){
                
                checkCaseContent = 1;
            }
        }
    }
    if(checkCaseContent){
        raiseError(node->lineno, "There is doublons in cases content\n");
        check_sem_err = 1;
    }
}

void checkNumberDefault(int *cpt, Node *node){
    if(!node){
        return;
    }
    if(node->label == Default){
        (*cpt)++;
    }
    checkNumberDefault(cpt, node->firstChild);
    checkNumberDefault(cpt, node->nextSibling);
}

/**
 * @brief 
 * 
 * @param assign 
 * @param tab 
 * @return int 
 */
int switchCheck(List listTable, char * tableName, Node *switchNode){
    int cptDefault = 0;
    checkBinaryCase(listTable, tableName, switchNode);
    checkNumberDefault(&cptDefault, switchNode->firstChild);
    if(cptDefault > 1){
        raiseError(switchNode->lineno, "switch has more than 1 default\n");
        check_sem_err = 1;
    }
}

int hasIfElse(Node *ifNode){
    for(Node *child = ifNode->firstChild; child; child = child->nextSibling){
        if(child->label == Else){
            return 1;
        }
    }
    return 0;
}

void checkReturnsRec(Node *n, int *parseCpt){
    if(!n){
        return;
    }
    if(n->label == If || n->label == Else){
        
        (*parseCpt)++;
        for(Node *childN = n->firstChild; childN; childN = childN->nextSibling){
            if(childN->label == If){
                checkReturnsRec(childN, parseCpt);
            } 
            if(childN->label == Return){
                (*parseCpt)--;
            }
        }
        
    }

    checkReturnsRec(n->nextSibling, parseCpt);
    checkReturnsRec(n->firstChild, parseCpt);
}

void checkReturnsFunction(List list, Node *declFonctNode) {
    int parseCpt = 0;
    int firstLevelCheck;
    Node *bodyNode = SECONDCHILD(declFonctNode);
    //Vérifier si la fonction a une return value
    char *nameFun = getFuncNameFromDecl(declFonctNode);
    Symbol_table *table = getTableInListByName(nameFun, list);
    Symbol funS = getSymbolInTableByName(table, nameFun);


    for(Node *child = bodyNode->firstChild; child; child = child->nextSibling){
        
        if(child->label == Return){
            /*if(funS.u.f_type.is_void && child->firstChild){
                raiseError(funS.lineno, "Can't return value on a void-return function\n");
                check_sem_err = 1;
            }*/
            firstLevelCheck = 1;
            return;
        } else {
            firstLevelCheck = 0;
        }
    }

    //Check unreached of non-void function
    if(!(funS.u.f_type.is_void)){
        checkReturnsRec(SECONDCHILD(declFonctNode), &parseCpt);
    }
    
    DEBUG("ParseCpt : %d\n", parseCpt);
    if(parseCpt){
        raiseError(declFonctNode->lineno, "Reached end of control flow in non-void function\n");
        check_sem_err = 1;
    }
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
            functionCallCheck(n, table, name_table, n->u.ident);
            break;
        case Variable: 
            variableExistCheck(n, table, name_table);
            parseSemErrorAux(n->nextSibling, table, name_table);
            return 1;
        case Assign:
            assignCheck(n, table, name_table);
            break;
        case Eq:
            equalCompareCheck(n, table, name_table);
            break;
        case Switch:
            switchCheck(table, name_table, n);
            break;
        default:
            parseSemErrorAux(n->nextSibling, table, name_table);
            parseSemErrorAux(n->firstChild, table, name_table);
            return 1;
    }
    parseSemErrorAux(n->nextSibling, table, name_table);
    parseSemErrorAux(n->firstChild, table, name_table);
}

int parseSemError(Node *node, List table){
    if(!node){
        return 1;
    }
    if(node->label != DeclFoncts){
        parseSemError(node->nextSibling, table);
        parseSemError(node->firstChild, table);
    }
    else{
        for(Node *n = node->firstChild; n; n = n->nextSibling){
            char *nameFun = getFuncNameFromDecl(n);
            Symbol_table *sTable = getTableInListByName(nameFun, table);
            Symbol funS = getSymbolInTableByName(sTable, nameFun);
            if(funS.u.f_type.is_void){
                checkReturnsFunction(table, n);
            }
            parseSemErrorAux(n, table, getFuncNameFromDecl(n));
        }
    }
    return 1;
}










