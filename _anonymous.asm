section .bss
global_vars: resq 2
main: resq 0
section  .text
global  _start
extern my_putchar
extern show_registers
extern my_getint
_start:
	push 0
	push 8
	pop rcx
	pop rax
	sub rax, rcx
	push rax
	mov r12, rax
	push 4
	pop rcx
	pop rax
	add rax, rcx
	push rax
	mov r12, rax
	push 192
	pop rcx
	pop rax
	add rax, rcx
	push rax
	mov r12, rax
	mov qword [global_vars + 0], rax
	mov rax, 0
	mov rbx, 0
	push 3
	push 0
	push 2
	pop rbx
	pop rdx
	pop rax
	idiv rbx
	push rax
	mov r12, rax
	mov qword [global_vars + 8], rax
	mov rax, 0
	mov rbx, 0
	mov r14, qword [global_vars + 0]
	mov r15, 2
	cmp r14, r15
	jg labelId_1
	mov r14, 0
	mov r15, 0
	jmp labelCode_1
labelId_1:
	mov rdi, 'A'
	call my_putchar
	mov rdi, 13
	call my_putchar
	mov rdi, 10
	call my_putchar
	add rbx, qword [global_vars + 8]
	jg labelId_2
	jmp labelCode_2
labelId_2:
	mov rdi, 'B'
	call my_putchar
	mov rdi, 13
	call my_putchar
	mov rdi, 10
	call my_putchar
	jmp labelCode_2
labelCode_2:
	mov rdi, 'C'
	call my_putchar
	mov rdi, 13
	call my_putchar
	mov rdi, 10
	call my_putchar
	jmp labelCode_1
labelCode_1:
	mov rdi, 'D'
	call my_putchar
	mov rdi, 13
	call my_putchar
	mov rdi, 10
	call my_putchar
	mov rax, 60
	mov rdi, 0
	syscall
