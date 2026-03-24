#ifndef KBOOT_H
#define KBOOT_H

#include <stdint.h>			// please lsp just stfu

#define CMDLINE_PTR	0x80000 + 0x1000
#define HEAP_END_PTR	0x8000


// https://web.archive.org/web/20200507134300/https://www.kernel.org/doc/html/latest/x86/boot.html
typedef struct linux_kernel_header {
	uint8_t  setup_sects;		// size (in sectors) of realmode setup code
	uint16_t root_flags;		// root perms [DEPRECATED]
	uint32_t syssize;		// size of the protectedmode code in units of 16-byte paras
	uint16_t ram_size;		// [Obsolete]
	uint16_t vid_mode;		
	uint16_t root_dev;		// default root device device number [DEPRECATED] (use root=)
	uint16_t boot_flag;		// 0xAA55 (magic byte)
	uint16_t jump;			// x86 jump instruction 
	uint32_t header;		// HdrS (magic string)
	uint16_t version;		// boot protocol version
	uint32_t realmode_swtch;	
	uint16_t start_sys;		// [Obsolete]
	uint16_t kernel_version;	// KERNEL version
	uint8_t  type_of_loader;	// 
					
	uint8_t  loadflags;		// (i) internal - (r) read - (w) write - (o) obselete
					// bit [0] (r) - 0 protected mode code is loaded at 0x10000
					// 		 1 protected mode code is loaded at 0x100000
					// bit [1] (i) - KASLR 1 on, 0 off
					// bit [5] (w) - 0 print early messages
					// 		 1 stfu
					// bit [6] (o) - keep segments
					// bit [7] (w) - can use heap
	uint16_t setup_move_size;	
	uint32_t code32_start;
	uint32_t ramdisk_image;
	uint32_t ramdisk_size;
	uint32_t bootsect_kludge;
	uint16_t heap_end_ptr;
	uint8_t  ext_loader_ver;
	uint8_t  ext_loader_type;
	uint32_t cmd_line_ptr;
	uint32_t initrd_addr_max;
	uint32_t kernel_alignment;
	uint8_t  relocatable_kernel;
	uint8_t  pad2;
	uint32_t cmdline_size;
	uint32_t hardware_subarch;
	uint64_t hardware_subarch_data;
	uint32_t payload_offset;
	uint32_t payload_length;
	uint64_t setup_data;
	uint64_t pref_address;		// (r) - if non zero preferred load address for the kernel
	uint32_t init_size;
	uint32_t handover_offset;	// (r) - offset from kernel base to EFI handover protocol entry point
	uint32_t kernel_info_offset;	// (r) - offset from kernel base to kernel info
} __attribute__((packed)) linux_kernel_header;


void boot(cnf_namespace *cnf, partition_table *fs);
void chainload(cnf_namespace *cnf, partition_table *fs);
extern void kchain();


#endif

