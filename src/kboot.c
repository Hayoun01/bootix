
#include "../inc/bootix.h"

static partition_table *find_kernel_fs(cnf_namespace *cnf, partition_table *fs){
	cnf_entry	*bpartenv = cnf_search_entries(cnf->next->entry, "boot_part");
	partition_table	*bpart = fs;
	uint32_t	bpart_num = 0;
	uint32_t	i = 0;

	if (bpartenv == NULL || bpartenv->val == NULL){
		log(ERR, "bpart env not found, please run os-probe");
	}
	bpart_num = atoi(bpartenv->val);
	// iterating over bparts to find the right one
	log(DBG, "Loading kernel from partition");

	while (bpart != NULL && i < bpart_num){
		bpart = bpart->next;
		i++;
	}
	if (i < bpart_num){
		log(ERR, "Bootable partition number is greater than available partitions");
	}
	
	return (bpart);
}


// chainloading to another bootloader
void chainload(cnf_namespace *cnf, partition_table *fs){
	// TODO
}

void kenv(linux_kernel_header *khdr, cnf_namespace *cnf){
	if ((khdr->loadflags & 0x1) != 1){
		log(ERR, "Bootloader only accepts bzImages for the moment.");
	}
	khdr->type_of_loader = 0xff;		// Otherwise, enter 0xFF here.
	if (cnf_search_entries(cnf->entry, "quiet")){		// suppress early messages
		khdr->loadflags |= (1 << 5);
	} else {
		khdr->loadflags &= ~(1 << 5);
	}
	// TODO : KEEP_SEGMENTS
	khdr->loadflags |= (1 << 7);		// we can use heap (TODO : load a valid heap)
	if (khdr->version < 0x0203){
		log(WARN, "Kernel version is too old, might experience some hiccups");
	}
	khdr->vid_mode = 0xffff;		// letting the kernel decide
	khdr->root_dev = 0; 
	khdr->realmode_swtch = 0;
	khdr->ext_loader_ver = 0;
	khdr->ext_loader_type = 0;
	// TODO: 
	khdr->cmd_line_ptr = CMDLINE_PTR;
	cnf_entry * cmdline= cnf_search_entries(cnf->entry, "cmdline");
	if (cmdline && cmdline->val){
		khdr->cmdline_size = strlen(cmdline->val) + 1;
		memcpy((void *)khdr->cmd_line_ptr, cmdline->val, khdr->cmdline_size + 1);
	}
	khdr->heap_end_ptr = HEAP_END_PTR;		// CHANGE ME
}

// for now we're only working with bzlinux
void kload(linux_kernel_header *khdr, fat32_obj *kfs, char *kfname){
	// loading real mode code
	if (khdr->setup_sects == 0)
		khdr->setup_sects = 4;

	void *rload_mem = (void *) 0x90000;		// CHANGE ME
	fat32_read(rload_mem, kfname, kfs, kfs->rootdir, (khdr->setup_sects + 1) * 512, 0);
	
	// loading compressed kernel now
	fat32_read((void *) khdr->code32_start, kfname, kfs, kfs->rootdir, 0, (khdr->setup_sects + 1) * 512);
	
	memcpy(rload_mem + 0x1f1, khdr, sizeof(linux_kernel_header));
}

char *kchose(cnf_namespace *cnf){
	char buff[8];
	cnf_entry *kfname_ent = cnf_find_allentries(cnf->next->entry, "kernel");
	char *kfname = "vmlinuz";
	
	if (cnf_len_entries(kfname_ent) > 1){
		memset(buff, 0, 8);
		printf("Please chose a kernel to boot from: \n");
		uint32_t i = 0;
		cnf_entry *kfit = kfname_ent;
		while (kfit != NULL){
			printf("\t[%d] - %s\n", i, kfit->val);
			kfit = kfit->next;
			i++;
		}
		int32_t choise = -1;
		while (choise > i || choise < 0){
			printf(">> ");
			read(buff, 7);
			choise = atoi(buff);
			if (choise > i || choise < 0){
				printf("Unvalid entry.\n");
			}
		}
		i = 0;
		kfit = kfname_ent;
		while (i < choise){
			kfit = kfit->next;
			i++;
		}
		kfname = strdup(kfit->val);
		cnf_free_entries(kfname_ent);
		return (kfname);

	}
	else if (kfname_ent != NULL)
		kfname = kfname_ent->val;

	return (strdup(kfname));
}


// last step : load kernel from /boot and jump to it
void boot(cnf_namespace *cnf, partition_table *fs){
	partition_table *kpart = find_kernel_fs(cnf, fs);
	fat32_obj *kfs = fat32_init(kpart);
	linux_kernel_header *khdr;
	char *kfname = kchose(cnf);

#ifndef DBGX
	clear_screen();
#endif
	printf("Booting from %s\n", cnf->next->ns);
	khdr = (linux_kernel_header *) fat32_read(NULL, kfname, kfs, kfs->rootdir, 1024, 0x01f1);
	if (khdr == NULL){
		printf("[!!!!] - Kernel %s, not found in boot partition", kfname);
		hang();
	}
	if (khdr->header != 0x53726448) {
		if (khdr->boot_flag == 0xaa55)
			log(ERR, "Kernel is too old.");
		else 
			log(ERR, "Corrupted kernel header");
	}
	kenv(khdr, cnf->next);

	// env done, loading the kernel
	kload(khdr, kfs, kfname);


	kchain();
}

