%include "tools.asm"

	global _start

	section .text

_start:
	mov rsi, msg1
	call print_cstring
	mov rsi, msg2
	call print_cstring
	mov rsi, msgc
	call print_cstring
	call print_embedded_cstring
	db "First embedded C string.", 0x0a, 0x00
	call print_embedded_cstring
	db "Second embedded C string for your viewing pleasure.", 0x0a, 0x00
	mov rcx, 10
.0:
	call print_embedded_cstring
	db "Integer is: ", 0x00
	push rcx
	mov rax, rcx
	call print_uint64
	call print_cr
	pop rcx
	loop .0
	call do_exit

msgc:
	db "random data interspersed within the code segment", 0x0a, 0x00

	section .data

msg1	db "This is the first message to print.", 0x0a, 0x00
msg2	db "Now here is a second message to print.", 0x0a, 0x00

