#include "nasm.h"


#define LABEL_IF "startIF_"
#define LABEL_WHILE "startWHILE_"
#define LABEL_COND "labelCond_"
#define LABEL_ELSE "labelElse_"
#define LABEL_CODE "endIF_" 
#define LABEL_EXPR "labelExpr_"
#define LABEL_TRUE "TRUE"
#define LABEL_FALSE "FALSE"

#define LABEL_SWITCH_COND "labelCondSwitch_"
#define LABEL_CASE "labelCase_"
#define LABEL_DEFAULT "label_default:"
#define SWITCH_CHECK ".CHECK_"
#define LABEL_CODE_SWITCH "ENDWITCH"

#define OPERATOR 0
#define VAR 1
#define CONST 2
#define FUNCTION 3

#define IN_GLOBAL 0
#define IN_FUNCTION 1


FILE *f;
static int firstCall = 1;


static int labelId;



void nasmTranslateParsing(Node *root, Symbol_table *global_var_table, List list, char *currentFunctionName);
void functionCallInstr(Node *fCallNode, char *calledId, char *callerId, List list, int toAssign, int stage, char *target);




static void initBss(int globalBssSize, List listTable){
    

    fprintf(f, "section .bss\n");
    fprintf(f, "%s: resq %d\n", GLOBAL, globalBssSize);
    fprintf(f, "number: resq 1\n");
}

void writeNasmHeader(int globalBssSize, List listTable){
    

    fprintf(f, "section .data\n");
    fprintf(f, "\t formatInt: db \"%s\", 10, 0\n", "%d");
    fprintf(f, "\t formatIntIn: db \"%s\", 0\n", "%d");
    fprintf(f, "\t fmtChar: db \"%s\", 0\n", "%c");
    fprintf(f, "\t integer1: times 8 db 0\n");
    initBss(globalBssSize, listTable);
    


    fprintf(f, "section  .text\n \
    global  _start\n \
    extern my_putchar\n \
    extern show_registers\n \
    extern my_getint\n \
    extern printf\n \
    extern scanf\n");   
    
}




void call_safe(char *content){
    SUB("rsp", "8");
    CALL(content);
    ADD("rsp", "8");
}

void end_asm(){
    fprintf(f, "_start:\n");

    
    fprintf(f, "\tcall main\n");

    fprintf(f, "\tmov rax, 60\n\tmov rdi, 0\n\tsyscall\n");
    fclose(f);
}


void writeTestRegisters(){
    call_safe("show_registers");
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

/**
 * @brief 
 * 
 * @param content 
 */
static void callPrintf(char *content){
    
    MOV("rsi", content);
    
    MOV("rax", "0");
    MOV("rdi", "formatInt");
    call_safe("printf");
}

int isNegVar(Node *negNode){
    return (negNode->label == Addsub && negNode->u.byte == '-' && (NULL == SECONDCHILD(negNode)));
}

int negInstr(Node *negNode, Symbol_table *globalTable, Symbol_table *funTable, List list){
    char buf[BUFSIZ];
    if(!isNegVar(negNode)){
        return 0;
    }
    Node *child = FIRSTCHILD(negNode);
    Symbol varSym = getSymbolInTableByName(funTable, child->u.ident);
    int priority; 
    switch (child->label){
        case Variable:
            priority = symbolPriority(list, funTable, varSym.symbol_name);
            sprintf(buf, "qword [%s %s %d]", (priority == IN_GLOBAL) ? GLOBAL : "rbp", (varSym.offset >= 0) ? "+" : "", varSym.offset);
            break;
        case Int:
            sprintf(buf, "%d", (child->u.num));
            break;
        case Character:
            sprintf(buf, "%d", (child->u.byte));
            break;
        case FunctionCall:
            COMMENT("functionCallNeg");

            functionCallInstr(child, child->u.ident, funTable->name_table, list, 0, 0, NULL);

            NEG("rax");
            COMMENT("[END] functionCallNeg");

            return 1;
        default:
            break;
    }
    MOV("rax", buf);
    NEG("rax");
    return 1;
}

/* We suppose node parameter in the body of the main */
/**
 * @brief 
 * 
 * @param addSubNode 
 * @param symbolTable 
 */
void opTranslate(Node* addSubNode, Symbol_table *symbolTable, List list, int stage, char *currentFunId){
    assert(addSubNode);
    char buf[BUFSIZ];
    char buf2[BUFSIZ];
    int denominator;
    Symbol s;
    int a, b;
    int offset;
    int priority;
    Symbol_table *globalTable = getTableInListByName(GLOBAL, list);
    Symbol_table *currentFunTable;
    int isLeftInGlobal = 0, isRightInGlobal = 0;
    if(negInstr(addSubNode, globalTable, symbolTable, list)){
        return;
    }
    if(FIRSTCHILD(addSubNode)->label == Addsub || FIRSTCHILD(addSubNode)->label == divstar){
        opTranslate(FIRSTCHILD(addSubNode), symbolTable, list, stage+1, currentFunId);
    } else {
        switch (FIRSTCHILD(addSubNode)->label){
            case Int:
            case Character: 
                a = addSubNode->firstChild->u.num;
                sprintf(buf, "%d", a);
                PUSH(buf);
                break;
            case Variable:
                if(((priority = symbolPriority(list, symbolTable, FIRSTCHILD(addSubNode)->u.ident)) == IN_FUNCTION)){
                    s = getSymbolInTableByName(symbolTable, FIRSTCHILD(addSubNode)->u.ident);
                } else {
                    s = getSymbolInTableByName(globalTable, FIRSTCHILD(addSubNode)->u.ident);
                }

                sprintf(buf, "qword [%s %s %d]",(priority == IN_GLOBAL) ? GLOBAL : "rbp", (s.offset >= 0) ? "+" : "", s.offset);

                
                PUSH(buf);
                break;
            default:
                break;
        }
        
    }
    if(SECONDCHILD(addSubNode)->label == Addsub || SECONDCHILD(addSubNode)->label == divstar){
        opTranslate(SECONDCHILD(addSubNode), symbolTable, list, stage+1, currentFunId);
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
                        PUSH("0");
                    }
                }
                sprintf(buf2, "%d", a);
                PUSH(buf2);
                
                break;
            case Variable:
                if(((priority = symbolPriority(list, symbolTable, SECONDCHILD(addSubNode)->u.ident)) == IN_FUNCTION)){
                    s = getSymbolInTableByName(symbolTable, SECONDCHILD(addSubNode)->u.ident);
                } else {
                    s = getSymbolInTableByName(globalTable, SECONDCHILD(addSubNode)->u.ident);

                }

                sprintf(buf2, "qword [%s %s %d]",(priority == IN_GLOBAL) ? GLOBAL : "rbp", (s.offset >= 0) ? "+" : "", s.offset);

                PUSH(buf2);
                break;
            default:
                break;
        }
        
    }
    if(addSubNode->label == Addsub || addSubNode->u.byte == '*'){
        POP("rcx");
        POP("rax");
    } else {
        POP("rbx");
        PUSH("0");
        POP("rdx");
        POP("rax");
    }

    switch(addSubNode->u.byte){
        case '-':
            SUB("rax", "rcx");
            break;
        case '+':
            ADD("rax", "rcx");
            break;
        case '*':
            MUL("rax", "rcx");
            break;
        case '/':
        case '%':
            DIV("rbx");
            break;
    }
    if(addSubNode->u.byte == '%'){
        PUSH("rdx"); 
        MOV("rax", "rdx");
        MOV("r12", "rdx");
    } else {
        PUSH("rax");
    }
    MOV("r12", "rax"); //Pour vérifier l'affichage avec show registers
    if(stage == 0){
        POP("rax");
    }
    
}

/*=================== Function setup ============ */

void callScanf(char *buf, char *format){
    MOV("r9", "rsp");
    AND("spl", "240");
    SUB("rsp", "16");
    PUSH("r9");
    MOV("rsi", buf);
    MOV("rdi", format);
    MOV("rax", "0");
    call_safe("scanf");

}

void callGetint(char *buf){
    callScanf("number", FMTINT);
    MOV("rax", "qword [number]");
    MOV(buf, "rax");
    MOV("rax", "0");

}

/*=====================================================*/
void assignInstr(Symbol_table *symbolTable, FILE* in, Node *assign_node, List list){
    int i;
    char c;
    int pos;
    char buf[BUFSIZ];
    char buf2[BUFSIZ];
    int priority;
    int isGlobalLayer = 0;
    Symbol lVar, rVal;
    Node *lValue = FIRSTCHILD(assign_node);
    Node *rValue = SECONDCHILD(assign_node);
    Symbol_table *globalTable = getTableInListByName(GLOBAL, list);
    if(lValue->label == Int || lValue->label == Character){
        raiseError(assign_node->lineno, "trying to assign numeric or character\n");
        return;
    }
    if(isSymbolInTable(globalTable, lValue->u.ident)){
        lVar = getSymbolInTableByName(globalTable, lValue->u.ident);
        isGlobalLayer = 1;
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
            sprintf(buf, "%d", c);
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            MOV(buf2, buf);
            break;
        case Int:
            i = rValue->u.num;
            sprintf(buf, "%d", i);
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            MOV(buf2, buf);
            break;
        case Addsub:
        case divstar:
            opTranslate(rValue, symbolTable, list, 0, symbolTable->name_table); // Put into r12 value of addition
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            MOV(buf2, "rax");
            MOV("rax", "0");
            MOV("rbx", "0");
            break;
        case Getint:
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            callGetint(buf2);
            break;
        case Getchar:
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            callScanf("number", FMTCHAR);
            MOV("rax", "qword [number]");
            MOV(buf2, "rax");
            MOV("rax", "0");
            break;
        case FunctionCall:
            COMMENT("[START] Assign to a function call");
            functionCallInstr(rValue, rValue->u.ident,  symbolTable->name_table, list, 0, 0, NULL);
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            MOV(buf2, "rax");
            COMMENT("[END] Assign to a function call");
            break;
        case Variable:
            COMMENT("assign variable");
            priority = symbolPriority(list, symbolTable, rValue->u.ident);
            rVal = getSymbolInTableByName((priority == IN_GLOBAL) ? globalTable : symbolTable, rValue->u.ident);
            sprintf(buf, "qword [%s %s %d]", (priority == IN_FUNCTION) ? "rbp" : GLOBAL, (rVal.offset >= 0) ? "+" : "", rVal.offset);
            MOV("rax", buf);
            sprintf(buf2, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (lVar.offset >= 0) ? "+" : "", lVar.offset);
            MOV(buf2, "rax");
            break;
        default:
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
        case FunctionCall:
            return FUNCTION;
        default:
            return -1;
    }
}



static void writePutint(Node *putIntNode, List list, Symbol_table *funTable){
    char buf[BUFSIZ];
    int n;
    Symbol s;
    int priority;
    Symbol_table *globalTable = getTableInListByName(GLOBAL, list);
    Node *child = FIRSTCHILD(putIntNode);
    switch(checkNodeContent(child)){
        case CONST:
            COMMENT("putint to const");
            n = child->u.num;
            break;
        case VAR:
            priority = symbolPriority(list, funTable, child->u.ident);
            s = getSymbolInTableByName((priority == IN_FUNCTION) ? funTable : globalTable, child->u.ident);
            sprintf(buf, "qword [%s %s %d]", (priority == IN_FUNCTION) ? "rbp" : GLOBAL, (s.offset >= 0) ? "+" : "", s.offset);
            callPrintf(buf);
            
            return;
        case OPERATOR:
            opTranslate(child, funTable, list, 0, funTable->name_table);
            callPrintf("rax");
            return;
        case FUNCTION:
            COMMENT("functionCall");
            functionCallInstr(child, child->u.ident, funTable->name_table, list, 0, 0, NULL);
            
            callPrintf("rax");
            COMMENT("END functionCall");
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
                    MOV("rdi", buf);
                    CALL("my_putchar");
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
            
            sprintf(buf, "qword [%s %s %d]", (contentLayer == IN_GLOBAL) ? GLOBAL : "rbp", (variable.offset >= 0) ? "+" : "", variable.offset);
            break;
    }

    MOV("rdi", buf);
    CALL("my_putchar");
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
            opTranslate(condNode, funTable, list, 0, funTable->name_table);  //rax
            MOV("r14", "rax");
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

            sprintf(buf, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (s.offset >= 0) ? "+" : "", s.offset);
            MOV("r14", buf);
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
            MOV("r14", buf);
        }
        break;
    }

    switch(checkNodeContent(opRight)){
        case OPERATOR: {
            opTranslate(condNode, funTable, list, 0, funTable->name_table);  //rax
            MOV("r15", "rax");

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

            sprintf(buf, "qword [%s %s %d]", (isGlobalLayer) ? GLOBAL : "rbp", (s.offset >= 0) ? "+" : "", s.offset);
            MOV("r15", buf);
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
            MOV("r15", buf);
        }
    }
}

void treatExpr(Node *conditionNode, List list, Symbol_table *funTable, char *labelIf, char *labelElse, char *labelCode, int hasElse){
    fprintf(f, ";treatExpr label : %d\n", conditionNode->label);
    Symbol s;
    NasmFunCall compFun;
    Symbol_table* globalTable;
    char buf[BUFSIZ];
    switch(conditionNode->label){
        case FunctionCall:
            COMMENT("ah");
            functionCallInstr(conditionNode, conditionNode->u.ident, funTable->name_table, list, 0, 0, NULL);
            MOV("rbx", "0");
            ADD("rbx", "rax");
            JG(labelIf);
            
            break;
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
                sprintf(buf, "qword [%s %s %d]", "rbp", (s.offset >= 0) ? "+" : "", s.offset);
            }
            MOV("rbx", "0");
            ADD("rbx", buf);
            JG(labelIf);
            break;
        case Int:
            sprintf(buf, "%d", conditionNode->u.num);
            MOV("rbx", "0");
            ADD("rbx", buf);
            JG(labelIf);
            break;
        case Eq:
        case Order:

            compareInstrAux(conditionNode, list, funTable);
            CMP("r14", "r15");
            
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
            MOV("r14", "0");
            MOV("r15", "0");
            break;
        case Addsub:
        case divstar:
            opTranslate(conditionNode, funTable, list, 0, funTable->name_table); //Resultat de l'opération dans rax
            ADD("rbx", "rax");
            JG(labelIf);

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
            JMP(labelCode);
            fprintf(f, "%s:\n", buf);
            labelId += 1;
            treatExpr(conditionNode->firstChild, list, funTable, labelIf, labelElse, labelCode, hasElse);
            break;
        default:
            break;
    }
}
void whileInstr(Node *whileNode, char *currentFunName, List list){
    char bufWhile[BUFSIZ];
    char bufCond[BUFSIZ];
    char bufEnd[BUFSIZ];
    labelId += 1;
    int currentId = labelId;
    Node *condNode = FIRSTCHILD(whileNode);
    sprintf(bufWhile, "%s%d", LABEL_WHILE, labelId);
    sprintf(bufCond, "%s%d", LABEL_COND, labelId);
    sprintf(bufEnd, "%s%d", LABEL_CODE, currentId);
    fprintf(f, "%s:", bufCond);
    treatExpr(condNode, list, getTableInListByName(currentFunName, list), bufWhile, NULL, bufEnd, 0);
    JMP(bufEnd);
    fprintf(f, "%s:\n", bufWhile);
    nasmTranslateParsing(SECONDCHILD(whileNode), getTableInListByName(GLOBAL, list), list, currentFunName);
    JMP(bufCond);
    fprintf(f, "%s:\n", bufEnd);
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
        JMP(bufElse);
    } else {
        JMP(bufCode);
    }
    
    fprintf(f, "%s:\n", bufLabel);
    nasmTranslateParsing(SECONDCHILD(ifInstr), globalTable, list, funTable->name_table); //Parsing du IF
    JMP(bufCode);
    if(hasElse){
        fprintf(f, "%s:\n", bufElse);
        nasmTranslateParsing(FIRSTCHILD(elseNode), globalTable, list, funTable->name_table);
    }
    fprintf(f, "%s%d:\n", LABEL_CODE, currentId);
}

void returnInstr(Node *returnNode, List list, char *currentFunId){
    int priority;
    char buf[BUFSIZ];

    if(!returnNode->firstChild){
        sprintf(buf, "end%s", currentFunId);
        JMP(buf);
        return;
    }

    Symbol_table *currentTable = getTableInListByName(currentFunId, list);
    Symbol_table *global = getTableInListByName(GLOBAL, list);
    Symbol funSym = getSymbolInTableByName(currentTable, currentTable->name_table);
    Symbol valueSym;
    Node *valueNode = returnNode->firstChild;
    
    switch (checkNodeContent(valueNode))
    {
    case OPERATOR:
        opTranslate(valueNode, currentTable, list, 0, currentFunId);
        sprintf(buf, "%d", (valueNode->label == Int) ? valueNode->u.num : valueNode->u.byte);
        break;
    case VAR:
        priority = symbolPriority(list, currentTable, valueNode->u.ident);
        valueSym = getSymbolInTableByName((priority == IN_GLOBAL) ? global : currentTable, valueNode->u.ident);
        sprintf(buf, "qword [%s %s %d]", (priority == IN_GLOBAL) ? GLOBAL : "rbp", (valueSym.offset >= 0) ? "+" : "", valueSym.offset);
        MOV("rax", buf);
        break;
    case CONST:
        sprintf(buf, "%d", (valueNode->label == Int) ? valueNode->u.num : valueNode->u.byte);
        MOV("rax", buf);
        break;
    case FUNCTION:
        functionCallInstr(valueNode, valueNode->u.ident, currentFunId, list, 0, 0, NULL);
        POP("rax");
        break;
    default:
        break;
    }

    sprintf(buf, "end%s", currentFunId);
    JMP(buf);

}


int isLastCase(Node *caseNode){
    for(Node *sibling = caseNode->nextSibling; sibling; sibling = sibling->nextSibling){
        if(!sibling || sibling->label == Case){
            return 0;
        }
    }
    return 1;
}

void switchInstr(Node *switchNode, char *currentFunId, List list){
    char bufCond[BUFSIZ];
    char bufCode[BUFSIZ];
    char buf[BUFSIZ];
    char bufCase[BUFSIZ];
    char bufCheck[BUFSIZ];
    int hasDefault = (hasLabel(switchNode, Default)) ? 1 : 0;
    int priority;
    int tmp;
    int cpt = 0;
    labelId += 1;
    int saveLabelId = labelId;
    Symbol_table *funTable = getTableInListByName(currentFunId, list);
    Symbol_table *globalTable = getTableInListByName(GLOBAL, list);



    MOV("r13", "0"); //Notre boolean

    sprintf(bufCheck, "%s%d%d", SWITCH_CHECK, saveLabelId, cpt); //.CHECK_XY
    sprintf(bufCond, "%s%d%d", LABEL_SWITCH_COND, saveLabelId, cpt);
    sprintf(bufCode, "%s%d", LABEL_CODE_SWITCH, saveLabelId);
    sprintf(bufCase, "%s%d%d", LABEL_CASE, saveLabelId, cpt);
    fprintf(f, "%s%d:\n", LABEL_SWITCH_COND, saveLabelId);
    
    //récupérer l'endroit où est stocker la fils du switch,  on fout le resultat dans r14
    switch(checkNodeContent(switchNode->firstChild)){
        case OPERATOR:
            opTranslate(switchNode->firstChild, funTable, list, 0, currentFunId);
            MOV("r14", "rax");
            break;
        case VAR:   
            priority = symbolPriority(list, funTable, switchNode->firstChild->u.ident);
            Symbol s = getSymbolInTableByName((priority == IN_FUNCTION) ? funTable : globalTable, switchNode->firstChild->u.ident);
            sprintf(buf, "qword [%s %s %d]", (priority == IN_GLOBAL) ? GLOBAL : "rbp", (s.offset >= 0) ? "+" : "", s.offset);
            MOV("r14", buf);
            break;
        case CONST:
            if(switchNode->firstChild->label == Int){
                sprintf(buf, "%d", switchNode->firstChild->u.num);
            } else {
                sprintf(buf, "%c", switchNode->firstChild->u.byte);
            }
            MOV("r14", buf);

            break;
        default:
            raiseError(switchNode->lineno, "can't evaluate orders in switch\n");
            check_sem_err = 1;
            break;
    }
    
    nasmTranslateParsing(FIRSTCHILD(SECONDCHILD(switchNode)), globalTable, list, currentFunId);

    for(Node *child = THIRDCHILD(switchNode); child; child = child->nextSibling){
        /*if(child->label == Break){
            nasmCall(JMP, bufCode, NULL);
        }*/
        //Pour chaque case qu'on rencontre
        if(child->label == Case){

            //on récupère le caseValue on le met dans r12
            Node *caseValue = child->firstChild;
            if(caseValue->label == Int){
                sprintf(buf, "%d", caseValue->u.num);
            } 
            if(caseValue->label == Character){
                sprintf(buf, "'%c'", caseValue->u.byte);
            }

            cpt += 1;
            tmp = cpt+1;
            sprintf(bufCase, "%s%d%d", LABEL_CASE, saveLabelId, cpt);
            sprintf(bufCheck, "%s%d%d", SWITCH_CHECK, saveLabelId, cpt);
            sprintf(bufCond, "%s%d%d", LABEL_SWITCH_COND, saveLabelId, cpt);
            fprintf(f,"%s:\n", bufCond);
            MOV("r12", buf);
            CMP("r12", "r14"); // On compare r14 et r12
            if(isLastCase(child)){
                if(hasDefault){
                    JNE("label_default");
                } else {
                    JNE(bufCode);
                }
            } else {
                
                sprintf(buf, "%s%d%d", LABEL_SWITCH_COND, saveLabelId, tmp);
                JNE(buf);
            }
            JE(bufCase);
            

            fprintf(f, "%s:\n", bufCase);
            MOV("r13", "1");
            //Parsing du case
            nasmTranslateParsing(SECONDCHILD(child), globalTable, list, currentFunId);
            COMMENT("End parsing case");
            if(child->nextSibling->label == Break){
                JMP(bufCode);
            } 
            else {
                if(!isLastCase(child)){
                    sprintf(buf, "%s%d%d", LABEL_CASE, saveLabelId, tmp);
                    JMP(buf);
                }
            }

            
            COMMENT("Here you're supposed to have smth");
        }
        if(child->label == Default){
            fprintf(f, "%s\n", LABEL_DEFAULT);
            nasmTranslateParsing(child->firstChild, globalTable, list, currentFunId);
        }

    }
    MOV("r14", "0");
    fprintf(f, "%s:\n", bufCode);
}


void functionCallInstr(Node *fCallNode, char *calledId, char *callerId, List list, int toAssign, int stage, char *target){
    Symbol paramSymbol;
    int priority;
    char buf[BUFSIZ];
    if(strcmp(calledId, callerId) == 0){
//mov rsp, rbp
        MOV("rsp", "rbp");
    }
    Symbol_table *callerTable = getTableInListByName(callerId, list);
    Symbol_table *calledTable = getTableInListByName(calledId, list);
    Symbol_table *globalTable;
    if(!(fCallNode->firstChild->label == Void)){  
        for(Node *paramNode = fCallNode->firstChild; paramNode; paramNode = paramNode->nextSibling){
            switch (paramNode->label)
            {
            case Variable:
                globalTable = getTableInListByName(GLOBAL, list);
                priority = symbolPriority(list, callerTable, paramNode->u.ident);
                paramSymbol = getSymbolInTableByName((priority == IN_GLOBAL) ? globalTable : callerTable, paramNode->u.ident);
                sprintf(buf, "qword [%s %s %d]", (priority == IN_GLOBAL) ? GLOBAL : "rbp", (paramSymbol.offset >= 0) ? "+" : "", paramSymbol.offset);
                PUSH(buf);
                break;
            case Int:
                sprintf(buf, "%d", paramNode->u.num);
                PUSH(buf);
                break;
            case Character:
                if(paramNode->u.byte == '\n'){
                    sprintf(buf, "\\n");
                } else {
                    sprintf(buf, "%d", paramNode->u.byte);
                }
                
                PUSH(buf);
                break;
            case FunctionCall:
                functionCallInstr(paramNode, paramNode->u.ident, callerId, list, 0, 0, NULL);
                PUSH("rax");
                break;
            case Addsub:
            case divstar:
                opTranslate(paramNode, callerTable, list, 0, callerId);
                PUSH("rax");
                break;
            default:
                break;
            }
        }
    }
    
    CALL(calledId);
}



void nasmTranslateParsing(Node *root, Symbol_table *global_var_table, List list, char *currentFunName){

    if(!root) {
        return;
    }
    
    Symbol_table * curFunTable = getTableInListByName(currentFunName, list);
    
    switch(root->label){
        case Else:
            return;
        case Assign:
            assignInstr(curFunTable, f, root, list);
            nasmTranslateParsing(root->nextSibling, global_var_table, list, currentFunName);
            return;
        case Putchar:
            writePutchar(root, curFunTable, list);
            break;
        case Putint:
            writePutint(root, list, curFunTable);
            nasmTranslateParsing(root->nextSibling, global_var_table, list, currentFunName);
            return;
        case If:
            labelId += 1;
            ifInstr(root, list, curFunTable);
            nasmTranslateParsing(root->nextSibling, global_var_table, list, currentFunName);
            return;
        case While:
            COMMENT("While started");
            whileInstr(root, currentFunName, list);
            COMMENT("end while");
            nasmTranslateParsing(root->nextSibling, global_var_table, list, currentFunName);
            return;
        case Return:
            returnInstr(root, list, currentFunName);
            break;
        case FunctionCall:
            functionCallInstr(root, root->u.ident, currentFunName, list, 0, 0, NULL);
            break;
        case Switch:
            COMMENT("Switch start");
            switchInstr(root, currentFunName, list);
            COMMENT("Switch end");
            nasmTranslateParsing(root->nextSibling, global_var_table, list, currentFunName);
            return;
        default:
            break;
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
        PUSH("rbp");
        MOV("rbp", "rsp");
        sprintf(numeric, "%d", sTable.u.f_type.nb_local * 8);
        SUB("rsp", numeric);
        nasmTranslateParsing(SECONDCHILD(n), global, list, nameFun);
        sprintf(numeric, "%d", (sTable.u.f_type.nb_local + sTable.u.f_type.nb_args) * 8);
        fprintf(f, "\tend%s:\n", nameFun);
        ADD("rsp", numeric);
        MOV("rsp", "rbp");
        POP("rbp");
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