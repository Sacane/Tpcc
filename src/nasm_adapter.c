#include "nasm_adapter.h"



FILE *f;



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

void write_op(Node *opNode){
    char buf[BUFSIZ];
    char l_char[BUFSIZ];
    
    switch(opNode->u.byte){
        case '-':
            switch(FIRSTCHILD(opNode)->label){
                case Variable:
                case Int:
                    sprintf(l_char, "%d", FIRSTCHILD(opNode)->u.num);
                    break;

            }
            
            break;
        case '+':
            if(SECONDCHILD(opNode)->label == Variable){
                
            }
            break;
        default:
            break;
    }
}





static char* asm_to_string(NasmFunCall nasmFunction){
    char writer[10];
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
        case N_AND:   return "add";
            break;
        case N_OR:    return "or";
            break;
    }
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

int writeNasmFunction(NasmFunCall nasmFunCall, char *var1, char *var2)
{  
    if(!nasmFunArityCheck(nasmFunCall, var1, var2)){
        DEBUG("Fail to write into the nasm generator file : Incorrect usage of writer fun\n");
        return 0;
    }

    switch(nasmArityOf(nasmFunCall)){
        case 0:
            fprintf(f, "\t%s\n", asm_to_string(nasmFunCall));
            break;
        case 1:
            fprintf(f, "\t%s %s\n", asm_to_string(nasmFunCall), var1);
            break;
        case 2:
            fprintf(f, "\t%s %s, %s\n", asm_to_string(nasmFunCall), var1, var2);
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
                writeNasmFunction(PUSH, buf, NULL);
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
                
                writeNasmFunction(PUSH, buf, NULL);
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
                        raiseError(addSubNode->lineno, "Trying to divide by 0 (nuuul)\n");
                        return;
                    } else {
                        writeNasmFunction(PUSH, "0", NULL);
                        denominator = a;
                        break;
                    }
                }
                sprintf(buf2, "%d", a);
                writeNasmFunction(PUSH, buf2, NULL);
                
                break;
            case Variable:
                s = getSymbolInTableByName(symbolTable, SECONDCHILD(addSubNode)->u.ident);
                sprintf(buf2, "qword [global_var + %d]", s.offset);
                writeNasmFunction(PUSH, buf2, NULL);
                break;
            default:
                break;
        }
        
    }
    writeNasmFunction(POP, "rcx", NULL);
    writeNasmFunction(POP, "rax", NULL);
    switch(addSubNode->u.byte){
        case '-':
            writeNasmFunction(SUB, "rax", "rcx");
            break;
        case '+':
            writeNasmFunction(ADD, "rax", "rcx");
            break;
        case '*':
            writeNasmFunction(MUL, "rax", "rcx");
            break;
        case '/':
            sprintf(buf, "%d", denominator);
            writeNasmFunction(MOV, "bl", buf);
            writeNasmFunction(DIV, "bl", NULL);
            break;
        case '%':
            sprintf(buf, "%d", denominator);
            writeNasmFunction(DIV, buf, NULL);
            break;
    }
    if(addSubNode->u.byte == '%'){
        writeNasmFunction(PUSH, "rdx", NULL); 
        writeNasmFunction(MOV, "r12", "rdx");
    } else {
        writeNasmFunction(PUSH, "rax", NULL);
    }
    writeNasmFunction(MOV, "r12", "rax"); //Pour vérifier l'affichage avec show registers
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
            DEBUG("Assign character\n");
            c = rValue->u.byte;
            sprintf(buf, "'%c'", c);
            sprintf(buf2, "qword [global_var + %d]", lVar.offset);
            writeNasmFunction(MOV, buf2, buf);
            
            break;
        case Int:
            DEBUG("Assign integer\n");
            i = rValue->u.num;
            sprintf(buf, "%d", i);
            sprintf(buf2, "qword [global_var + %d]", lVar.offset);
            writeNasmFunction(MOV, buf2, buf);
            break;
        case Addsub:
        case divstar:
            DEBUG("Assign addition or substraction\n");
            opTranslate(rValue, symbolTableOfGlobal); // Put into r12 value of addition
            sprintf(buf2, "qword [global_var + %d]", lVar.offset);
            writeNasmFunction(MOV, buf2, "rax");
            writeNasmFunction(MOV, "rax", "0");
            break;
        default:
            raiseWarning(rValue->lineno, "Assign from variable are Not available in this version of compilation\n");
            return;

    }
}


//TODO
static void nasmTranslateParsing(Node *root, Symbol_table *global_var_table){

    if(!root) {
        return;
    }

    if(root->label == Assign && isSymbolInTable(global_var_table, FIRSTCHILD(root)->u.ident)){
        assign_global_var(global_var_table, f, root);
    }
    nasmTranslateParsing(root->firstChild, global_var_table);
    nasmTranslateParsing(root->nextSibling, global_var_table);
}


void buildNasmFile(Node *root, List list){
    f = fopen("_anonymous.asm", "wr");
    Symbol_table *table;
    table = getTableInListByName(GLOBAL, list);
    
    writeNasmHeader(table->total_size, list);
    nasmTranslateParsing(root, table); 
    end_asm();
}


void makeExecutable(char *fname){
    char buf[BUFSIZ];
    sprintf(buf, "nasm -f elf64 _anonymous.asm");
    system(buf);
    sprintf(buf, "gcc -o %s my_putchar.o _anonymous.o -nostartfiles -no-pie", fname);
    system(buf);
}