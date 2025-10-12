%include "tools.asm"

	global	_start

	section	.text

_start:
	mov rsi, prompt
	mov rdx, prompt.end - prompt
	call print_string

	mov rcx, 100
cycle:
	push rcx
	mov rcx, 1000000000
	movlpd xmm0, [var_x]		;Load our variables
	movlpd xmm1, [var_p]
	movlpd xmm2, [var_s]
	movlpd xmm4, [const_m1]		;Load our constants
	movlpd xmm5, [const_2]

.0:
	movlpd xmm3, [const_1]		;xmm3 gets destroyed so load it here after every loop
	divsd xmm3, xmm0		;p += (1/x) * s
	mulsd xmm3, xmm2
	addsd xmm1, xmm3
	mulsd xmm2, xmm4		;s *= -1
	addsd xmm0, xmm5		;x += 2

	loop .0

	movlpd [var_x], xmm0
	movlpd [var_p], xmm1
	movlpd [var_s], xmm2

	movlpd xmm6, [const_4]
	mulsd xmm6, xmm1
	movlpd [var_p4], xmm6

	mov rsi, xmsg
	mov rdx, xmsg.end - xmsg
	call print_string
	mov rax, [var_x]
	mov rdx, 18
	call print_double
	mov rsi, pmsg
	mov rdx, pmsg.end - pmsg
	call print_string
	mov rax, [var_p4]
	mov rdx, 18
	call print_double
	call print_cr

	pop rcx
	dec rcx
	jnz cycle			;too far for loop instruction

done:
	jmp do_exit

	section .data

prompt:
	db "PI generator in assembly language!", 0ah
.end:
xmsg:
	db "X = "
.end:
pmsg:
	db " P*4 = "
.end:

const_1:
	dq 1.0
const_m1:
	dq -1.0
const_2:
	dq 2.0
const_4:
	dq 4.0
var_x:
	dq 3.0
var_s:
	dq -1.0
var_p:
	dq 1.0
var_p4:
	dq 0.0

