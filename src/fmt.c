#include "../inc/bootix.h"


static void puthex(unsigned int n) {
	if (n >= 16)
		puthex(n / 16);

	unsigned int digit = n % 16;
	if (digit < 10)
		putchar(digit + '0');
	else
		putchar(digit - 10 + 'a');
}

static void putptr(unsigned int n) {
	print("0x");
	puthex(n);
}


static void putnbr(int n) {
	if (n < 0)
	{
		putchar('-');
		n = -n;
	}

	if (n >= 10)
		putnbr(n / 10);
	putchar((n % 10) + '0');
}


static void fmt_handle(char fmt, va_list *arg) {
	if (fmt == 'c')
		putchar(va_arg(*arg, int));
	else if (fmt == 's')
		print(va_arg(*arg, char *));
	else if (fmt == '%')
		putchar('%');
	else if (fmt == 'd' || fmt == 'i')
		putnbr(va_arg(*arg, int));
	else if (fmt == 'x')
		puthex(va_arg(*arg, unsigned int));
	else if (fmt == 'p')
		putptr(va_arg(*arg, unsigned long long));
}

void printf(char *fmt, ...) {
	int		i;
	va_list	args;

	va_start(args, fmt);
	i = 0;
	while (fmt[i] != '\x00')
	{
		if (fmt[i] == '%')
			fmt_handle(fmt[++i], &args);
		else
			putchar(fmt[i]); 
		if (fmt[i] != '\x00')
			i++;
	}
	va_end(args);
}

void vprintf(char *fmt, va_list args) {
 	int		i;

	va_start(args, fmt);
	i = 0;
	while (fmt[i] != '\x00')
	{
		if (fmt[i] == '%')
			fmt_handle(fmt[++i], &args);
		else
			putchar(fmt[i]);
		if (fmt[i] != '\x00')
			i++;
	}
	va_end(args);
}
