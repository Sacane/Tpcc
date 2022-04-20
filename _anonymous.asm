section .bss
global_var: resq 2
foo: resq 1
main: resq 0
bar: resq 1
section  .text
global  _start
extern my_putchar
extern my_getint
_start:
	mov rdi, 'P'
	call my_putchar
	mov rdi, 13
	call my_putchar
	mov rdi, 10
	call my_putchar
	mov rax, 60
	mov rdi, 0
	syscall
