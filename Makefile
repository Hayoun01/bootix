# compiling stuff
ASM = nasm
CC = i386-elf-gcc
CFLAGS = -m32 -ggdb -nostdlib -ffreestanding -mno-red-zone -fno-exceptions -T src/linker.ld # -Wall -Wextra -Werror 
QEMU = qemu-system-x86_64

NAME = bootix.img
S2NAME = env/stage2.bin

CSRC = src/bootix.c src/io.c src/fmt.c src/alloc.c src/mbr.c src/string.c \
       src/int.c src/fat32.c src/log.c src/time.c src/conf.c src/kboot.c
BOOT_SRC = src/boot.s
S2_SRC = src/entry.s

# some partition variables
SIZE=200M
PART_START = 2048
BOOTP_SIZE = 100					# in MB
BOOTP_SECS = $(shell echo $$(($(BOOTP_SIZE) * 2048)))	# in sectors
BOOTP_SIZEB = $(shell echo $$(($(BOOTP_SECS) * 512)))   # in BYTES 
BOOTP_OFF  = $(shell echo $$(($(PART_START) * 512)))
TOTAL_SIZE = 250					# Total size in MB
TOTAL_SECS = $(shell echo $$(($(TOTAL_SIZE) * 2048))) 

# rootfs vars
ROOTFS_START_SEC = $(shell echo $$(($(PART_START) + $(BOOTP_SECS))))
ROOTFS_SECS = $(shell echo $$(($(TOTAL_SECS) - $(ROOTFS_START_SEC))))
ROOTFS_OFF  = $(shell echo $$(($(ROOTFS_START_SEC) * 512)))
ROOTFS_SIZEB= $(shell echo $$(($(ROOTFS_SECS) * 512)))		# in bytes

BOOT_IMG = build/boot.fat32
ROOT_IMG = build/rootfs.ext4

PART_FILES = env/btx.cnf env/stage2.bin env/vmlinuz
STAGE1 = build/boot.bin
STAGE2 = build/entry.bin

OFFSET=$(shell echo $$(($(PART_START)*512)))

all: build stage1 stage2

build:
	if [ ! -d "build/" ]; then \
		mkdir build; \
	fi

stage1: 
	$(ASM) -f bin $(BOOT_SRC) -o $(STAGE1)

stage2:
	$(ASM) -f elf $(S2_SRC) -o $(STAGE2)
	$(CC) $(CSRC) $(STAGE2) -o build/stage2.elf $(CFLAGS)
	objcopy -O binary build/stage2.elf $(S2NAME)

img: stage1 stage2 $(BOOT_IMG) $(ROOT_IMG)
	dd if=/dev/zero of=$(NAME) bs=1M count=$(TOTAL_SIZE)
	# creating bootable partition table
	printf "label: dos\nunit: sectors\n\n$(NAME)1 : start=$(PART_START), size=$(BOOTP_SECS), type=c, bootable\n$(NAME)2 : start=$(ROOTFS_START_SEC), size=$(ROOTFS_SECS), type=83\n" | sfdisk $(NAME) > /dev/null 2>&1
	# inserting the bootloader inthe mbr
	dd if=$(STAGE1) of=$(NAME) bs=446 count=1 conv=notrunc
	# copying the bootable partition
	dd if=$(BOOT_IMG) of=$(NAME) bs=512 seek=$(PART_START) conv=notrunc status=none
	# copying rootfs to the second partition
	dd if=$(ROOT_IMG) of=$(NAME) bs=512 seek=$(ROOTFS_START_SEC) conv=notrunc status=none

# creating the bootable partition
$(BOOT_IMG):
	dd if=/dev/zero of=$@ bs=1 count=0 seek=$(BOOTP_SIZEB) status=none
	mkfs.fat -F 32 $@ > /dev/null
	mcopy -i $@ $(PART_FILES) ::

$(ROOT_IMG):
	dd if=/dev/zero of=$@ bs=1 count=0 seek=$(ROOTFS_SIZEB) status=none
	mkfs.ext4 -d env/initramfs -O ^64bit -F $@ > /dev/null



run: fclean build stage1 img
	$(QEMU) -nographic -serial mon:stdio -drive format=raw,file=$(NAME)

dbg: fclean build stage1 img
	# $(QEMU) -nographic -serial mon:stdio -drive format=raw,file=$(NAME) -s -S
	qemu-system-i386 -nographic -serial mon:stdio -drive format=raw,file=$(NAME) -s -S


# cleaner
clean: 
	rm -rf build/

fclean: clean 
	rm $(NAME)
