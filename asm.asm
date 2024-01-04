[bits 64]
section .text
global loadgdt:
loadgdt:
	push rbp
	mov rbp,rsp
	sub rsp,10
	mov [rsp+0],di
	mov [rsp+2],rsi
	lgdt [rsp]
	mov rsp,rbp
	pop rbp
	pop rax
	mov rcx,8
	push rcx
	push rax
	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax
	o64 retf
global setr10
setr10:
	mov r10,rdi
	cli
	hlt
	ret
global setcr3
setcr3:
	mov rax,rdi
	mov cr3,rax
	ret
global getcr3
getcr3:
	mov rax,cr3
	ret
global loadidt
loadidt:
	push rbp
	mov rbp,rsp
	sub rsp,10
	mov [rsp+0],di
	mov [rsp+2],rsi
	lidt [rsp]
	mov rsp,rbp
	pop rbp
	ret
global io_out32
io_out32:
	mov dx,di
	mov eax,esi
	out dx,eax
	ret
global io_in32
io_in32:
	mov dx,di
	in eax,dx
	ret
global io_out8
io_out8:
	push dx
	push ax
	mov dx,di
	mov ax,si
	out dx,al
	pop ax
	pop dx
	ret
global io_in8
io_in8:
	mov dx,di
	in al,dx
	ret
global readmsr
readmsr:
	mov ecx,edi
	rdmsr
	ret
global writemsr
writemsr:
	mov ecx,edi
	mov eax,esi
	wtmsr
	ret
global switchcont
switchcont:
    mov [rsi + 0x40], rax
    mov [rsi + 0x48], rbx
    mov [rsi + 0x50], rcx
    mov [rsi + 0x58], rdx
    mov [rsi + 0x60], rdi
    mov [rsi + 0x68], rsi

    lea rax, [rsp + 8]
    mov [rsi + 0x70], rax  ; RSP
    mov [rsi + 0x78], rbp

    mov [rsi + 0x80], r8
    mov [rsi + 0x88], r9
    mov [rsi + 0x90], r10
    mov [rsi + 0x98], r11
    mov [rsi + 0xa0], r12
    mov [rsi + 0xa8], r13
    mov [rsi + 0xb0], r14
    mov [rsi + 0xb8], r15

    mov rax, cr3
    mov [rsi + 0x00], rax  ; CR3
    mov rax, [rsp]
    mov [rsi + 0x08], rax  ; RIP
    pushfq
    pop qword [rsi + 0x10] ; RFLAGS

    mov ax, cs
    mov [rsi + 0x20], rax
    mov bx, ss
    mov [rsi + 0x28], rbx
    mov cx, fs
    mov [rsi + 0x30], rcx
    mov dx, gs
    mov [rsi + 0x38], rdx

    fxsave [rsi + 0xc0]
    ; fall through to RestoreContext

global RestoreContext
RestoreContext:  ; void RestoreContext(void* task_context)
    ; iret 用のスタックフレーム
    cli
    push qword [rdi + 0x28] ; SS
    push qword [rdi + 0x70] ; RSP
    push qword [rdi + 0x10] ; RFLAGS
    push qword [rdi + 0x20] ; CS
    push qword [rdi + 0x08] ; RIP

    ; コンテキストの復帰
    fxrstor [rdi + 0xc0]

    mov rax, [rdi + 0x00]
    mov cr3, rax
    mov rax, [rdi + 0x30]
    mov fs, ax
    mov rax, [rdi + 0x38]
    mov gs, ax

    mov rax, [rdi + 0x40]
    mov rbx, [rdi + 0x48]
    mov rcx, [rdi + 0x50]
    mov rdx, [rdi + 0x58]
    mov rsi, [rdi + 0x68]
    mov rbp, [rdi + 0x78]
    mov r8,  [rdi + 0x80]
    mov r9,  [rdi + 0x88]
    mov r10, [rdi + 0x90]
    mov r11, [rdi + 0x98]
    mov r12, [rdi + 0xa0]
    mov r13, [rdi + 0xa8]
    mov r14, [rdi + 0xb0]
    mov r15, [rdi + 0xb8]

    mov rdi, [rdi + 0x60]

    o64 iret
global grf
grf:
	pushfq
	pop rax
	ret
global srf
srf:
	push rdi
	popfq
	ret
global io_out16
io_out16:
	mov dx,di
	mov ax,si
	out dx,ax
	ret
global io_in16
io_in16:
	mov dx,di
	in ax,dx
	ret
