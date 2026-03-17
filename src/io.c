#include "../inc/bootix.h"


static inline void outb(unsigned short port, unsigned char val) {
	__asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
	unsigned char ret;
	__asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

void putchar(char c) {
	while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
	outb(SERIAL_PORT, c);
}

void print(const char *str){
	while (*str) {
		putchar(*str++);
	}
}

// serial print
void puts(const char *str) {
	print(str);
	print("\r\n");
}

unsigned char readchar() {
	while ((inb(SERIAL_PORT + 5) & 0x01) == 0);
	return inb(SERIAL_PORT);
}

void read(char *buff, uint32_t len){
	uint32_t	i = 0;
	char		c = readchar();

	while (i < (len - 1)){
		if (c == '\n' || c == '\r'){
			putchar('\n');
			break;
		}
		if ((c == 0x7f || c == 0x8) && i > 0){
			i--;
			putchar(0x08);
			putchar(' ');
			putchar(0x08);
		} else {
			buff[i++] = c;
			putchar(c);
		}
		c = readchar();
	}
	buff[i] = '\0';
}

void write(void *buf, uint32_t nbyte){
	uint32_t i = 0;
	while (i < nbyte){
		putchar(*((char *) buf + i));
	}
}
