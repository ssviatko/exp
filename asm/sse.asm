%include "tools.asm"

	global	_start

	section	.text

_start:
	mov rsi, prompt
	mov rdx, prompt.end - prompt
	call print_string

	movsd xmm0, [pi]		;load PI into xmm0
	mov rax, 2
	cvtsi2sd xmm1, rax		;load 2.0 into xmm1
	mulsd xmm0, xmm1		;get PI * 2
	movlpd [resultlow], xmm0
	movaps [resultlow], xmm0	;Save entire 128 bit contents of xmm0

	cvtsd2si rax, xmm0		;Get integer portion of result
	mov [resultint], rax		;save it
	cvtsi2sd xmm1, rax		;immediately float it again
	subsd xmm0, xmm1		;subtract it to get the fractional portion

	mov rax, 1000000000000000	;Multiply fractional part by 1 quadrillion and save
	cvtsi2sd xmm2, rax
	mulsd xmm0, xmm2
	cvtsd2si rax, xmm0
	mov [resultfrac], rax

	mov rax, [resulthigh]		;Print 128 bit xmm0 (PI * 2)
	call print_qword
	mov rax, [resultlow]
	call print_qword
	call print_cr

	mov rax, [resultint]		;print int and fractional parts as hex strings
	call print_qword
	mov rsi, dash
	mov rdx, dash.end - dash
	call print_string
	mov rax, [resultfrac]
	call print_qword
	call print_cr

	mov rax, [resultint]		;print int and fractional parts as ascii strings
	call print_uint64
	mov rsi, dash
	mov rdx, dash.end - dash
	call print_string
	mov rax, [resultfrac]
	call print_uint64
	call print_cr

	mov rax, 1234567890
	call print_uint64
	call print_cr
	mov rax, 327689
	call print_uint64
	call print_cr
	mov rax, 9876321
	call print_int64
	call print_cr
	mov rax, -8787878
	call print_int64
	call print_cr
	mov rax, 0ffffffffffffffffh
	call print_int64
	call print_cr
	mov rax, -9876321
	call print_int64
	call print_cr

	mov rax, [pi]
	mov rdx, 4
	call print_double
	call print_cr
	mov rax, [resultlow]
	mov rdx, 87
	call print_double
	call print_cr

	movaps xmm0, [resultlow]
	mov rax, 8723
	cvtsi2sd xmm1, rax
	mulsd xmm0, xmm1
	push rax
	movlpd [rsp], xmm0
	pop rax
	mov rdx, 15
	call print_double
	call print_cr

	mov rax, 0			;Generate a NaN
	cvtsi2sd xmm0, rax
	cvtsi2sd xmm1, rax
	divsd xmm0, xmm1
	push rax
	movlpd [rsp], xmm0
	pop rax
	mov rdx, 0
	call print_double
	call print_cr

	mov rax, 1			;Generate an Infinity
	cvtsi2sd xmm0, rax
	mov rax, 0
	cvtsi2sd xmm1, rax
	divsd xmm0, xmm1
	push rax
	movlpd [rsp], xmm0
	pop rax
	mov rdx, 0
	call print_double
	call print_cr

	mov rax, 3			;Generate a negative number
	cvtsi2sd xmm0, rax
	mov rax, 7
	cvtsi2sd xmm1, rax
	subsd xmm0, xmm1
	push rax
	movlpd [rsp], xmm0
	pop rax
	mov rdx, 3
	call print_double
	call print_cr

	mov rax, -3797
	cvtsi2sd xmm0, rax
	mov rax, 95
	cvtsi2sd xmm1, rax
	divsd xmm0, xmm1
	push rax
	movlpd [rsp], xmm0
	pop rax
	mov rdx, 6
	call print_double
	call print_cr

	mov rsi, singlesyay
	mov rdx, singlesyay.end - singlesyay
	call print_string

	mov eax, 83
	cvtsi2ss xmm0, eax
	mov eax, 57
	cvtsi2ss xmm1, eax
	subss xmm0, xmm1
	push rax
	movss [rsp], xmm0
	pop rax
	mov rdx, 6
	call print_single
	call print_cr

	mov eax, 0			;Generate a NaN
	cvtsi2ss xmm0, eax
	cvtsi2ss xmm1, eax
	divss xmm0, xmm1
	push rax
	movss [rsp], xmm0
	pop rax
	mov rdx, 0
	call print_single
	call print_cr

	mov eax, 1			;Generate an Infinity
	cvtsi2ss xmm0, eax
	mov eax, 0
	cvtsi2ss xmm1, eax
	divss xmm0, xmm1
	push rax
	movss [rsp], xmm0
	pop rax
	mov rdx, 0
	call print_single
	call print_cr

	mov eax, 83
	cvtsi2ss xmm0, eax
	mov eax, -57
	cvtsi2ss xmm1, eax
	divss xmm0, xmm1
	push rax
	movss [rsp], xmm0
	pop rax
	mov rdx, 6
	call print_single
	call print_cr

	mov eax, 7
	cvtsi2ss xmm0, eax
	mov eax, 3
	cvtsi2ss xmm1, eax
	divss xmm0, xmm1
	push rax
	movss [rsp], xmm0
	pop rax
	mov rdx, 6
	call print_single
	call print_cr

done:
	jmp do_exit

	section .data

prompt:
	db "Fun with SSE instructions!", 0ah
.end:
dash:
	db " - "
.end:
singlesyay:
	db "And now.. single precision values!", 0ah
.end:

pi:
	dq 3.1415926535

	align 16

resultlow:
	dq 0
resulthigh:
	dq 0
resultint:
	dq 0
resultfrac:
	dq 0

