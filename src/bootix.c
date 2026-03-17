#include "../inc/bootix.h"

void version(){
	puts("BOOTIX version " VERSION);
	puts("--- An experimental bootloader ---");
}

void hang(void){
	while (true);
}

int boot_main(void){
	version();
	log(DBG, "Searching for boot partitions");
	multiboot();
	hang();
	return (0);
}
