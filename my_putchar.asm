;my_putchar function
section .data
    chr: dw "a",0
section .text
    global my_putchar

;Mettre argument dans rdi

my_putchar:
    push rdi
        mov rax, 1
        mov rdi, 1
        mov rsi, rsp
        mov rdx, 1
        syscall       
        ret