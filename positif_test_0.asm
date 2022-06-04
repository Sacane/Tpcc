section .data
	 formatInt: db "%d", 10, 0
	 formatIntIn: db "%d", 0
	 fmtChar: db "%c", 0
section .bss
global_vars: resq 2
number: resq 1
section  .text
     global  _start
     extern my_putchar
     extern show_registers
     extern my_getint
     extern printf
     extern scanf
foo:
	push rbp
	mov rbp, rsp
	sub rsp, 8
	; Start assignement
	mov qword [rbp  -8], 10
	; End assignement
	; Function call start
	mov rdi, 'W'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, 'e'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, ' '
	call my_putchar
	; End function call
	; Function call start
	mov rdi, 'g'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, 'o'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, 't'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, ' '
	call my_putchar
	; End function call
	; Function call start
	mov rdi, 'p'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, 'u'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, 't'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, 'c'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, 'h'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, 'a'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, 'r'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, qword [rbp  -8]
	call my_putchar
	; End function call
endfoo:
	add rsp, 8
	mov rsp, rbp
	pop rbp
	ret
main:
	push rbp
	mov rbp, rsp
	sub rsp, 8
	; Function call start
	mov rdi, 'P'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, `\n`
	call my_putchar
	; End function call
	; Function call start
	; putint to const
	mov rsi, 110
	mov rax, 0
	mov rdi, formatInt
	sub rsp, 8
	call printf
	add rsp, 8
	; End function call
	; Start assignement
	; [START] Assign to a function call
	call bar
	mov qword [rbp  -8], rax
	; [END] Assign to a function call
	; End assignement
	; Function call start
	mov rsi, qword [rbp  -8]
	mov rax, 0
	mov rdi, formatInt
	sub rsp, 8
	call printf
	add rsp, 8
	; End function call
	; Return instr start
	mov rax, 1
	jmp endmain
	; Return instr end
endmain:
	add rsp, 8
	mov rsp, rbp
	pop rbp
	ret
bar:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	; Function call start
	mov rdi, 'b'
	call my_putchar
	; End function call
	; Function call start
	mov rdi, `\n`
	call my_putchar
	; End function call
	; Return instr start
	mov rax, 1
	jmp endbar
	; Return instr end
endbar:
	add rsp, 0
	mov rsp, rbp
	pop rbp
	ret
_start:
	call main
	mov rax, 60
	mov rdi, 0
	syscall
