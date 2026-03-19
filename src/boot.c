#include "../inc/bootix.h"

static partition_table *find_kernel_fs(cnf_namespace *cnf, partition_table **fs){
	cnf_entry	*bpartenv = cnf_search_entries(cnf->next->entry, "boot_part");
	partition_table	**bpart = fs;
	uint32_t	bpart_num = 0;
	uint32_t	i = 0;

	if (bpartenv == NULL | bpartenv->val == NULL){
		log(ERR, "bpart env not found, please run os-probe");
	}
	bpart_num = atoi(bpartenv->val);
	// iterating over bparts to find the right one
	log(DBG, "Loading kernel from partition");

	while (bpart[i] != NULL){
		i++;
	}
	if (i < bpart_num){
		log(ERR, "Bootable partition number is greater than available partitions");
	}
	
	return (bpart[bpart_num]);
}

// last step : load kernel from /boot and jump to it
void boot(cnf_namespace *cnf, partition_table **fs){
	partition_table *kpart = find_kernel_fs(cnf, fs);
	fat32_obj *kfs = fat32_init(kpart);
	linux_kernel_header *khdr;
	cnf_entry *kfname_ent = cnf_search_entries(cnf->next->entry, "kernel_filename");
	char *kfname = "vmlinuz";

	if (kfname_ent != NULL)
		kfname = kfname_ent->val;


	printf("Booting from %s\n", cnf->next->ns);
	khdr = (linux_kernel_header *) fat32_read(kfname, kfs, kfs->rootdir, 1024, 0x01f1);
	if (khdr == NULL){
		printf("[!!!!] - Kernel %s, not found in boot partition", kfname);
		hang();
	}
	if (khdr->header != 0x53726448) {
		log(ERR, "Corrupted kernel header");
	}

}
