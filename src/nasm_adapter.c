#include "nasm_adapter.h"



FILE *f;



void init_global_asm(int size){
    fprintf(f, "section .bss\n");
    fprintf(f, "global_var: resp %d\n", size);
}

void init_asm_(int total_global_size){

    init_global_asm(total_global_size);
    fprintf(f, "section  .text\nglobal  _start\n_start:\n");   
    
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
        case MUL:  return "mul";
            break;
        case DIV:  return "div";
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
            return 1;
        case MOV:
        case ADD: 
        case SUB: 
        case MUL: 
        case DIV: 
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
            fprintf(f, "\t%s %s %s\n", asm_to_string(nasmFunCall), var1, var2);
            break;
        default:
            break;
    }
    return 1;

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

    Symbol lVar = get_symbol_by_name(global_table, lValue->u.ident);

    switch(rValue->label){
        case Character:
            DEBUG("Assign character\n");
            c = rValue->u.byte;
            sprintf(buf, "'%c'", c);
            sprintf(buf2, "byte [resp + %d]", lVar.offset);
            insert_fun(MOV, buf, buf2);
            
            break;
        case Int:
            DEBUG("Assign integer\n");
            i = rValue->u.num;
            sprintf(buf, "%d", i);
            sprintf(buf2, "dword [resp + %d]", lVar.offset);
            insert_fun(MOV, buf, buf2);
            break;
        default:
            DEBUG("Not available in this version of compilation\n");
            return;

    }
}


void write_global_eval(Symbol_table *global_table, Node *assign_node){

    
    Node *l_value = assign_node->firstChild;
    Node *r_value = l_value->nextSibling;

    if(!is_symbol_in_table(global_table, l_value->u.ident) || is_symbol_in_table(global_table, r_value->u.ident)){
        return;
    }

}   

/*
int treat_simple_sub_in_main(Node *root){
    FILE* out;
    Node* functions = root->firstChild->nextSibling;
    
    out = fopen("_anonymous.asm", "w");

    init_asm_(OPEN, out);

    for(Node *function = functions->firstChild; function; function = function->nextSibling){
        
        if (!(strcmp(function->firstChild->firstChild->firstChild->u.ident, "main"))){
            parse_and_apply_substraction(function->firstChild->nextSibling, out);
        }

    }

    init_asm_(CLOSE, out);
    fclose(out);


    return 1;
}*/


//TODO
void parse_tree(Node *root, Symbol_table *global_var_table){

    if(!root) {
        return;
    }

    if(root->label == Assign && is_symbol_in_table(global_var_table, FIRSTCHILD(root)->u.ident)){
        assign_global_var(global_var_table, f, root);
    }


    parse_tree(root->firstChild, global_var_table);
    parse_tree(root->nextSibling, global_var_table);
}





void build_asm(Node *root, List list){
    f = fopen("_anonymous.asm", "wr");
    Symbol_table *table;
    table = get_table_by_name("global_vars", list);
    
    init_asm_(table->total_size);
    parse_tree(root, table); //TODO
    end_asm();
}