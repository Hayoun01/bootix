#include "../inc/bootix.h"


static void bios_interrupt(bios_regs* regs) {	
	__asm__ volatile ("mov %0, %%edi" : : "r"(regs));
	prot_to_real();	
}

void	read_sector_lba(void *buffer, uint16_t sectors, uint32_t lba){
	uint32_t	i = 0;
	uint32_t	clba = lba;
	char		*buffptr = (char *) buffer;
	char		rbuff[512];		// realmode seg buffer
	dap	cdap = {
		.size = 0x10,
		.reserv = 0,
		.count = 1,
		.offset = (((uint32_t) &rbuff) & 0xffff),
		.segment = (((((uint32_t) &rbuff) & 0xffff0000) >> (8*2))) / 16,
		.lba_low = lba,
		.lba_high = 0x0
	};

	bios_regs	ctx = {};
	memset(&ctx, 0, sizeof(bios_regs));
	ctx.edx = boot_drive;
	ctx.esi = (uint16_t) &cdap;
	ctx.eax = 0x4200;
	ctx.intno = 0x13;

	while (i < sectors){
		cdap.lba_low = clba;
		bios_interrupt(&ctx);
		memcpy(buffptr, rbuff, 512);
		buffptr+=512;
		clba += 512;
		i++;
	}
}

void	read_size_lba(void *buffer, uint32_t size, uint32_t lba){
	uint32_t sectors = ((uint32_t)(size / 512)) + 1;
	char *sectors_buff = malloc(sectors * 512);
	read_sector_lba(sectors_buff, sectors, lba);
	memcpy(buffer, sectors_buff, size);
	((char *)buffer)[size] = '\x00';
	free(sectors_buff);
}
