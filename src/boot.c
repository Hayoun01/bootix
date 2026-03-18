#include "../inc/bootix.h"

static partition_table *find_kernel_fs(cnf_namespace *cnf, partition_table **fs){

}

// last step : load kernel from /boot and jump to it
void boot(cnf_namespace *cnf, partition_table **fs){
	printf("Booting from %s\n", cnf->next->ns);
	partition_table *kfs = find_kernel_fs(cnf, fs);
}
