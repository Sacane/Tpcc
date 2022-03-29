global  _start
section  .text
_start:
	push 1
	push 2
	pop r14
	pop r12
	sub r12, r14
	push r12
	mov rax, 60
	mov rdi, 0
	syscall
