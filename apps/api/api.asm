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
