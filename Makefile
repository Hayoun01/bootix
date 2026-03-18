# compiling stuff
ASM = nasm
CC = i386-elf-gcc
CFLAGS = -m32 -ggdb -nostdlib -ffreestanding -mno-red-zone -fno-exceptions -T src/linker.ld # -Wall -Wextra -Werror 
QEMU = qemu-system-x86_64

NAME = bootix.img
S2NAME = env/stage2.bin

CSRC = src/bootix.c src/io.c src/fmt.c src/alloc.c src/mbr.c src/string.c \
       src/int.c src/fat32.c src/log.c src/time.c src/conf.c src/boot.c
BOOT_SRC = src/boot.s
S2_SRC = src/entry.s

# some partition variables
PART_START = 2048
PART_TYPE = c			# fat32 atm
MKFS = mkfs.fat
FS_LABEL = BOOTIX
PART_FILES = env/btx.cnf env/stage2.bin
LOOP_DEV = /dev/loop0
SIZE=100M
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

img: stage1 stage2
	dd if=/dev/zero of=$(NAME) bs=$(SIZE) count=1
	printf "label: dos\nunit: sectors\n\n$(NAME)1 : start=$(PART_START), type=c, bootable\n" | sfdisk $(NAME)
	dd if=$(STAGE1) of=$(NAME) bs=446 count=1 conv=notrunc
	$(MKFS) -F 32 --offset $(PART_START) $(NAME)		# for now we are working with fat32 as a bootable disk
	mcopy -i $(NAME)@@$(OFFSET) $(PART_FILES) ::



run: build stage1 img
	$(QEMU) -nographic -serial mon:stdio -drive format=raw,file=$(NAME)

dbg: build stage1 img
	# $(QEMU) -nographic -serial mon:stdio -drive format=raw,file=$(NAME) -s -S
	qemu-system-i386 -nographic -serial mon:stdio -drive format=raw,file=$(NAME) -s -S


# cleaner
clean: 
	rm -rf build/

fclean: clean 
	rm $(NAME)
