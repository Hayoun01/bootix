;; STAGE 2 of bootix

bits 16;
section .stage2;
global stage2;

stage2:
	mov dl, [0x7bff];
	mov [boot_drive], dl;
	call clear_screen;
	call real_to_prot;		;; switching to protected mode

clear_screen:
	mov ah, 0x0;
	mov al, 0x03;
	int 0x10;
	ret;

; linux kernel chain
krjmp:
	mov eax, cr0
	and eax, 0xfffffffe
	mov cr0, eax
	jmp 0:krinit

krinit:
	mov ax, 0x9000
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x4000
	xor ax, ax
	mov fs, ax
	mov gs, ax
	cli
	mov dl, [boot_drive] 
	jmp 0x9000:0x0200

section .modes
bits 16;

%define NULL_SEG	0
%define CODE_SEG	0x8
%define DATA_SEG	0x10
%define RCODE_SEG	0x18
%define RDATA_SEG	0x20

;; bios_regs
%define BIOS_EAX	0
%define BIOS_EBX	2
%define BIOS_ECX	4
%define BIOS_EDX	6
%define BIOS_ESI	8
%define BIOS_EDI	10
%define BIOS_INT	12

; protected mode gdt
pgdt: 
pgdt_null:
	dq 0x0;
pgdt_code:
	dw 0x0ffff			;; segment limiter
	dw 0x0				;; base address
	db 0x0;				;; base address
	db 0b10011010;
	db 0b11001111;
	db 0

pgdt_data:
	dw 0x0ffff			;; segment limiter
	dw 0x0				;; base address
	db 0x0
	db 0b10010010
	db 0b11001111
	db 0
pgdt_end:
; real mode gdt
rgdt:
rgdt_code:
	dw 0xFFFF
	dw 0
	db 0
	db 0x9e
	db 0
	db 0

rgdt_data:
	dw 0xFFFF
	dw 0
	db 0
	db 0x92
	db 0
	db 0
rgdt_end:

pgdt_desc:
	dw rgdt_end-pgdt-1
	dd pgdt

real_gdt:
    dw 0x0000, 0x0000, 0x0000, 0x0000
    dw 0xFFFF, 0x0000, 0x9A00, 0x008F 
real_gdt_ptr:
    dw real_gdt_ptr - real_gdt - 1
    dd real_gdt



rmode_stub:
	mov eax, cr0
	and eax, 0xfffffffe
	mov cr0, eax;
	jmp 0:rmode

rmode:
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x6000
	sti
	jmp rmode_call

rmode_call:
	movzx esi, byte [edi + BIOS_INT]   ; load interrupt number (0‑127)
	imul esi, 3                          ; multiply by 3
	add esi, int_table                   ; add base address
	mov [int_call], esi

	mov eax, [edi + BIOS_EAX]	;; restoring the registers
	mov ebx, [edi + BIOS_EBX]	;; restoring the registers
	mov ecx, [edi + BIOS_ECX]	;; restoring the registers
	mov edx, [edi + BIOS_EDX]	;; restoring the registers
	mov esi, [edi + BIOS_ESI]	;; restoring the registers
	mov edi, [edi + BIOS_EDI]	;; restoring the registers


	call [int_call]                 ;; indirect call

	jmp prot_end;

prot_end:
	cli
	xor ax, ax;			;; clearing ds since gdt descriptor must be in ds:gdt_desc
	mov ds, ax;
	lgdt [pgdt_desc]
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	jmp CODE_SEG:prot_ret		;; far jump to protected mode return stub


real_to_prot:
	cli;				;; clearing interrupts
	xor ax, ax;			;; clearing ds since gdt descriptor must be in ds:gdt_desc
	mov ds, ax;
	lgdt [pgdt_desc];
	mov eax, cr0;			;; setting bit 0 in cr0 register
	or eax, 1;
	mov cr0, eax;
	mov ax, DATA_SEG
	jmp CODE_SEG:load_c

int_table:
%assign i 0
%rep 128
    int i
    ret
%assign i i+1
%endrep

int_call	dw 0

bits 32;
global prot_to_real;

load_c:
	mov	ds, ax
	mov	ss, ax
	mov	es, ax
	mov	esp,bootix_stack_top;
	extern boot_main;
	call boot_main;
	jmp $;

prot_to_real:
	cli;
	mov [saved_esp], esp
	mov ax, RDATA_SEG;
	mov ds, ax;
	mov es, ax;
	mov ss, ax;
	jmp 0x18:rmode_stub

prot_ret:
	; Reload protected mode data segments
	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov ss, ax
	; Restore original stack pointer
	mov esp, [saved_esp]
	ret                          ; return to caller (in protected mode)


global kchain
kchain:
	cli
	mov ax, RDATA_SEG;
	mov ds, ax;
	mov es, ax;
	mov ss, ax;
	jmp 0x18:krjmp




section .bss
align 4
global boot_drive 
boot_drive	db 0;
saved_esp:        resd 1
bootix_stack_bottom: equ $
	resb 16384 ; 16 KB
bootix_stack_top:
