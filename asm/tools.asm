
	section .text

do_exit:		;Call Linux sys_exit
	mov rax, 60
	xor rdi, rdi
	syscall

input_string:		;Get string from stdin to input buffer at RSI capable of holding up to 4096 chars
	mov rax, 0
	mov rdi, 0
	mov rdx, 4096
	syscall
	ret		;Returns length read in RAX

print_string:		;Print string pointed to by RSI of length RDX
	call save_register_file
print_string_nosave:
	mov rax, 1
	mov rdi, 1
	syscall
	call load_register_file
	ret

print_cr:		;Print a carriage return (linefeed)
	mov al, 0ah
	jmp print_char

print_space:
	mov al, ' '
	jmp print_char

print_char:		;Print the character in AL
	mov [charout], al
	mov rax, 1
	mov rdi, 1
	mov rsi, charout
	mov rdx, 1	;just print 1 character
	syscall
	ret

print_embedded_cstring:
	push rsi
	mov rsi, [rsp+8]
	call print_cstring
	mov [rsp+8], rsi
	pop rsi
	ret

print_cstring:
	call save_register_file
	cld
.0:
	lodsb		;grab character at RSI
	or al, al
	jz .1
	push rsi
	call print_char
	pop rsi
	jmp .0
.1:
	push rsi
	call load_register_file
	pop rsi
	ret

print_thex:		;print ten-byte extended real as hex
	mov rax, rsi
	add rax, 9  ;point at the end
	mov rsi, rax
	std			;set direction flag for decrement
	mov rcx, 10
.0	lodsb
	push rsi
	push rcx
	push rax
	call print_byte
	pop rax
	pop rcx
	pop rsi
	loop .0
	ret

print_byte:		;Print hex byte in AL
	push ax		;save low nibble
	shr al, 4
	call print_nibble
	pop ax
	call print_nibble
	ret

print_nibble:		;Print the low nibble of AL as an ASCII character
	and al, 0fh
	cmp al, 0ah
	jge .0
	or al, 30h
	jmp print_char
.0	sub al, 10
	add al, 'A'
	jmp print_char

print_dword:		;print the contents of EAX
	push rax
	shr rax, 24
	call print_byte
	pop rax
	push rax
	shr rax, 16
	call print_byte
	pop rax
	push rax
	shr rax, 8
	call print_byte
	pop rax
	jmp print_byte

print_qword:		;print the contents of RAX
	push rax
	shr rax, 32
	call print_dword
	pop rax
	jmp print_dword

save_register_file:
	mov [rsava], rax
	mov [rsavb], rbx
	mov [rsavc], rcx
	mov [rsavd], rdx
	mov [rsavsi], rsi
	mov [rsavdi], rdi
	mov [rsavbp], rbp
	mov [rsavsp], rsp
	mov [rsav8], r8
	mov [rsav9], r9
	mov [rsav10], r10
	mov [rsav11], r11
	mov [rsav12], r12
	mov [rsav13], r13
	mov [rsav14], r14
	mov [rsav15], r15
	ret

load_register_file:
	mov rax, [rsava]
	mov rbx, [rsavb]
	mov rcx, [rsavc]
	mov rdx, [rsavd]
	mov rsi, [rsavsi]
	mov rdi, [rsavdi]
	mov rbp, [rsavbp]
; note: leave rsp alone
	mov r8, [rsav8]
	mov r9, [rsav9]
	mov r10, [rsav10]
	mov r11, [rsav11]
	mov r12, [rsav12]
	mov r13, [rsav13]
	mov r14, [rsav14]
	mov r15, [rsav15]
	ret

print_regs:
	call save_register_file
; conventional registers

	mov al, 'A'
	call print_char
	mov al, 'X'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsava]
	call print_qword
	mov al, ' '
	call print_char

	mov al, 'B'
	call print_char
	mov al, 'X'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsavb]
	call print_qword
	mov al, ' '
	call print_char

	mov al, 'C'
	call print_char
	mov al, 'X'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsavc]
	call print_qword
	mov al, ' '
	call print_char

	mov al, 'D'
	call print_char
	mov al, 'X'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsavd]
	call print_qword
	mov al, ' '
	call print_char

	mov al, 'S'
	call print_char
	mov al, 'I'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsavsi]
	call print_qword
	mov al, ' '
	call print_char

	mov al, 'D'
	call print_char
	mov al, 'I'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsavdi]
	call print_qword
	mov al, ' '
	call print_char

	mov al, 'B'
	call print_char
	mov al, 'P'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsavbp]
	call print_qword
	mov al, ' '
	call print_char

	mov al, 'S'
	call print_char
	mov al, 'P'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsavsp]
	call print_qword
	mov al, ' '
	call print_char

	call print_cr

; print 8-15

	mov al, ' '
	call print_char
	mov al, '8'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsav8]
	call print_qword
	mov al, ' '
	call print_char

	mov al, ' '
	call print_char
	mov al, '9'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsav9]
	call print_qword
	mov al, ' '
	call print_char

	mov al, '1'
	call print_char
	mov al, '0'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsav10]
	call print_qword
	mov al, ' '
	call print_char

	mov al, '1'
	call print_char
	mov al, '1'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsav11]
	call print_qword
	mov al, ' '
	call print_char

	mov al, '1'
	call print_char
	mov al, '2'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsav12]
	call print_qword
	mov al, ' '
	call print_char

	mov al, '1'
	call print_char
	mov al, '3'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsav13]
	call print_qword
	mov al, ' '
	call print_char

	mov al, '1'
	call print_char
	mov al, '4'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsav14]
	call print_qword
	mov al, ' '
	call print_char

	mov al, '1'
	call print_char
	mov al, '5'
	call print_char
	mov al, ':'
	call print_char
	mov rax, [rsav15]
	call print_qword
	mov al, ' '
	call print_char
	call print_cr
	call load_register_file
	ret

print_uint64_ascii:		;Formats unsigned 64 bit value in RAX as decimal ASCII string
	mov rdi, 0		;ASCII digit counter
.0:
	mov rdx, 0		;pad top 64 bits of RDX:RAX dividend to 0
	mov rbx, 10		;Divide by 10
	div rbx			;Remainder is our ascii character, result gets recycled
	mov [outb+rdi],dl	;Save our unconverted value
	inc rdi
	cmp rax, 0		;Are we at "0 remainder something" yet?
	jg .0			;If not, cycle again and get the next digit

	mov [asciilen], rdi	;stash our string length
	mov rcx, rdi		;Make it our counter
	dec rdi			;Position it over our last character
	mov rsi, rdi		;Make it our source offset
	add rsi, outb		;Make our offset an address pointer
	mov rdi, ascii		;Set up output pointer
.1:
	std			;Reverse our number string and make it printable
	lodsb			;Load first byte and decrement RSI
	or al, 30h		;Make it an ASCII character
	cld
	stosb			;Stash it in the ASCII buffer
	loop .1			;Recycle until finished
	ret

print_uint64:			;Prints unsigned 64 bit value in RAX as decimal
	call save_register_file
print_uint64_nosave:
	call print_uint64_ascii
	mov rdx, [asciilen]
	mov rsi, ascii
	call print_string
	jmp load_register_file

print_int64:			;Prints signed 64 bit value in RAX a decimal
	call save_register_file
	bt rax, 63		;Highest bit set?
	jc .0
	call print_uint64_nosave	;No, so process it as a positive integer
	ret
.0:
	neg rax			;Two's complement
	push rax
	mov al, '-'
	call print_char		;Print a negative sign
	pop rax
	call print_uint64_nosave
	ret

print_double:				;Print double float in RAX with RDX decimal places of precision (up to 18 digits)
	call save_register_file
print_double_nosave:
	mov [pd_double], rax		;Save our double
	mov [pd_dp], rdx
	mov r8, 07ff0000000000001h	;Set up constants for NaN and Infinity compare
	mov r9, 07fffffffffffffffh
	mov r10, 0fff0000000000001h
	mov r11, 07ff0000000000000h	;positive infinity
	mov r12, 0fff0000000000000h	;negative infinity
	cmp rax, r11
	je print_posinf
	cmp rax, r12
	je print_neginf
	cmp rax, r10
	jae print_nan			;anything bigger than fff0...1 is a NaN
	cmp rax, r8
	jb .3				;Not a NaN
	cmp rax, r9
	jbe print_nan
.3:
	test rax, rax			;we know at this point it's not a NaN or Inf, so check sign bit
	jns .4				;positive, so proceed as normal
	mov r8, 07fffffffffffffffh	;Clear sign bit - similar to an abs(rax)
	and rax, r8
	mov [pd_double], rax		;Update it
	mov al, '-'
	call print_char			;Print a negative sign
.4:
	movsd xmm0, [pd_double]		;load double
	cvttsd2si rbx, xmm0		;Get truncated integer portion
	mov [pd_int], rbx		;Save it
	cvtsi2sd xmm1, rbx		;And immediately float it
	subsd xmm0, xmm1		;Subtract it from our original double to get fractional portion
	mov rax, 1000000000000000000	;One quintillion (1 followed by 18 zeros)
	cvtsi2sd xmm1, rax
	mulsd xmm0, xmm1		;Fractional part * 1 quintillion
	cvtsd2si rbx, xmm0		;Get frac
	mov [pd_frac], rbx		;Save it

	mov rax, [pd_int]
	call print_uint64
	mov rdx, [pd_dp]
	test rdx, rdx			;If we're not printing decimal places, fall through to exit
	jz .1
	mov al, '.'
	call print_char
	mov rax, [pd_frac]
	call print_uint64_ascii
	mov rdx, [pd_dp]
	cmp rdx, [asciilen]
	jle .2
	mov rdx, [asciilen]
.2:
	mov rsi, ascii
	call print_string
.1:
	call load_register_file
	ret

print_posinf:
	mov rdx, [Inflen]
	mov rsi, PosInfstr
	jmp print_string_nosave

print_neginf:
	mov rdx, [Inflen]
	mov rsi, NegInfstr
	jmp print_string_nosave

print_nan:
	mov rdx, [NaNlen]
	mov rsi, NaNstr
	jmp print_string_nosave

print_single:				;Print the single precision float in EAX with RDX decimal precision
	call save_register_file
	cmp eax, 07f800000h		;cvtss2sd instruction doesn't propagate NaNism or Infinity, so check it here
	je print_posinf
	cmp eax, 0ff800000h
	je print_neginf
	cmp eax, 0ff800001h
	jae print_nan
	cmp eax, 07f800001h
	jb .1				;Not a NaN
	cmp eax, 07fffffffh
	jbe print_nan
.1:
	push rax
	cvtss2sd xmm0, [rsp]		;Convert it to a double and call print_double
	movsd [rsp], xmm0
	pop rax
	jmp print_double_nosave

random64:
	call save_register_file
	mov rdi, randbuff
	mov rsi, 8
	mov rdx, 0
	mov rax, 318
	syscall
	call load_register_file
	mov rax, [randbuff]
	ret

random_mod:
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi

	push rax			;save our modulus
	call random64
	pop rbx
	xor rdx, rdx			;set up high qword for division
	div rbx
	mov rax, rdx			;stick modulus in rax

	pop rdi
	pop rsi
	pop rdx
	pop rcx
	pop rbx
	ret

	section .data

	align 32

ssesavl:
	dq 0
ssesavh:
	dq 0
pd_double:
	dq 0
pd_dp:
	dq 0
pd_int:
	dq 0			;Integer portion
pd_frac:
	dq 0			;Fractional portion
NaNstr:
	db "NaN"
NaNlen:
	dq 3
PosInfstr:
	db "+Infinity"
NegInfstr:
	db "-Infinity"
Inflen:
	dq 9

; save locations for print_regs

rsava:
	dq 0
rsavb:
	dq 0
rsavc:
	dq 0
rsavd:
	dq 0
rsavsi:
	dq 0
rsavdi:
	dq 0
rsavbp:
	dq 0
rsavsp:
	dq 0
rsav8:
	dq 0
rsav9:
	dq 0
rsav10:
	dq 0
rsav11:
	dq 0
rsav12:
	dq 0
rsav13:
	dq 0
rsav14:
	dq 0
rsav15:
	dq 0

	section .bss

randbuff:
	resb 8
charout:
	resb 1
outb:
	resb 40			;Work buffer for display routines
ascii:
	resb 40			;ASCII output string
asciilen:
	resb 8			;Length of ASCII string
