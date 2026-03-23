#include "../inc/bootix.h"

static uint32_t cluster_to_lba(uint32_t cluster, fat32_obj *fs_info){
	return (fs_info->cluster_beg + (cluster - fs_info->volid->BPB_NumFATs) * fs_info->volid->BPB_SecPerClus) ;
}

#ifdef DBGX
void print_dir_entry(fat32_dir_entry *dir){
	puts("[FAT32 rootdir entry]");
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

	while (*buffptr != '\0'){
		memcpy(idir, buffptr, sizeof(fat32_dir_entry) - sizeof(fat32_dir_entry *));
		buffptr += sizeof(fat32_dir_entry) - sizeof(fat32_dir_entry *);
#ifdef DBGX
		print_dir_entry(idir);
#endif
		if (*buffptr == '\0' || *buffptr == '\xe5'){
			idir->next = NULL;
			return (dir);
		}
		idir->next = malloc(sizeof(fat32_dir_entry));
		idir = idir->next;
	}

	return (NULL);
}

uint32_t fat32_filenamecmp(char *f32, char *fn){
	uint8_t i = 0;
	while (*f32 != '\0' && *fn != '\0' && i < 10){
		// checking here for ext and skipping whitespaces
		if (*fn == '.' || *f32 == ' '){
			if (*fn == '.')
				fn++;			// skipping the '.'
			while(*f32 == ' ') {
				f32++;
				i++;
			}
		}
		if (*f32 != upper(*fn)){
			return (*f32 - *fn);
		}
		f32++;
		fn++;
		i++;
	}
	if (*f32 == ' ')
		*f32 = 0x0;
	return (*f32 - upper(*fn));
}

uint32_t fat32_dir_cluster(fat32_dir_entry *dir){
	if (dir == NULL)
		return (0);
	return ((dir->first_cluster_hi << (8*2)) | dir->first_cluster_lo);
}

// opens and read file
char *fat32_read(char *buff, char *filename, fat32_obj *fs, fat32_dir_entry *dentry, uint32_t size, uint32_t offset){
	char tmp[512];

	while (dentry != NULL){
		if (fat32_filenamecmp(dentry->filename, filename) == 0)
			break;
		dentry = dentry->next;
	}
	if (dentry == NULL)
		return NULL;

	if (offset >= dentry->file_size)
		return NULL;
	if (size == 0 || offset + size > dentry->file_size)
		size = dentry->file_size - offset;

	if (buff == NULL){
		buff = malloc(size + 1);
		if (!buff)
			return NULL;
		memset(buff, 0, size + 1);
	}

	uint32_t lba = cluster_to_lba(fat32_dir_cluster(dentry), fs) + (offset / 512);

	read_sector_lba(tmp, 1, lba);

	uint32_t first = 512 - (offset % 512);
	if (first > size)
		first = size;

	memcpy(buff, tmp + (offset % 512), first);

	if (size > first){
		uint32_t remain = size - first;
		lba++;
		
		uint32_t sectors = remain / 512;
		if (sectors > 0) {
			read_sector_lba(buff + first, sectors, lba);
			lba += sectors;
		}
		
		uint32_t last_partial = remain % 512;
		if (last_partial > 0) {
			read_sector_lba(tmp, 1, lba);
			memcpy(buff + first + (sectors * 512), tmp, last_partial);
		}
	}

	buff[size] = '\0';
	return buff;
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
	if (fs_info->rootdir == NULL){
		log(ERR, "No config file is found on bootable directory, please run os-probe to generate one");
	}
	return (fs_info);
}

void *fat32_obj_free(fat32_obj *fs){
	fat32_dir_entry *dentry = fs->rootdir;
	// freeing rootdir entries lol
	while (dentry != NULL){
		free(dentry);
		dentry = dentry->next;
	}
	free(fs->volid);
	free(fs);

	return (NULL);
}

