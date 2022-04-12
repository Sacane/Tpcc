;my_getint function
section .data
    chr: dw "a",0
section .text
    global my_getint

;Mettre argument dans rdi

my_getint:
    push rdi
    sub rsp, 8
    mov rax, 0
    mov rdi, 0
    mov rsi, rsp
    mov rdx, 1
    syscall
    pop rax
    ret