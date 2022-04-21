#include "nasm_adapter.h"


#define LABEL_IF "labelId_"
#define LABEL_ELSE "labelElse_"
#define LABEL_CODE "labelCode_" 
#define OPERATOR 0
#define VAR 1
#define CONST 2

FILE *f;



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
    for(List l = listTable; l; l = l->next){
        Symbol_table *st = l->table;
        if(strcmp(st->name_table, GLOBAL) == 0){
            continue;
        }
        fprintf(f, "%s: resq %d\n", st->name_table, st->total_size);
    }
}

void writeNasmHeader(int globalBssSize, List listTable){
    

    initBss(globalBssSize, listTable);
    
    fprintf(f, "section  .text\nglobal  _start\nextern my_putchar\nextern show_registers\nextern my_getint\n_start:\n");   
    
}

void end_asm(){
    fprintf(f, "\tmov rax, 60\n\tmov rdi, 0\n\tsyscall\n");
    fclose(f);
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


/* We suppose node parameter in the body of the main */
/**
 * @brief 
 * 
 * @param addSubNode 
 * @param symbolTable 
 */
void opTranslate(Node* addSubNode, Symbol_table *symbolTable){
    assert(addSubNode);
    char buf[BUFSIZ];
    char buf2[BUFSIZ];
    int denominator;
    Symbol s;
    int a, b;
    int offset;
    if(FIRSTCHILD(addSubNode)->label == Addsub || FIRSTCHILD(addSubNode)->label == divstar){
        opTranslate(FIRSTCHILD(addSubNode), symbolTable);
    } else {
        switch (FIRSTCHILD(addSubNode)->label){
            case Int:
            case Character: 
                a = addSubNode->firstChild->u.num;
                sprintf(buf, "%d", a);
                nasmCall(PUSH, buf, NULL);
                break;
            case Variable:
                s = getSymbolInTableByName(symbolTable, FIRSTCHILD(addSubNode)->u.ident);
                switch(s.u.p_type){
                    case INT:
                        sprintf(buf, "qword [%s + %d]",GLOBAL, s.offset);
                        break;
                    case CHAR:
                        sprintf(buf, "qword [%s + %d]",GLOBAL, s.offset);
                        break;
                    default:
                        break;
                }
                
                nasmCall(PUSH, buf, NULL);
                break;
            default:
                break;
        }
        
    }
    if(SECONDCHILD(addSubNode)->label == Addsub || SECONDCHILD(addSubNode)->label == divstar){
        opTranslate(SECONDCHILD(addSubNode), symbolTable);
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
                s = getSymbolInTableByName(symbolTable, SECONDCHILD(addSubNode)->u.ident);
                sprintf(buf2, "qword [%s + %d]",GLOBAL, s.offset);
                nasmCall(PUSH, buf2, NULL);
                break;
            default:
                break;
        }
        
    }
    if(!((addSubNode->u.byte == '/') && !(addSubNode->u.byte == '%'))){
        nasmCall(POP, "rcx", NULL);
        nasmCall(POP, "rax", NULL);
    } else {
        nasmCall(POP, "rbx", NULL);
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

}

//temporaire seulement pour variable global
void assign_global_var(Symbol_table *symbolTableOfGlobal, FILE* in, Node *assign_node){
    int i;
    char c;
    int pos;
    char buf[BUFSIZ];
    char buf2[BUFSIZ];
    Node *lValue = FIRSTCHILD(assign_node);
    Node *rValue = SECONDCHILD(assign_node);
    if(lValue->label == Int || lValue->label == Character){
        raiseError(assign_node->lineno, "trying to assign numeric or character\n");
        return;
    }
    Symbol lVar = getSymbolInTableByName(symbolTableOfGlobal, lValue->u.ident);
    switch(rValue->label){
        case Character:
            c = rValue->u.byte;
            sprintf(buf, "'%c'", c);
            sprintf(buf2, "qword [%s + %d]", GLOBAL, lVar.offset);
            nasmCall(MOV, buf2, buf);
            
            break;
        case Int:
            i = rValue->u.num;
            sprintf(buf, "%d", i);
            sprintf(buf2, "qword [%s + %d]", GLOBAL, lVar.offset);
            nasmCall(MOV, buf2, buf);
            break;
        case Addsub:
        case divstar:
            opTranslate(rValue, symbolTableOfGlobal); // Put into r12 value of addition
            sprintf(buf2, "qword [%s + %d]", GLOBAL, lVar.offset);
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

static void writePutchar(Node *putcharNode){
    char buf[BUFSIZ];
    char c = FIRSTCHILD(putcharNode)->u.byte;
    if(c == '\n'){
        sprintf(buf, "%d", 13);
        nasmCall(MOV, "rdi", buf);
        nasmCall(CALL, "my_putchar", NULL);
        sprintf(buf, "%d", 10);
        nasmCall(MOV, "rdi", buf);
        nasmCall(CALL, "my_putchar", NULL);
        return;
    } else {
        sprintf(buf, "'%c'", c);
    }
    nasmCall(MOV, "rdi", buf);
    nasmCall(CALL, "my_putchar", NULL);
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
            opTranslate(condNode, funTable);  //rax
            nasmCall(MOV, "r14", "rax");

        }
        break;
        case VAR: {
            if(isSymbolInTable(global, opLeft->u.ident)){
                s = getSymbolInTableByName(global, opLeft->u.ident);
            } 
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
            opTranslate(condNode, funTable);  //rax
            nasmCall(MOV, "r15", "rax");

        }
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

    switch(cond->label){
        case Variable:
            globalTable = getTableInListByName(GLOBAL, list);
            //Global
            if(isSymbolInTable(globalTable, cond->u.ident)){
                s = getSymbolInTableByName(globalTable, cond->u.ident);
                sprintf(buf, "qword [%s + %d]", GLOBAL, s.offset);
            }
            //Local
            if(isSymbolInTable(funTable, cond->u.ident)){
                s = getSymbolInTableByName(funTable, cond->u.ident);
                sprintf(buf, "qword [%s + %d]", funTable->name_table, s.offset);
            }
            nasmCall(ADD, "rbx", buf);
            nasmCall(JG, bufLabel, NULL);
            break;
        case Int:
            sprintf(buf, "%d", cond->u.num);
            nasmCall(ADD, "rbx", buf);
            nasmCall(JG, bufLabel, NULL);
            break;
        case Order:
            compareInstrAux(cond, list, funTable);
            nasmCall(CMP, "r14", "r15");
            
            if(strcmp(cond->u.ident, "<=") == 0){
                compFun = JLE;
            }
            if (strcmp(cond->u.ident, ">=") == 0){
                compFun = JGE;
            }
            if(strcmp(cond->u.ident, "<") == 0){
                compFun = JL;
            }
            if(strcmp(cond->u.ident, ">") == 0){
                compFun = JG;
            }
            if(strcmp(cond->u.ident, "==") == 0){
                compFun = JE;
            }
            if(strcmp(cond->u.ident, "!=") == 0){
                compFun = JNE;
            }

            nasmCall(compFun, bufLabel, NULL);
            nasmCall(MOV, "r14", "0");
            nasmCall(MOV, "r15", "0");
            break;
        case Addsub:
        case divstar:
            opTranslate(cond, funTable); //Resultat de l'opération dans rax
            nasmCall(ADD, "rbx", "rax");
            nasmCall(JG, bufLabel, NULL);

            break;
        default:
            break;
    }
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

    if(root->label == Assign && isSymbolInTable(global_var_table, FIRSTCHILD(root)->u.ident)){
        assign_global_var(global_var_table, f, root);
    }
    if(root->label == Putchar){
        writePutchar(root); 
    }
    if(root->label == If){
        labelId += 1;
        ifInstr(root, list, getTableInListByName(currentFunName, list));
        nasmTranslateParsing(root->nextSibling, global_var_table, list, currentFunName);
        return;
    }

    nasmTranslateParsing(root->firstChild, global_var_table, list, currentFunName);
    nasmTranslateParsing(root->nextSibling, global_var_table, list, currentFunName);

}


static void ifElseInstr(Node *rootIf){

}

static void translateMain(Node *root, Symbol_table *global, List list, Symbol_table *mainTable){
    Node *funs = SECONDCHILD(root);
    for(Node *n = FIRSTCHILD(funs); n; n = n->nextSibling){
        if(strcmp("main", n->firstChild->firstChild->firstChild->u.ident) == 0){
            nasmTranslateParsing(SECONDCHILD(n), global, list, "main");
            return;
        }
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