#include "../inc/bootix.h"

static uint32_t cluster_to_lba(uint32_t cluster, fat32_obj *fs_info){
	return (fs_info->cluster_beg + (cluster - fs_info->volid->BPB_NumFATs) * fs_info->volid->BPB_SecPerClus) ;
}

#ifdef DBGX
void print_dir_entry(fat32_dir_entry *dir){
	puts("[A fucking FAT32 dir entry]");
	printf("-> filename: %s\n", dir->filename);
	printf("-> creation time: %d / creation date: %d\n", dir->creation_time, dir->creation_date);
	printf("-> access date: %d\n", dir->access_date);
	printf("-> cluster: %d\n", (dir->first_cluster_hi << 16) | dir->first_cluster_lo);
	printf("-> filesize: %d\n", dir->file_size);

}
#endif

fat32_dir_entry *fat32_open_dir(fat32_obj *fs, uint32_t cluster){
	char buffer[512];
	char *buffptr = buffer;
	fat32_dir_entry *idir = malloc(sizeof(fat32_dir_entry));
	fat32_dir_entry *dir = idir;
	read_sector_lba(buffer, 1, cluster_to_lba(cluster, fs));

	// TODO: FUCKING FIX ME
	while (*buffptr != '\0'){
		memcpy(idir, buffptr, sizeof(fat32_dir_entry) - sizeof(fat32_dir_entry));
		buffptr += sizeof(fat32_dir_entry) - sizeof(fat32_dir_entry);
		print_dir_entry(idir);
		if (*buffptr == '\0' || *buffptr == '\xe5'){
			idir->next = NULL;
			return (dir);
		}
		idir->next = malloc(sizeof(fat32_dir_entry));
		idir = idir->next;
	}

	print_dir_entry((fat32_dir_entry *)buffer);
	return (NULL);
}

fat32_obj *fat32_init(partition_table *bootable){
	fat32_obj *fs_info = malloc(sizeof(fat32_obj));
	fs_info->volid = malloc(sizeof(fat32_bpb));
	read_sector_lba(fs_info->volid, 1, bootable->lba);		// reading the volume id
	// calculating some offsets
	fs_info->fat_beg = bootable->lba + fs_info->volid->BPB_RsvdSecCnt;
	fs_info->cluster_beg = fs_info->fat_beg +
		(fs_info->volid->BPB_NumFATs * fs_info->volid->BPB_FATSz32);
	fs_info->rootdir = fat32_open_dir(fs_info, fs_info->volid->BPB_RootClus);
	
	


	return (fs_info);
}

void *fat32_obj_free(fat32_obj *fs){
	free(fs->volid);
	free(fs);
	return (NULL);
}
