#ifndef FAT32_H
#define FAT32_H

#include <stdint.h>		// shutting the lsp the fuck up

typedef struct fat32_bpb{
    uint8_t  BS_jmpBoot[3];			// jump instruction 
    uint8_t  BS_OEMName[8];			// OEM version 
    uint16_t BPB_BytsPerSec;			// bytes per sector 
    uint8_t  BPB_SecPerClus;			// Sectors per cluster
    uint16_t BPB_RsvdSecCnt;			// Number of reserved sectors (including boot sector)
    uint8_t  BPB_NumFATs;			// number of fats 
    uint16_t BPB_RootEntCnt;			// maximum number of rootdir entries 
    uint16_t BPB_TotSec16;			// total sectors in volume 
    uint8_t  BPB_Media;				// media descriptor 
    uint16_t BPB_FATSz16;			// Sectors per FAT 
    uint16_t BPB_SecPerTrk;			// sectors per track
    uint16_t BPB_NumHeads;			// num of heads
    uint32_t BPB_HiddSec;			// hidden sectors 
    uint32_t BPB_TotSec32;			// total sectors
    uint32_t BPB_FATSz32;			// sectors per fat 32 
    uint16_t BPB_ExtFlags;			// extended flags
    uint16_t BPB_FSVer;				// file system version 
    uint32_t BPB_RootClus;			// cluster number of the root directory
    uint16_t BPB_FSInfo;			// sector number of the FSInfo structure 
    uint16_t BPB_BkBootSec;			// sector number of the backup boot sector
    uint8_t  BPB_Reserved[12];
    uint8_t  BS_DrvNum;				// boot_drive
    uint8_t  BS_Reserved1;
    uint8_t  BS_BootSig;
    uint32_t BS_VolID;
    uint8_t  BS_VolLab[11];
    uint8_t  BS_FilSysType[8];			// fs in string
    uint8_t  BootCode[448];			// boot message and code
    
    /* Boot sector signature (0x55, 0xAA) */
    uint16_t BootSectorSig;
    
} __attribute__((packed)) fat32_bpb;

typedef struct fat32_dir_entry{
	uint8_t 	filename[8];
	uint8_t 	fileext[3];
	uint8_t 	attr;           	// file attributes (fat32_dir_attr)
	uint8_t 	nt_reserved;    	// reserved for NT
	uint8_t 	creation_tenths; 	// creation time (tenths of sec)
	uint16_t	creation_time;   	// creation time
	uint16_t	creation_date;  	 // creation date
	uint16_t	access_date;     	// last access date
	uint16_t	first_cluster_hi;	// high 16 bits of first cluster
	uint16_t	write_time;		// last write time
	uint16_t	write_date;      	// last write date
	uint16_t	first_cluster_lo;	// low 16 bits of first cluster
	uint32_t	file_size;      	 // file size in bytes
	struct fat32_dir_entry	*next;
} __attribute__((packed)) fat32_dir_entry;

typedef struct fat32_obj{
	fat32_bpb	*volid;			// the fucking volume id
	fat32_dir_entry	*rootdir;
	uint32_t	fat_beg;		// where does fat (tables) start
	uint32_t	cluster_beg;
}fat32_obj;

typedef enum fat32_dir_attr {
	RD_ONLY	= (1 << 0),			// READ_ONLY lsb
	HIDDEN	= (1 << 1),			// hidden dir (should not show in dir listing)
	SYS	= (1 << 2),			// System directory
	VOLID	= (1 << 3),			// Volume ID
	SUBDIR	= (1 << 4),			// Subdirectory
	ARCHIVE	= (1 << 5),			// has changed since last backup
} fat32_dir_attr;

fat32_obj *fat32_init(partition_table *bootable);

#endif
