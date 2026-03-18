#include "../inc/bootix.h"


uint32_t strcmp(char *sa, char *sb){
	uint32_t i = 0;
	while (sa[i] == sb[i] && sb[i] != '\x00' && sa[i] != '\x00' ){
		i++;
	}
	return (sa[i] - sb[i]);
}

uint32_t strlen(char *s){
	uint32_t i = 0;
	while (s[i] != '\x00'){
		i++;
	}
	return (i);
}

void strcpy(char *dst, char *src){
	uint32_t i = 0;
	if (dst == NULL || src == NULL)
		return ;
	while (src[i] != '\0'){
		dst[i] = src[i];
		i++;
	}
	dst[i] = '\x00';
}

char *strdup(char *s){
	char *ret = malloc(strlen(s) + 1);
	strcpy(ret, s);
	return (ret);
}

char *strchr(char *s, uint8_t c){
	uint32_t i = 0;

	while (s[i] != c && s[i] != 0) {
		i++;
	}
	if (s[i] == 0)
		return (NULL);
	return (s + i);
}

void memcpy(void *dst, void *src, uint32_t n){
	if (dst == NULL || src == NULL)
		return ;
	uint32_t i = 0;
	while (i < n){
		*(uint8_t *)dst++ = *(uint8_t *)src++;
		i++;
	}
}

void memset(void *s, uint8_t c, uint32_t n){
	uint32_t i = 0;
	while(i < n){
		*(char *)s = c;
		i++;
	}

}

char upper(char c){
	if (c >= 'a' && c <= 'z'){
		return (c - 32);
	}
	return (c);
}

int isdigit(int c) {
	return (c >= '0' && c <= '9');
}

int32_t atoi(char *s) {
	int32_t i = 0;
	int8_t sign = 1;
	int32_t nbr = 0;

	while (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r')
		i++;
	if (s[i] == '-')
		sign = -1;
	if (s[i] == '-' || s[i] == '+')
		i++;
	while (isdigit(s[i])) {
		nbr = nbr * 10 + (s[i] - '0');
		i++;
	}
	return (nbr * sign);
}
