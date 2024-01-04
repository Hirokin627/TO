[bits 64]
global Kernelmain
extern stack
extern nKernelmain
Kernelmain:
	mov rsp,stack
	add rsp,1024*1024
	call nKernelmain
