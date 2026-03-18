#ifndef BOOTIX_H
#define BOOTIX_H

// #define DBGX			// DELETE ME IF YOU DON'T WANT DEBUG MSGS
#include <stdint.h>
#include <stdbool.h>

#define SERIAL_PORT	0x3F8
#define NULL		0
#define VERSION		"0.1 [Not Tested]"

// some fmt macros
typedef char* va_list;
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
#define va_start(ap, last)  ( ap = (va_list)&(last) + _INTSIZEOF(last) )
#define va_arg(ap, type)    ( *(type *)((ap += _INTSIZEOF(type)) - _INTSIZEOF(type)) )
#define va_end(ap)          ( (void)0 )

// other incs
#include "mbr.h"
#include "alloc.h"
#include "int.h"
#include "fat32.h"
#include "log.h"
#include "time.h"
#include "conf.h"
#include "boot.h"


void hang(void);

// io.c
void putchar(char c);
void puts(const char* str);
void print(const char *str);
void read(char *buff, uint32_t len);
void write(void *buf, uint32_t nbyte);
void vprintf(char *fmt, va_list args);

// fmt
void printf(char *fmt, ...);

// string functions
int32_t atoi(char *s);
void memcpy(void *dst, void *src, uint32_t n);
void memset(void *s, uint8_t c, uint32_t n);
char upper(char c);
uint32_t strcmp(char *sa, char *sb);
uint32_t strlen(char *s);
char *strdup(char *s);
char *strchr(char *s, uint8_t c);
void strcpy(char *dst, char *src);

// modes
extern void prot_to_real();


#endif
