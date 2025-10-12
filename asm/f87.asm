%include "tools.asm"

	global	_start

	section	.text

_start:
	mov rsi, splash
	mov rdx, splash.end - splash
	call print_string
	call print_cr

	finit

; convert gamma into decimal
	fld tword [gamma]
	mov rax, 100000000	;8 decimal places
	push rax
	fild qword [rsp]
	fmul
	fbstp tword [gamma_b]
	pop rax

; convert randt into decimal
	fld tword [randt]
	mov rax, 100000000	;8 decimal places
	push rax
	fild qword [rsp]
	fmul
	fbstp tword [randt_b]
	pop rax

; preform operation on gamma & randt
	fld tword [gamma]
	fld tword [randt]
	fchs
	fdiv
	fstp tword [result]

; convert result at top of stack into decimal
	fld tword [result]
	mov rax, 100000000	;8 decimal places
	push rax
	fild qword [rsp]
	fmul
	fbstp tword [result_b]
	pop rax

; print results
	mov rsi, gammais
	mov rdx, gammais.end - gammais
	call print_string
	mov rsi, gamma
	call print_thex
	mov rsi, bcdis
	mov rdx, bcdis.end - bcdis
	call print_string
	mov rsi, gamma_b
	call print_thex
	call print_cr

	mov rsi, randtis
	mov rdx, randtis.end - randtis
	call print_string
	mov rsi, randt
	call print_thex
	mov rsi, bcdis
	mov rdx, bcdis.end - bcdis
	call print_string
	mov rsi, randt_b
	call print_thex
	call print_cr

	mov rsi, resultis
	mov rdx, resultis.end - resultis
	call print_string
	mov rsi, result
	call print_thex
	mov rsi, bcdis
	mov rdx, bcdis.end - bcdis
	call print_string
	mov rsi, result_b
	call print_thex
	call print_cr

	jmp do_exit


	section .data

splash:
	db "Fun with x87 instructions!"
.end:
gammais:
	db "gamma (Euler/Mascheroni constant) is (hex)  : "
.end:
randtis:
	db "random floating point value is (hex)        : "
.end:
resultis:
	db "result of floating point operation is (hex) : "
.end:
bcdis:
	db " BCD (8 decimal places): "
.end:

gamma:
	dt 0.577215
randt:
	dt 1.875430

	section .bss

gamma_b:
	resb 11
randt_b:
	resb 11
result:
	resb 10
result_b:
	resb 11


