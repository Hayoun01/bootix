#include "../inc/bootix.h"


static void bios_interrupt(bios_regs* regs) {	
	__asm__ volatile ("mov %0, %%edi" : : "r"(regs));
	prot_to_real();	
}

void	read_sector_lba(void *buffer, uint16_t sectors, uint32_t lba){
	dap	cdap = {
		.size = 0x10,
		.reserv = 0,
		.count = sectors,
		.offset = (((uint32_t) buffer) & 0xffff),
		.segment = ((((uint32_t) buffer) & 0xffff0000) >> (8*2)),
		.lba_low = lba,
		.lba_high = 0x0
	};

	bios_regs	ctx = {};
	memset(&ctx, 0, sizeof(bios_regs));
	ctx.edx = boot_drive;
	ctx.esi = (uint16_t) &cdap;
	ctx.eax = 0x4200;
	ctx.intno = 0x13;
	bios_interrupt(&ctx);
}
