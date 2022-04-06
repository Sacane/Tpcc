section .bss
global_var: resp 27
section  .text
global  _start
_start:
mov 12 dword [resp + 0]
mov 24 dword [resp + 4]
mov 'c' byte [resp + 20]
mov '0' byte [resp + 21]
	mov rax, 60
	mov rdi, 0
	syscall
