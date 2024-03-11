[bits 64]
global cons_putc
cons_putc:
	mov rax,0
	int 0x40
	ret
global cons_puts
cons_puts:
	mov rax,1
	int 0x40
	ret
global malloc
malloc:
	mov rax,2
	int 0x40
	ret
global asmgetwin
asmgetwin
	mov rax,3
	int 0x40
	ret
global asm_fopen
asm_fopen:
	mov rax,5
	int 0x40
	ret
global closef
closef:
	mov rax,6
	int 0x40
	ret
global free
free:
	mov rax,7
	int 0x40
	ret
