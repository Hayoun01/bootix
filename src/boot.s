
; ---------------  defs --------------- ;
%define NULL		0
%define convmem		0x8000
%define BUFFER		0x6000
%define volid		0x9000		;; where bootable partition volume id is stored
%define volid_end	0x9200		;; volume id end addr
%define root_dir	0x9200		;; root dir cluster 
%define root_dir_end	0x9400		;; root dir cluster 

%define convmem_end	0x0FFFF
%define ptable		0x61BE
%define stage2_size	4096		;; (bytes)

;; ---------- Some FAT defs ---------- ;;
;;; volume id [offsets]
%define BPB_BytsPerSec	0xb		; always 512	(16 bits)
%define BPB_SecPerClus	0xd		; [2**n]	(8  bits)
%define BPB_RsvdSecCnt	0xe		; usually 0x20	(16 bits)
%define BPB_NumFATs	0x10		; always 2	(8  bits)
%define BPB_FATSz32	0x24		; deps		(32 bits)
%define BPB_RootClus	0x2c		; 0x2		(32 bits)
%define BPB_Sig		0x1fe		; 0xaa55	(16 bits)


;; dir entry 
%define DIR_Filename	0x0
%define DIR_Attr	0xb
%define DIR_NTReserved	0xc
%define DIR_CreatTenths	0xd
%define DIR_CreatTime	0xe
%define DIR_CreatDate	0x10
%define DIR_AccessDate	0x12
%define DIR_ClusterHi	0x14
%define DIR_WriteTime	0x16
%define DIR_WriteDate	0x18
%define DIR_ClusterLo	0x1a
%define DIR_FileSize	0x1c
;; ---------- Some FAT defs ---------- ;;
; ---------------  defs --------------- ;

bits 16;
org 0x6000
section .boot
global .boot

; [params]
;; None 
; [desc]
;; migrating step1 to 0x0000:0x6000 then jumping
; [ret]
;; nothing
start:
	xor ax, ax
	mov ds, ax
	mov es, ax
	cli
	mov sp, 0x7c00			;; setting up stack and segments
	sti
    
	mov si, sp;
	mov di, 0x6000;
	mov cx, 512;
	cld				;; memcpy code
	rep	
	movsw
	mov ax, (boot)
	jmp  ax;

migrate:
; will never execute



; TODO : parse 0xb fat32 with CHS addressing, for now we only handle LBA

; [params]
;;
; [desc]
;; 
; [ret]
;;
boot: 
	mov [boot_drive], dl		;; preserving the bootdrive for further reads
	mov si, ptable;
	xor ax, ax;
	call find_boot;			;; checking if there is a boot partition
	cmp ax, 0;
	jnz boot_err;			;; alr there's none, exiting...
	push si;
	mov si, boot_succ;		;; booting
	call prstr;
	pop si;
	call load_step2;

; [params]
;; si : address of the partition table
; [desc]
;; 
; [ret]
;;
load_step2:
	call load_fat_volid;
	mov ax, [volid + BPB_Sig];
	cmp ax, 0xaa55;
	jne unvalid_fatp;
	call calc_fat32_vals		;; computing some fat values
	mov bx, root_dir;
	call find_stage_2		;; literally search for stage2 in the root dir entries
	cmp ax, 0;
	jne stage2_not_found;
	mov si, bx;
	call read_stage_2;
	ret ;

; [params]
;; si : dir entry address of stage 2
; [desc]
;; loads stage2 from disk in 0x8000
; [ret]
;; returns nothing just jumps to the code
read_stage_2:
	mov ax, [si + DIR_ClusterHi];
	shl eax, 16;
	mov ax, [si + DIR_ClusterLo]	;; we got stage 2 sector
	call cluster2lba;		;; now we read it in 0x8000

	mov [dap.lba_low], ax
	shr eax, 16;
	mov [dap.lba_low + 2], ax


	mov word[dap.count], 15		;; reading 10 sectors
	mov word[dap.offset], 0x7c00	;; stage 2 will be loaded in 0x0000:0x7c00
	mov word[dap.segment], 0x0

	call load_sector_lba;
	mov dl, [boot_drive];		;; saving boot drive to stage 2. one byte after
	mov [0x7bff], dl;
	jmp 0x7c00;
	ret;


; [params]
;; bx: ptr to dir entries
; [desc]
;; searches for STAGE2 in root directory entries
; [ret]
;; ax : cluster number
find_stage_2:
	cmp bx, root_dir_end;
	jge err;
	mov si, bx;
	mov di, stage2_filename;
	call fat32_strcmp;
	cmp ax, 0x0;
	je .found
	add bx, 32;			;; next dir entry
	jmp find_stage_2;
.found :
	ret;


; [params]
;; VOID
; [desc]
;; calculates some basic fat32 LBA offsets
;; expects 0x9000 to have bootable fat32 Volume id, call load_fat_volid before 
; [ret]
;; nothing
calc_fat32_vals:
	; DataStart = cluster_lba_start + BPB_RsvdSecCnt + (BPB_NumFATs * BPB_FATSz32)
	; RootDirSector = DataStart + (BPB_RootClus - 2) * BPB_SecPerClus
	xor eax, eax;
	mov al, [volid + BPB_NumFATs];	;; number of fats
	mov ebx, [volid + BPB_FATSz32];	;; sectors per fat
	mul ebx;			;; eax = (number of fats * sectors per fat)

	xor ecx, ecx;
	mov bx, [cluster_lba_start];
	mov cx, [volid + BPB_RsvdSecCnt];

	add cx, bx;			;; ecx 
	add ecx, eax;			;; ecx = cluster_begin_lba
	mov [data_cluster_lba], ecx	;;

	;; ecx = cluster_begin_lba, now calculating RootDirSector
	mov eax, [volid + BPB_RootClus]
	call cluster2lba;		;; will return 


	;; reading the directory entry
	mov word[dap.segment], 0x0;	;; directory entry will be just after the volume ID
	mov word[dap.offset], 0x9200;
	mov word[dap.count], 0x1;	;; reading just one sector
	mov [dap.lba_low], ax;
	call load_sector_lba;

	ret;

; [params]
;; eax : cluster number, also will return the lba there
; [desc]
;; expects Volume id to be in (0x9000)
; [ret]
;; NONE
cluster2lba:
	sub eax, 2;
	xor ebx, ebx;
	mov bl, [volid + BPB_SecPerClus], 
	mul ebx;
	
	mov ebx , [data_cluster_lba] ;
	add eax, ebx;
	ret

; [params]
;; si : address of the partition table
; [desc]
;; loads fat volumeID using lba from partition table
; [ret]
;; returns nothing
load_fat_volid:
	mov ax, [si + 8]		;; copying the lba 2bytes at a time
	mov [dap.lba_low], ax
	mov [cluster_lba_start], ax;
	mov ax, [si + 10]
	mov [dap.lba_low + 2], ax

	; saving lba

	mov word[dap.count], 1		;; reading one sector
	mov word[dap.offset], 0x9000		;; reading in 0x0000:0x9000
	mov word[dap.segment], 0x0

load_sector_lba :
	mov dl, [boot_drive]		;; drive mode
	mov si, dap			;; setting dap 
	mov ah, 0x42
	int 0x13
	ret;

; [params]
;; takes si as a pointer to a partition table entry
; [ret]
;; ax : ret val [true/false]
; [desc]
;; checks if the current partition table is bootable using the [Boot indicator] byte
is_bootable:
	mov al, 1;			failure from start
	mov bl, byte[si];		extracting the bootable value from here
	cmp bl, 0x80;			checking if it's bootable 
	jne done;
	mov bl, byte[si + 0x4];		checking system id to make sure it's a fat32 partition
	sub bl, 0xb;			must be either 0x0 or 0x1 
	cmp bl, 0x1;
	jbe valid;			greater or overflow
	ret;

; [params]
;; si: ptable addr
; [desc]
;; iterates over the partition table and check if the partition is bootable
; [ret]
;; doesn't return anything but increments if the [si]
find_boot: 
	;; out-of-bound read check
	cmp si, ptable_end;
	jge err			;; we did not find any bootable device
	;; out-of-bound read check
	call is_bootable;
	cmp al, 0x0;
	jz done			;; we found a bootable partition
	add si, 16;
	jmp find_boot;

boot_err:
	mov si, boot_err_msg;
	call prstr;
	jmp $;
; si : filename from dir entry
; di : str 2
fat32_strcmp:
.loop:
	lodsb			;; increasing si
	cmp al, 0x20		;; in case of 0x20 we reached the end of filename
	je valid
	scasb			;; comparing si with di
	jne err;		;; substitue for not eq
	test al, al		;; check for null
	jne .loop
valid:
	xor ax, ax
	ret



; si : src
; di : dst
; cx : size
memcpy: 
	cld			;; clears the direction flag DF=0 so insuring forward auto-increment
				;; of SI/DI after each word move
	rep			;; Loops until CX=0.
	movsw			;; Moves 2 bytes from [DS:SI] to [ES:DI], then increments SI/DI by 2 each iteration.
	ret;

; [params]
;; si : string to print 
; [desc]
;;  prints a string using bios interrupts
; [ret]
;; VOID
prstr:
	lodsb;
	or al, al;
	jz done;
	mov ah, 0xe;		interrupt
	mov bh, 0x0;
	int 0x10;
	jmp prstr;

stage2_not_found:
	mov si, stage2_nf;
	call prstr;
	hlt;

unvalid_fatp:
	mov si, boot_unvalid_fat;
	call prstr;

hlt:
	hlt;

err: 
	mov ax, 0xff;
	ret

; simple return hook for loops
done:
	ret

; section .bss
ptable_end		equ 0x7c00+0x1be + (16 * 4)
cluster_lba_start	dd 0
data_cluster_lba	dd 0
root_dir_cluster_lba	dd 0
stage2_filename		db "STAGE2"

dap:
	.size     db 0x10
	.reserved db 0
	.count    dw 1
	.offset   dw 0x8000
	.segment  dw 0x0000
	.lba_low  dd 1
	.lba_high dd 0



;; error messages
boot_err_msg		db 'Boot not found.', 0x0a, 0;
boot_succ		db 'Boot part found', 0x0a, 0;
boot_unvalid_fat	db 'Unvalid Boot part', 0xa, 0;
stage2_nf		db 'S2 not found', 0xa, 0;

boot_drive   		db 0
; sig
times  510 - ($-$$) db 0
dw 0xaa55
