
avx: main.cc asm.o
	g++ main.cc asm.o -o avx

asm.o: asm.asm
	nasm asm.asm -f elf64 -l asm.lst -o asm.o

