#include "nasm_adapter.h"



FILE *f;



void init_global_asm(int size){
    fprintf(f, "section .bss\n");
    fprintf(f, "global_var: resq %d\n", size);
}

void init_asm_(int total_global_size){

    init_global_asm(total_global_size);
    fprintf(f, "section  .text\nglobal  _start\nextern my_putchar\nextern my_getint\n_start:\n");   
    
}

void end_asm(){
    fprintf(f, "\tmov rax, 60\n\tmov rdi, 0\n\tsyscall\n");
    fclose(f);
}

void write_op(Node *op_node){
    char buf[BUFSIZ];
    char l_char[BUFSIZ];
    
    switch(op_node->u.byte){
        case '-':
            switch(FIRSTCHILD(op_node)->label){
                case Variable:
                case Int:
                    sprintf(l_char, "%d", FIRSTCHILD(op_node)->u.num);
                    break;

            }
            
            break;
        case '+':
            if(SECONDCHILD(op_node)->label == Variable){
                
            }
            break;
        default:
            break;
    }
}





static char* asm_to_string(NasmFunCall asm_fun){
    char writer[10];
    switch (asm_fun){
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

static int asm_arity_of(NasmFunCall fc){
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

static int asm_fun_checker(NasmFunCall nasmFunCall, char *var1, char *var2){
    switch (asm_arity_of(nasmFunCall)){
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

int insert_fun(NasmFunCall nasmFunCall, char *var1, char *var2)
{  
    if(!asm_fun_checker(nasmFunCall, var1, var2)){
        DEBUG("Fail to write into the nasm generator file : Incorrect usage of writer fun\n");
        return 0;
    }

    switch(asm_arity_of(nasmFunCall)){
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
void opTranslate(Node* addSubNode, Symbol_table *global_table){
    assert(addSubNode);
    char buf[BUFSIZ];
    char buf2[BUFSIZ];
    int denominator;
    Symbol s;
    int a, b;
    int offset;
    if(FIRSTCHILD(addSubNode)->label == Addsub || FIRSTCHILD(addSubNode)->label == divstar){
        opTranslate(FIRSTCHILD(addSubNode), global_table);
        
    } else {
        switch (FIRSTCHILD(addSubNode)->label){
            case Int:
            case Character: 
                a = addSubNode->firstChild->u.num;
                sprintf(buf, "%d", a);
                insert_fun(PUSH, buf, NULL);
                break;
            case Variable:
                s = getSymbolInTableByName(global_table, FIRSTCHILD(addSubNode)->u.ident);
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
                
                insert_fun(PUSH, buf, NULL);
                break;
            default:
                break;
        }
    }
    if(SECONDCHILD(addSubNode)->label == Addsub || SECONDCHILD(addSubNode)->label == divstar){
        opTranslate(SECONDCHILD(addSubNode), global_table);
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
                        insert_fun(PUSH, "0", NULL);
                        denominator = a;
                        break;
                    }
                }
                sprintf(buf2, "%d", a);
                insert_fun(PUSH, buf2, NULL);
                
                break;
            case Variable:
                s = getSymbolInTableByName(global_table, SECONDCHILD(addSubNode)->u.ident);
                sprintf(buf2, "qword [global_var + %d]", s.offset);
                insert_fun(PUSH, buf2, NULL);
                break;
            default:
                break;
        }
        
    }
    insert_fun(POP, "rcx", NULL);
    insert_fun(POP, "rax", NULL);
    switch(addSubNode->u.byte){
        case '-':
            insert_fun(SUB, "rax", "rcx");
            break;
        case '+':
            insert_fun(ADD, "rax", "rcx");
            break;
        case '*':
            insert_fun(MUL, "rax", "rcx");
            break;
        case '/':
            sprintf(buf, "%d", denominator);
            insert_fun(DIV, buf, NULL);
            break;
        case '%':
            sprintf(buf, "%d", denominator);
            insert_fun(DIV, buf, NULL);
            break;
    }
    if(addSubNode->u.byte == '%'){
        insert_fun(PUSH, "rdx", NULL); 
        insert_fun(MOV, "r12", "rdx");
    } else {
        insert_fun(PUSH, "rax", NULL);
    }
    
    insert_fun(MOV, "r12", "rax"); //Pour vérifier l'affichage avec show registers
    
}

//temporaire seulement pour variable global
void assign_global_var(Symbol_table *global_table, FILE* in, Node *assign_node){
    int i;
    char c;
    int pos;
    char buf[BUFSIZ];
    char buf2[BUFSIZ];
    Node *lValue = FIRSTCHILD(assign_node);
    Node *rValue = SECONDCHILD(assign_node);
    if(lValue->label == Int || lValue->label == Character){
        DEBUG("Error : trying to assign numeric or character\n");
        return;
    }
    if(rValue->label == Variable){
        DEBUG("Error : trying to assign variable to variable\n");
        return;
    }

    Symbol lVar = getSymbolInTableByName(global_table, lValue->u.ident);

    switch(rValue->label){
        case Character:
            DEBUG("Assign character\n");
            c = rValue->u.byte;
            sprintf(buf, "'%c'", c);
            sprintf(buf2, "qword [global_var + %d]", lVar.offset);
            insert_fun(MOV, buf2, buf);
            
            break;
        case Int:
            DEBUG("Assign integer\n");
            i = rValue->u.num;
            sprintf(buf, "%d", i);
            sprintf(buf2, "qword [global_var + %d]", lVar.offset);
            insert_fun(MOV, buf2, buf);
            break;
        case Addsub:
        case divstar:
            DEBUG("Assign addition or substraction\n");
            opTranslate(rValue, global_table); // Put into r12 value of addition
            sprintf(buf2, "qword [global_var + %d]", lVar.offset);
            insert_fun(MOV, buf2, "rax");
            insert_fun(MOV, "rax", "0");
            break;
        default:
            DEBUG("Assign from variable are Not available in this version of compilation\n");
            return;

    }
}

/*int treat_simple_sub_in_main(Node *root){
    Node* functions = root->firstChild->nextSibling;
    

    for(Node *function = functions->firstChild; function; function = function->nextSibling){
        
        if (!(strcmp(function->firstChild->firstChild->firstChild->u.ident, "main"))){
            parse_and_apply_substraction(function->firstChild->nextSibling);
        }

    }




    return 1;
}
*/

//TODO
void parse_tree(Node *root, Symbol_table *global_var_table){

    if(!root) {
        return;
    }

    if(root->label == Assign && isSymbolInTable(global_var_table, FIRSTCHILD(root)->u.ident)){
        assign_global_var(global_var_table, f, root);
        /*if(SECONDCHILD(root)->label == Addsub){
            opTranslate(SECONDCHILD(root), global_var_table);
        }*/
    }
    


    parse_tree(root->firstChild, global_var_table);
    parse_tree(root->nextSibling, global_var_table);
}


void buildNasmFile(Node *root, List list){
    f = fopen("_anonymous.asm", "wr");
    Symbol_table *table;
    table = getTableInListByName("global_vars", list);
    
    init_asm_(table->total_size);
    parse_tree(root, table); //TODO
    end_asm();
}


void makeExecutable(char *fname){
    char buf[BUFSIZ];
    sprintf(buf, "nasm -f elf64 _anonymous.asm");
    system(buf);
    sprintf(buf, "gcc -o %s my_putchar.o _anonymous.o -nostartfiles -no-pie", fname);
    system(buf);
}