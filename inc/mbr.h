#ifndef MBR_H
#define MBR_H

#define PART_MAX	0x20

#define PARTABLE_LOC	0x6000 + 0x1be
#define PARTABLE_SIZE	16


// sys_ids
#define SYS_FAT32_CHS	0xb
#define SYS_FAT32_LBA	0xc

#define SYS_EXT_CHS	0x5
#define SYS_EXT_LBA	0xf

typedef struct partition_table {
	uint8_t		boot_indicator;
	uint8_t		start_head;
	uint8_t		start_sector;
	uint16_t	start_cylinder;
	uint8_t		sys_id;
	uint8_t		end_sector;
	uint16_t	end_cylinder;
	uint32_t	lba;
	uint32_t	sectors;
} partition_table;


void multiboot();

#endif
