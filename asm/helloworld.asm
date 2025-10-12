	global	_start

	section	.text

_start:
	mov rax, 1	;syscall 1
	mov rdi, 1	;stdout
	mov rsi, message
	mov rdx, messageend - message
	syscall

	mov rax, 60
	xor rdi, rdi
	syscall

	section .data

message:
	db "Hello, world. This is a 64 bit assembly program.", 0ah
messageend:

