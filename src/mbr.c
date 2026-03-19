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
	ptbl->boot_indicator = raw[0];
	ptbl->start_head     = raw[1];
	ptbl->start_sector   = raw[2] & 0x3F;
	ptbl->start_cylinder = ((raw[2] & 0xC0) << 2) | raw[3];
	ptbl->sys_id	     = raw[4];
	ptbl->end_sector     = raw[5] & 0x3F;
	ptbl->end_cylinder   = ((raw[5] & 0xC0) << 2) | raw[6];
	ptbl->lba            = *(uint32_t*)(raw + 8);
	ptbl->sectors        = *(uint32_t*)(raw + 12);
}

void extended_boot_records(partition_table **fs, uint32_t it){

}

partition_table **detect_partitions(){
	// iterating over all the partition tables
	partition_table **fs = malloc(sizeof(partition_table) * (PART_MAX + 1));
	memset(fs, 0, sizeof(partition_table) * (PART_MAX + 1));


	int i = -1;
	while (++i < 4){
		fs[i] = malloc(sizeof(partition_table));
		init_partable(fs[i], (void *) PARTABLE_LOC + (16 * i));
		// including extended partitions
		if (fs[i]->sys_id == SYS_EXT_CHS || fs[i]->sys_id == SYS_EXT_LBA){
			extended_boot_records(fs, i);
		}
#ifdef DBGX
		print_partition(fs[i]);
#endif
	}
	return (fs);
}

static cnf_namespace *load_config(partition_table *bootable){
	log(DBG, "Loading partition in lba %p\n", bootable->lba);
	fat32_obj *fs = fat32_init(bootable);
	char *conf_cont = fat32_read("btx.cnf", fs, fs->rootdir, 0, 0);
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
	partition_table **fs = detect_partitions();
	uint8_t		i = 0;
	cnf_namespace	*cnf;
	cnf_namespace	*os;
	
	// finding the first bootable partition
	while (i < PART_MAX){
		if (fs[i] == NULL || fs[i]->boot_indicator == 0x80)
			break;
		i++;
	}
	// paranoia check
	if (fs[i] == NULL){
		log(ERR, "Cannot find bootable partition");
	}
	if (fs[i]->sys_id != SYS_FAT32_CHS && fs[i]->sys_id != SYS_FAT32_LBA ){
		log(ERR, "Bootable partition is not fat32, reinstall bootix or fix manually");
	}
	log(DBG, "Loading config file from first bootable partition");
	cnf = load_config(fs[i]);
	if (cnf->next == NULL || cnf->next->ns == NULL)
		log(ERR, "No bootable operating system to boot from.");
	os = boot_menu(cnf->next);
	os = cnf_clone(os);
	cnf_free(cnf->next);				// freeing other entries
	cnf->next = os;					// saving os env with default
	boot(cnf, fs);
}
