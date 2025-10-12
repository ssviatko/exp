%include "tools.asm"

	global	_start

	section	.text

_start:
	mov rsi, prompt
	mov rdx, prompt.end - prompt
	call print_string

	mov rsi, inb
	call input_string
	cmp rax, 0
	je done		;do nothing if we didn't read anything

	push rax	;save message length for later
	mov rsi, num_read
	mov rdx, num_read.end - num_read
	call print_string
	pop rax
	push rax
	call print_dword
	call print_cr

	cld
	mov rsi, inb
	mov rdi, inb

nextchar:
	lodsb
	cmp al, 'a'
	jb notalpha
	cmp al, 'z'
	jg notalpha
	sub al, ('z' - 'Z')	;convert to upper case

notalpha:
	stosb
	cmp rdi, inb.end
	jb nextchar

	pop rdx		;retrieve number of bytes actually read
	mov rax, 1
	mov rdi, 1
	mov rsi, inb
	syscall		;echo the message
	call print_cr

done:
	jmp do_exit

	section .data

prompt:
	db "Please enter a text string:", 0ah, ">"
.end:

num_read:
	db "Number of characters read: "
.end:

	section .bss

inb:
	resb 4096
.end:

