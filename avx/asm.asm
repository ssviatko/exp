	global doubler

	section .text

doubler:
	push rbx
	push rbp
	mov rbp, rsp
	mov rsi, rdi		;pointer into source
	mov rdi, res0		;dest is array
	mov rcx, 8
	rep movsd		;move 8 dwords
	mov rbx, res0
	vmovaps ymm0, [rbx]
	vmovaps ymm1, ymm0
	vaddps ymm2, ymm0, ymm1
	vmovups [rbx], ymm2
	mov rax, rbx		;return pointer to array
	pop rbp
	pop rbx
	ret

	section .data

	align 32

res0	dd 0
res1	dd 0
res2	dd 0
res3	dd 0
res4	dd 0
res5	dd 0
res6	dd 0
res7	dd 0

