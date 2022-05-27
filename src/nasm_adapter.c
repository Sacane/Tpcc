#include "nasm_adapter.h"


#define LABEL_IF "labelId_"
#define LABEL_ELSE "labelElse_"
#define LABEL_CODE "labelCode_" 
#define LABEL_EXPR "labelExpr_"
#define LABEL_TRUE "TRUE"
#define LABEL_FALSE "FALSE"
#define OPERATOR 0
#define VAR 1
#define CONST 2

#define IN_GLOBAL 0
#define IN_FUNCTION 1

FILE *f;
static int firstCall = 1;


static int labelId;



void nasmTranslateParsing(Node *root, Symbol_table *global_var_table, List list, char *currentFunctionName);



static char* stringOfNasmFun(NasmFunCall nasmFunction){
    switch (nasmFunction){
        case PUSH: return "push";
        case POP:  return "pop";
        case MOV:  return "mov";
        case ADD:  return "add";
        case SUB:  return "sub";
        case MUL:  return "imul";
        case DIV:  return "idiv";
        case NOT:  return "neg";
        case SYSCALL: return "syscall";
        case JMP:   return "jmp";
        case JG:    return "jg";
        case JE:    return "je";
        case RET:   return "ret";
        case N_AND: return "add";
        case N_OR:  return "or";
        case CALL:  return "call";
        case CMP :  return "cmp";
        case JLE : return "jle";
        case JGE : return "jge";
        case JL: return "jl";
        case JNE: return "jne";

    }
}

static void initBss(int globalBssSize, List listTable){
    

    fprintf(f, "section .bss\n");
    fprintf(f, "%s: resq %d\n", GLOBAL, globalBssSize);

}

void writeNasmHeader(int globalBssSize, List listTable){
    

    fprintf(f, "section .data\n");
    fprintf(f, "\t formatInt: db \"%s\", 10, 0\n", "%d");

    initBss(globalBssSize, listTable);
    


    fprintf(f, "section  .text\n \
    global  _start\n \
    extern my_putchar\n \
    extern show_registers\n \
    extern my_getint\n \
    extern printf\n");   
    
}



static int nasmArityOf(NasmFunCall fc){
    switch (fc){
        case PUSH: 
        case POP:    
        case NOT:  
        case JMP:
        case JG:
        case JE:
        case DIV:
        case CALL:
        case JLE:
        case JGE:
        case JL:
        case JNE:
            return 1;
        case MOV:
        case ADD: 
        case SUB: 
        case MUL: 
        case N_AND: 
        case N_OR:  
        case CMP:
            return 2;
        case SYSCALL:
        case RET: 
            return 0;
    }
}



static char *varStaticAdress(){
    char buf[BUFSIZ];
}

static int nasmFunArityCheck(NasmFunCall nasmFunCall, char *var1, char *var2){
    switch (nasmArityOf(nasmFunCall)){
        case 0:
            return (!var1) && (!var2);
        case 1:
            return var1 && (!var2);
        case 2:
            return var1 && var2;
        default:
            return -1;
    } 
}


int nasmCall(NasmFunCall nasmFunCall, char *var1, char *var2)
{  
    if(!nasmFunArityCheck(nasmFunCall, var1, var2)){
        DEBUG("Fail to write into the nasm generator file : Incorrect usage of writer fun\n");
        return 0;
    }

    switch(nasmArityOf(nasmFunCall)){
        case 0:
            fprintf(f, "\t%s\n", stringOfNasmFun(nasmFunCall));
            break;
        case 1:
            fprintf(f, "\t%s %s\n", stringOfNasmFun(nasmFunCall), var1);
            break;
        case 2:
            fprintf(f, "\t%s %s, %s\n", stringOfNasmFun(nasmFunCall), var1, var2);
            break;
        default:
            break;
    }
    return 1;

}

void end_asm(){
    fprintf(f, "_start:\n");

    
    fprintf(f, "\tcall main\n");

    fprintf(f, "\tmov rax, 60\n\tmov rdi, 0\n\tsyscall\n");
    fclose(f);
}


void writeTestRegisters(){
    nasmCall(CALL, "show_registers", NULL);
}

void write_global_eval(Symbol_table *global_table, Node *assign_node){

    
    Node *l_value = assign_node->firstChild;
    Node *r_value = l_value->nextSibling;

    if(!isSymbolInTable(global_table, l_value->u.ident) || isSymbolInTable(global_table, r_value->u.ident)){
        return;
    }

}   

int symbolPriority(List list, Symbol_table *functionTable, char *nameSymbol){
    Symbol_table *global;
    int isGlobalLayer = 0;
    global = getTableInListByName(GLOBAL, list);
    
    if(isSymbolInTable(global, nameSymbol)){
        isGlobalLayer = 1;
    }
    if (isSymbolInTable(functionTable, nameSymbol)){
        isGlobalLayer = 0;
    }
    return (isGlobalLayer) ? IN_GLOBAL : IN_FUNCTION;

}

/* We suppose node parameter in the body of the main */
/**
 * @brief 
 * 
 * @param addSubNode 
 * @param symbolTable 
 */
void opTranslate(Node* addSubNode, Symbol_table *symbolTable, List list, int stage){
    assert(addSubNode);
    char buf[BUFSIZ];
    char buf2[BUFSIZ];
    int denominator;
    Symbol s;
    int a, b;
    int offset;
    int priority;
    Symbol_table *globalTable = getTableInListByName(GLOBAL, list);
    int isLeftInGlobal = 0, isRightInGlobal = 0;
    if(FIRSTCHILD(addSubNode)->label == Addsub || FIRSTCHILD(addSubNode)->label == divstar){
        opTranslate(FIRSTCHILD(addSubNode), symbolTable, list, stage+1);
    } else {
        switch (FIRSTCHILD(addSubNode)->label){
            case Int:
            case Character: 
                a = addSubNode->firstChild->u.num;
                sprintf(buf, "%d", a);
                nasmCall(PUSH, buf, NULL);
                break;
            case Variable:
                if(((priority = symbolPriority(list, symbolTable, FIRSTCHILD(addSubNode)->u.ident)) == IN_FUNCTION)){
                    s = getSymbolInTableByName(symbolTable, FIRSTCHILD(addSubNode)->u.ident);
                } else {
                    s = getSymbolInTableByName(globalTable, FIRSTCHILD(addSubNode)->u.ident);
                }

                sprintf(buf, "qword [%s + %d]",(priority == IN_GLOBAL) ? GLOBAL : symbolTable->name_table, s.offset);

                
                nasmCall(PUSH, buf, NULL);
                break;
            default:
                break;
        }
        
    }
    if(SECONDCHILD(addSubNode)->label == Addsub || SECONDCHILD(addSubNode)->label == divstar){
        opTranslate(SECONDCHILD(addSubNode), symbolTable, list, stage+1);
    }
    if(addSubNode && (addSubNode->label == Addsub || addSubNode->label == divstar)){

        switch (SECONDCHILD(addSubNode)->label){
            case Int:
            case Character:
                a = SECONDCHILD(addSubNode)->u.num;
                if((addSubNode->u.byte == '/' || addSubNode->u.byte == '%')){
                    if(a == 0){
                        raiseError(addSubNode->lineno, "Trying to divide by 0\n");
                        exit(EXIT_FAILURE);
                    } else {
                        nasmCall(PUSH, "0", NULL);
                    }
                }
                sprintf(buf2, "%d", a);
                nasmCall(PUSH, buf2, NULL);
                
                break;
            case Variable:
                if(((priority = symbolPriority(list, symbolTable, SECONDCHILD(addSubNode)->u.ident)) == IN_FUNCTION)){
                    s = getSymbolInTableByName(symbolTable, SECONDCHILD(addSubNode)->u.ident);
                } else {
                    s = getSymbolInTableByName(globalTable, SECONDCHILD(addSubNode)->u.ident);

                }

                sprintf(buf2, "qword [%s + %d]",(priority == IN_GLOBAL) ? GLOBAL : symbolTable->name_table, s.offset);

                nasmCall(PUSH, buf2, NULL);
                break;
            default:
                break;
        }
        
    }
    if(addSubNode->label == Addsub || addSubNode->u.byte == '*'){
        nasmCall(POP, "rcx", NULL);
        nasmCall(POP, "rax", NULL);
    } else {
        nasmCall(POP, "rbx", NULL);
        nasmCall(PUSH, "0", NULL);
        nasmCall(POP, "rdx", NULL);
        nasmCall(POP, "rax", NULL);
    }

    switch(addSubNode->u.byte){
        case '-':
            nasmCall(SUB, "rax", "rcx");
            break;
        case '+':
            nasmCall(ADD, "rax", "rcx");
            break;
        case '*':
            nasmCall(MUL, "rax", "rcx");
            break;
        case '/':
        case '%':
            nasmCall(DIV, "rbx", NULL);
            break;
    }
    if(addSubNode->u.byte == '%'){
        nasmCall(PUSH, "rdx", NULL); 
        nasmCall(MOV, "rax", "rdx");
        nasmCall(MOV, "r12", "rdx");
    } else {
        nasmCall(PUSH, "rax", NULL);
    }
    nasmCall(MOV, "r12", "rax"); //Pour vérifier l'affichage avec show registers
    if(stage == 0){
        nasmCall(POP, "rax", NULL);
    }
    
}

/*=================== Function setup ============ */

void setFunctionHeader(Node *functionRoot, List lst, Symbol_table* funTable){

    fprintf(f, "%s:", funTable->name_table);
    

}

//We suppose functionRoot in declFonct
void allocateLocalVar(Node *functionRoot, List lst, Symbol_table *funTable){
    int totalOffset;    
    char buf[BUFSIZ];
    totalOffset = (-1) * funTable->total_size;
    Node *varsNode = FIRSTCHILD(SECONDCHILD(functionRoot));
    sprintf(buf, "%d", totalOffset);
    nasmCall(ADD, "rsp", buf);

}



/*=====================================================*/
void assign_global_var(Symbol_table *symbolTable, FILE* in, Node *assign_node, List list){
    int i;
    char c;
    int pos;
    char buf[BUFSIZ];
    char buf2[BUFSIZ];
    int isGlobalLayer = 0;
    Symbol lVar;
    Node *lValue = FIRSTCHILD(assign_node);
    Node *rValue = SECONDCHILD(assign_node);
    Symbol_table *globalTable = getTableInListByName(GLOBAL, list);
    if(lValue->label == Int || lValue->label == Character){
        raiseError(assign_node->lineno, "trying to assign numeric or character\n");
        return;
    }
    if(isSymbolInTable(globalTable, lValue->u.ident)){
        lVar = getSymbolInTableByName(globalTable, lValue->u.ident);
    }
    //Local var
    if(isSymbolInTable(symbolTable, lValue->u.ident)){
        lVar = getSymbolInTableByName(symbolTable, lValue->u.ident);
        isGlobalLayer = 0;

    } else {
        isGlobalLayer = 1;
    }
    switch(rValue->label){
        case Character:
            c = rValue->u.byte;
            sprintf(buf, "'%c'", c);
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (isGlobalLayer) ? "+" : "", lVar.offset);
            nasmCall(MOV, buf2, buf);
            break;
        case Int:
            i = rValue->u.num;
            sprintf(buf, "%d", i);
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (isGlobalLayer) ? "+" : "", lVar.offset);
            nasmCall(MOV, buf2, buf);
            break;
        case Addsub:
        case divstar:
            opTranslate(rValue, symbolTable, list, 0); // Put into r12 value of addition
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (isGlobalLayer) ? "+" : "", lVar.offset);
            nasmCall(MOV, buf2, "rax");
            nasmCall(MOV, "rax", "0");
            nasmCall(MOV, "rbx", "0");
            break;
        default:
            raiseWarning(rValue->lineno, "Assign from variable are Not available in this version of compilation\n");
            return;

    }
}

static void orderTranslate(Node *orderNode){
    Node *leftExpr = FIRSTCHILD(orderNode);
    Node *rightExpr = SECONDCHILD(orderNode);
    
}

int checkNodeContent(Node *n){
    switch(n->label){
        case Addsub:
        case divstar:
            return OPERATOR;
        case Variable:
            return VAR;
        case Int:
        case Character:
            return CONST;
        default:
            return -1;
    }
}

/**
 * @brief 
 * 
 * @param content 
 */
static void callPrintf(char *content){
    
    nasmCall(MOV, "rsi", content);
    
    nasmCall(MOV, "rax", "0");
    nasmCall(MOV, "rdi", "formatInt");
    nasmCall(CALL, "printf", NULL);
}

static void writePutint(Node *putIntNode, List list, Symbol_table *funTable){
    char buf[BUFSIZ];
    int n;
    Symbol s;
    Symbol_table *globalTable = getTableInListByName(GLOBAL, list);

    switch(checkNodeContent(FIRSTCHILD(putIntNode))){
        case CONST:
            n = FIRSTCHILD(putIntNode)->u.num;
            break;
        case VAR:
            if(isSymbolInTable(globalTable, FIRSTCHILD(putIntNode)->u.ident)){
                s = getSymbolInTableByName(globalTable, FIRSTCHILD(putIntNode)->u.ident);
                
                sprintf(buf, "qword [%s + %d]", GLOBAL, s.offset);
                callPrintf(buf);
                return;
            } else {
                s = getSymbolInTableByName(funTable, FIRSTCHILD(putIntNode)->u.ident);
                sprintf(buf, "qword [rbp %d]", s.offset);
                callPrintf(buf);
                return;
            }
            break;
        default:
            opTranslate(FIRSTCHILD(putIntNode), globalTable, list, 0);
            callPrintf("rax");
            return;
    }
    sprintf(buf, "%d", n);
    callPrintf(buf);
}

static void writePutchar(Node *putcharNode, Symbol_table *funTable, List list){
    char buf[BUFSIZ];
    char c = FIRSTCHILD(putcharNode)->u.byte;
    Symbol variable;
    int contentLayer;
    Symbol_table *global = getTableInListByName(GLOBAL, list);
    switch (checkNodeContent(FIRSTCHILD(putcharNode))){
        case CONST:
            c = FIRSTCHILD(putcharNode)->u.byte;
            switch(c){
                case '\n':
                    sprintf(buf, "`\\n`");
                    nasmCall(MOV, "rdi", buf);
                    nasmCall(CALL, "my_putchar", NULL);
                    return;
                default:
                    sprintf(buf, "'%c'", c);
                    break;
            }
            break;
        case VAR:
            if(isSymbolInTable(global, FIRSTCHILD(putcharNode)->u.ident)){
                variable = getSymbolInTableByName(global, FIRSTCHILD(putcharNode)->u.ident);
                contentLayer = IN_GLOBAL;
            }
            if (isSymbolInTable(funTable, FIRSTCHILD(putcharNode)->u.ident)){
                variable = getSymbolInTableByName(funTable, FIRSTCHILD(putcharNode)->u.ident);
                contentLayer = IN_FUNCTION;
            } else {
                contentLayer = IN_GLOBAL;
            }
            
            sprintf(buf, "qword [%s %s %d]", (contentLayer == IN_GLOBAL) ? GLOBAL : "rbp", (contentLayer == IN_GLOBAL) ? "+" : "", variable.offset);
            break;
    }

    nasmCall(MOV, "rdi", buf);
    nasmCall(CALL, "my_putchar", NULL);
}



int compareInstrAux(Node *condNode, List list, Symbol_table *funTable){
    Node *opLeft, *opRight;
    int lValue, rValue;
    Symbol_table *global;
    Symbol s;
    
    opLeft = FIRSTCHILD(condNode);
    opRight = SECONDCHILD(condNode);
    char buf[BUFSIZ];
    int isGlobalLayer;

    global = getTableInListByName(GLOBAL, list);


    switch(checkNodeContent(opLeft)){
        case OPERATOR: {
            opTranslate(condNode, funTable, list, 0);  //rax
            nasmCall(MOV, "r14", "rax");
        }
        break;
        case VAR: {
            if(isSymbolInTable(global, opLeft->u.ident)){
                s = getSymbolInTableByName(global, opLeft->u.ident);
            } //TODO : remove to refract local variables
            if(isSymbolInTable(funTable, opLeft->u.ident)){
                s = getSymbolInTableByName(funTable, opLeft->u.ident);
                isGlobalLayer = 0;
            } 
            else {
                isGlobalLayer = 1;
            }

            sprintf(buf, "qword [%s + %d]", (isGlobalLayer) ? GLOBAL : funTable->name_table, s.offset);
            nasmCall(MOV, "r14", buf);
        }
        break;
        case CONST: {
            if(opLeft->label == Character){
                lValue = opLeft->u.byte;
            }
            else {
                lValue = opLeft->u.num;
            }
            sprintf(buf, "%d", lValue);
            nasmCall(MOV, "r14", buf);
        }
        break;
    }

    switch(checkNodeContent(opRight)){
        case OPERATOR: {
            opTranslate(condNode, funTable, list, 0);  //rax
            nasmCall(MOV, "r15", "rax");

        }
        break;
        case VAR: {
            if(isSymbolInTable(global, opRight->u.ident)){
                s = getSymbolInTableByName(global, opRight->u.ident);
            } 
            if(isSymbolInTable(funTable, opRight->u.ident)){
                s = getSymbolInTableByName(funTable, opRight->u.ident);
                isGlobalLayer = 0;
            } 
            else {
                isGlobalLayer = 1;
            }

            sprintf(buf, "qword [%s + %d]", (isGlobalLayer) ? GLOBAL : funTable->name_table, s.offset);
            nasmCall(MOV, "r15", buf);
        }
        break;
        case CONST: {
            if(opRight->label == Character){
                rValue = opRight->u.byte;
            }
            else {
                rValue = opRight->u.num;
            }
            sprintf(buf, "%d", rValue);
            nasmCall(MOV, "r15", buf);
        }
    }
}

void treatExpr(Node *conditionNode, List list, Symbol_table *funTable, char *labelIf, char *labelElse, char *labelCode, int hasElse){

    Symbol s;
    NasmFunCall compFun;
    Symbol_table* globalTable;
    char buf[BUFSIZ];
    switch(conditionNode->label){
        case Variable:
            globalTable = getTableInListByName(GLOBAL, list);
            //Global
            if(isSymbolInTable(globalTable, conditionNode->u.ident)){
                s = getSymbolInTableByName(globalTable, conditionNode->u.ident);
                sprintf(buf, "qword [%s + %d]", GLOBAL, s.offset);
            }
            //Local
            if(isSymbolInTable(funTable, conditionNode->u.ident)){
                s = getSymbolInTableByName(funTable, conditionNode->u.ident);
                sprintf(buf, "qword [%s + %d]", funTable->name_table, s.offset);
            }
            nasmCall(MOV, "rbx", "0");
            nasmCall(ADD, "rbx", buf);
            nasmCall(JG, labelIf, NULL);
            break;
        case Int:
            sprintf(buf, "%d", conditionNode->u.num);
            nasmCall(MOV, "rbx", "0");
            nasmCall(ADD, "rbx", buf);
            nasmCall(JG, labelIf, NULL);
            break;
        case Eq:
        case Order:

            compareInstrAux(conditionNode, list, funTable);
            nasmCall(CMP, "r14", "r15");
            
            if(strcmp(conditionNode->u.ident, "<=") == 0){
                compFun = JLE;
            }
            if (strcmp(conditionNode->u.ident, ">=") == 0){
                compFun = JGE;
            }
            if(strcmp(conditionNode->u.ident, "<") == 0){
                compFun = JL;
            }
            if(strcmp(conditionNode->u.ident, ">") == 0){
                compFun = JG;
            }
            if(strcmp(conditionNode->u.ident, "==") == 0){
                compFun = JE;
            }
            if(strcmp(conditionNode->u.ident, "!=") == 0){
                compFun = JNE;
            }

            nasmCall(compFun, labelIf, NULL);
            nasmCall(MOV, "r14", "0");
            nasmCall(MOV, "r15", "0");
            break;
        case Addsub:
        case divstar:
            opTranslate(conditionNode, funTable, list, 0); //Resultat de l'opération dans rax
            nasmCall(ADD, "rbx", "rax");
            nasmCall(JG, labelIf, NULL);

            break;
        case Or:

            sprintf(buf, "%s%d:", LABEL_EXPR, labelId);

            if((conditionNode->firstChild->label != Or && conditionNode->firstChild->label != And) &&
            (conditionNode->firstChild->nextSibling->label != Or && conditionNode->firstChild->nextSibling->label != And)){
                fprintf(f, "%s\n", buf);
            }
            labelId += 1;
            sprintf(buf, "%s%d:", LABEL_EXPR, labelId);
            treatExpr(conditionNode->firstChild, list, funTable, labelIf, buf, labelCode, hasElse);
            sprintf(buf, "%s%d:\n", LABEL_EXPR, labelId);
            labelId += 1;
            fprintf(f, "%s", buf);
            treatExpr(conditionNode->firstChild->nextSibling, list, funTable, labelIf, labelElse, labelCode, hasElse);
            break;
        case And:
            sprintf(buf, "%s%d", LABEL_EXPR, labelId);
            labelId += 1;
            fprintf(f, "%s:\n", buf);
            sprintf(buf, "%s%d", LABEL_EXPR, labelId);
            treatExpr(conditionNode->firstChild->nextSibling, list, funTable, buf, labelElse, labelCode, hasElse);
            nasmCall(JMP, labelCode, NULL);
            fprintf(f, "%s:\n", buf);
            labelId += 1;
            treatExpr(conditionNode->firstChild, list, funTable, labelIf, labelElse, labelCode, hasElse);
            
            
            break;
        default:
            break;
    }
}


void ifInstr(Node *ifInstr, List list, Symbol_table *funTable){
    char buf[BUFSIZ];
    char bufLabel[BUFSIZ];
    char bufElse[BUFSIZ];
    char bufCode[BUFSIZ];
    Node *elseNode = NULL;
    int hasElse = 0; 
    int checkIf = 0;
    int currentId = labelId;
    NasmFunCall compFun;
    Symbol_table * globalTable;
    Node *cond = FIRSTCHILD(ifInstr);
    Symbol s;
    int value;
    for(Node *childIf = ifInstr->firstChild; childIf; childIf = childIf->nextSibling){
        hasElse = (childIf->label == Else) ? 1 : 0;
        if(hasElse){
            elseNode = childIf;
        }
        
    }
    sprintf(bufLabel, "%s%d", LABEL_IF, labelId);
    sprintf(bufCode, "%s%d", LABEL_CODE, currentId);
    if(hasElse) sprintf(bufElse, "%s%d", LABEL_ELSE, labelId);
    treatExpr(cond, list, funTable, bufLabel, bufElse, bufCode, hasElse);

    if(hasElse) {
        nasmCall(JMP, bufElse, NULL);
    } else {
        nasmCall(JMP, bufCode, NULL);
    }
    
    fprintf(f, "%s:\n", bufLabel);
    nasmTranslateParsing(SECONDCHILD(ifInstr), globalTable, list, funTable->name_table); //Parsing du IF
    nasmCall(JMP, bufCode, NULL);
    if(hasElse){
        fprintf(f, "%s:\n", bufElse);
        nasmTranslateParsing(FIRSTCHILD(elseNode), globalTable, list, funTable->name_table);
    }
    fprintf(f, "%s%d:\n", LABEL_CODE, currentId);
}


//TODO : boolean operations
void nasmTranslateParsing(Node *root, Symbol_table *global_var_table, List list, char *currentFunName){

    if(!root || root->label == Else) {
        return;
    }
    if(root->label == Assign){
        assign_global_var(getTableInListByName(currentFunName, list), f, root, list);
    }
    if(root->label == Putchar){
        writePutchar(root, getTableInListByName(currentFunName, list), list); 
    }
    if(root->label == Putint){
        Symbol_table *funTable = getTableInListByName(currentFunName, list);
        writePutint(root, list, funTable);
    }
    if(root->label == If){
        labelId += 1;
        fprintf(f, ";IF parsing started\n");
        ifInstr(root, list, getTableInListByName(currentFunName, list));
        nasmTranslateParsing(root->nextSibling, global_var_table, list, currentFunName);
        return;
    }
    if(root->label == While){
        //TODO while
        return;
    }

    nasmTranslateParsing(root->firstChild, global_var_table, list, currentFunName);
    nasmTranslateParsing(root->nextSibling, global_var_table, list, currentFunName);

}

void setFunctions(Node *fonctNode, List list){
    
    Symbol_table *global = getTableInListByName(GLOBAL, list);
    fprintf(f, "%s:", fonctNode->firstChild->firstChild->firstChild->u.ident);
    nasmTranslateParsing(SECONDCHILD(fonctNode), global, list, fonctNode->firstChild->firstChild->firstChild->u.ident);

}

static void translateMain(Node *root, Symbol_table *global, List list, Symbol_table *mainTable){
    Node *funs = SECONDCHILD(root);
    char buf[BUFSIZ];
    char numeric[10];
    for(Node *n = FIRSTCHILD(funs); n; n = n->nextSibling){
        char *nameFun = getFuncNameFromDecl(n);
        Symbol_table *funTable = getTableInListByName(nameFun, list);
        Symbol sTable = getSymbolInTableByName(funTable, nameFun);
        fprintf(f, "%s:\n", nameFun);
        nasmCall(PUSH, "rbp", NULL);
        nasmCall(MOV, "rbp", "rsp");
        sprintf(numeric, "%d", sTable.u.f_type.nb_local * 8);
        nasmCall(SUB, "rsp", numeric);
        //tree parsing
        nasmTranslateParsing(SECONDCHILD(n), global, list, nameFun);
        nasmCall(ADD, "rsp", numeric);
        nasmCall(MOV, "rsp", "rbp");
        nasmCall(POP, "rbp", NULL);
        fprintf(f, "\tret\n");
    }
}


void buildNasmFile(Node *root, List list){
    f = fopen("_anonymous.asm", "wr");
    Symbol_table *table, *mainTable;
    table = getTableInListByName(GLOBAL, list);
    mainTable = getTableInListByName("main", list);
    writeNasmHeader(table->total_size, list);
    translateMain(root, table, list, mainTable);
    end_asm();
}


void makeExecutable(char *fname){
    char buf[BUFSIZ];
    sprintf(buf, "nasm -f elf64 _anonymous.asm");
    system(buf);
    sprintf(buf, "gcc -o %s my_putchar.o _anonymous.o -nostartfiles -no-pie", fname);
    system(buf);
}