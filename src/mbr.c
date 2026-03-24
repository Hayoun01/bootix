#include "../inc/bootix.h"

///////////////////////////  DBG  ///////////////////////////
#ifdef DBGX
static void print_partition(partition_table *ptbl){
	puts("[PARTITION TABLE]");
	printf("\tboot_indicator %d\n", ptbl->boot_indicator);
	printf("\tstart_head %d\n", ptbl->start_head);
	printf("\tstart_sector %d\n", ptbl->start_sector);
	printf("\tstart_cylinder %d\n", ptbl->start_cylinder);
	printf("\tsys_id %d\n", ptbl->sys_id);
	printf("\tend_sector %d\n", ptbl->end_sector);
	printf("\tend_cylinder %d\n", ptbl->end_cylinder);
	printf("\tLBA %d\n", ptbl->lba);
	printf("\tsectors %d\n", ptbl->sectors);

}
#endif
///////////////////////////  DBG  ///////////////////////////



// init_partable - inits partition table
void init_partable(partition_table *ptbl, void *ptbl_addr){
	uint8_t *raw = (uint8_t*) ptbl_addr;
	ptbl->boot_indicator	= raw[0];
	ptbl->start_head	= raw[1];
	ptbl->start_sector	= raw[2] & 0x3F;
	ptbl->start_cylinder	= ((raw[2] & 0xC0) << 2) | raw[3];
	ptbl->sys_id		= raw[4];
	ptbl->end_sector	= raw[5] & 0x3F;
	ptbl->end_cylinder	= ((raw[5] & 0xC0) << 2) | raw[6];
	ptbl->lba		= *(uint32_t*)(raw + 8);
	ptbl->sectors		= *(uint32_t*)(raw + 12);
	ptbl->next		= NULL;
}

partition_table *detect_partitions(void *partable_ptr){
	// iterating over all the partition tables
	partition_table *fs = NULL;
	partition_table *fsit = NULL;

	int i = -1;
	while (++i < 4){
		if (*(char *)partable_ptr == 0) {
			continue;
		}
		if (fsit == NULL){
			fsit = malloc(sizeof(partition_table));
			fs = fsit;
		} else {

			fsit->next = malloc(sizeof(partition_table));
			fsit = fsit->next;
		}
		init_partable(fsit, (void *) partable_ptr + (16 * i));
		// including extended partitions
		if (fsit->sys_id == SYS_EXT_CHS || fsit->sys_id == SYS_EXT_LBA){
			// reading the extended boot record
			char *ebr = malloc(512);
			read_sector_lba(ebr, 1, fsit->lba);
			partition_table *efs = detect_partitions(ebr + 0x1be);
			fsit->next = efs;
			while (efs->next != NULL)
				efs = efs->next;
			fsit = efs;
			free(ebr);
		}
#ifdef DBGX
		print_partition(fsit);
#endif
	}
	return (fs);
}

static cnf_namespace *load_config(partition_table *bootable){
	log(DBG, "Loading partition in lba %p\n", bootable->lba);
	fat32_obj *fs = fat32_init(bootable);
	char *conf_cont = fat32_read(NULL, "btx.cnf", fs, fs->rootdir, 0, 0);
	cnf_namespace *cnf;
	if (conf_cont == NULL){
		log(ERR, "Config file (btx.cnf) not found, boot from another usb and run os-probe");
	}
	fat32_obj_free(fs);
	cnf = cnf_parse(conf_cont);	// parsing the file
	free(conf_cont);
	return (cnf);
	
}

cnf_namespace *boot_menu(cnf_namespace *cnf){
	uint32_t i = 0;
	cnf_namespace	*cnfit = cnf;
	char buff[8];
	uint32_t num = -1;
	// quick sanity check to multi boot
	while (cnfit != NULL){
		cnfit = cnfit->next;
		i++;
	}
	if (i < 2)
		return cnf;


	i = 0;
	cnfit = cnf;
	while (cnfit != NULL){
		printf("[%d] - %s\n",i++, cnfit->ns);
		cnfit = cnfit->next;
	}


	while (true){
		printf("Chose an operating system to boot from [0-%d] >> ", i-1);
		memset(buff, '\x00' , 8);
		read(buff, 7);
		num = atoi(buff);
		if (num > i || num < 0){
			printf("Invalid entry\n");
		} else {
			break;
		}
	}
	i = 0;
	cnfit = cnf;
	while (i < num){
		cnfit = cnfit->next;
		i++;
	}

	return (cnfit);
}

void multiboot(){
	partition_table *fs = detect_partitions((void *)PARTABLE_PTR);
	partition_table *fsit = fs;
	uint8_t		i = 0;
	cnf_namespace	*cnf;
	cnf_namespace	*os;
	
	// finding the first bootable partition
	while (fsit != NULL){
		if (fsit == NULL || fsit->boot_indicator == 0x80)
			break;
	}
	// paranoia check
	if (fsit == NULL){
		log(ERR, "Cannot find bootable partition");
	}
	if (fs->sys_id != SYS_FAT32_CHS && fs->sys_id != SYS_FAT32_LBA ){
		log(ERR, "Bootable partition is not fat32, reinstall bootix or fix manually");
	}
	log(DBG, "Loading config file from first bootable partition");
	cnf = load_config(fsit);
	if (cnf->next == NULL || cnf->next->ns == NULL)
		log(ERR, "No bootable operating system to boot from.");
	os = boot_menu(cnf->next);
	os = cnf_clone(os);
	cnf_free(cnf->next);				// freeing other entries
	cnf->next = os;					// saving os env with default
	if ( cnf_search_entries(cnf->next->entry, "chainload")){
		chainload(cnf, fs);
	} else {
		boot(cnf, fs);
	}
}
