#include "nasm_adapter.h"



FILE *f;

static char* stringOfNasmFun(NasmFunCall nasmFunction){
    switch (nasmFunction){
        case PUSH: return "push";
            break;
        case POP:  return "pop";
            break;
        case MOV:  return "mov";
            break;
        case ADD:  return "add";
            break;
        case SUB:  return "sub";
            break;
        case MUL:  return "imul";
            break;
        case DIV:  return "idiv";
            break;
        case NOT:  return "neg";
            break;
        case SYSCALL: return "syscall";
            break;
        case JMP:   return "jmp";
            break;
        case JG:    return "jg";
            break;
        case JE:    return "je";
            break;
        case RET:   return "ret";
            break;
        case N_AND: return "add";
            break;
        case N_OR:  return "or";
            break;
        case CALL:  return "call";
            break;
    }
}

static void initBss(int globalBssSize, List listTable){
    fprintf(f, "section .bss\n");
    fprintf(f, "global_var: resq %d\n", globalBssSize);
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
    
    fprintf(f, "section  .text\nglobal  _start\nextern my_putchar\nextern my_getint\n_start:\n");   
    
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
            return 1;
        case MOV:
        case ADD: 
        case SUB: 
        case MUL: 
        case N_AND: 
        case N_OR:  
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
                        sprintf(buf, "qword [global_var + %d]", s.offset);
                        break;
                    case CHAR:
                        sprintf(buf, "qword [global_var + %d]", s.offset);
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
                        denominator = a;
                        break;
                    }
                }
                sprintf(buf2, "%d", a);
                nasmCall(PUSH, buf2, NULL);
                
                break;
            case Variable:
                s = getSymbolInTableByName(symbolTable, SECONDCHILD(addSubNode)->u.ident);
                sprintf(buf2, "qword [global_var + %d]", s.offset);
                nasmCall(PUSH, buf2, NULL);
                break;
            default:
                break;
        }
        
    }
    nasmCall(POP, "rcx", NULL);
    nasmCall(POP, "rax", NULL);
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
            sprintf(buf, "%d", denominator);
            nasmCall(MOV, "bl", buf);
            nasmCall(DIV, "bl", NULL);
            break;
        case '%':
            sprintf(buf, "%d", denominator);
            nasmCall(DIV, buf, NULL);
            break;
    }
    if(addSubNode->u.byte == '%'){
        nasmCall(PUSH, "rdx", NULL); 
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
            sprintf(buf2, "qword [global_var + %d]", lVar.offset);
            nasmCall(MOV, buf2, buf);
            
            break;
        case Int:
            i = rValue->u.num;
            sprintf(buf, "%d", i);
            sprintf(buf2, "qword [global_var + %d]", lVar.offset);
            nasmCall(MOV, buf2, buf);
            break;
        case Addsub:
        case divstar:
            opTranslate(rValue, symbolTableOfGlobal); // Put into r12 value of addition
            sprintf(buf2, "qword [global_var + %d]", lVar.offset);
            nasmCall(MOV, buf2, "rax");
            nasmCall(MOV, "rax", "0");
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

//TODO : boolean operations
static void nasmTranslateParsing(Node *root, Symbol_table *global_var_table){
    if(!root) {
        DEBUG("return\n");
        return;
    }

    if(root->label == Assign && isSymbolInTable(global_var_table, FIRSTCHILD(root)->u.ident)){
        assign_global_var(global_var_table, f, root);
    }
    if(root->label == Putchar){
        DEBUG("Helo\n");
        writePutchar(root); 
    }

    nasmTranslateParsing(root->firstChild, global_var_table);
    nasmTranslateParsing(root->nextSibling, global_var_table);
}

static void orderInstr(Node *orderNode){

}

static void translateMain(Node *root, Symbol_table *global){
    Node *funs = SECONDCHILD(root);
    for(Node *n = FIRSTCHILD(funs); n; n = n->nextSibling){
        if(strcmp("main", n->firstChild->firstChild->firstChild->u.ident) == 0){
            DEBUG("main\n");
            nasmTranslateParsing(SECONDCHILD(n), global);
            return;
        }
    }
}


void buildNasmFile(Node *root, List list){
    f = fopen("_anonymous.asm", "wr");
    Symbol_table *table, *mainTable;
    table = getTableInListByName(GLOBAL, list);
    
    writeNasmHeader(table->total_size, list);

    translateMain(root, table);
    end_asm();
}


void makeExecutable(char *fname){
    char buf[BUFSIZ];
    sprintf(buf, "nasm -f elf64 _anonymous.asm");
    system(buf);
    sprintf(buf, "gcc -o %s my_putchar.o _anonymous.o -nostartfiles -no-pie", fname);
    system(buf);
}