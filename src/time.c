#include "../inc/bootix.h"

uint8_t read_cmos(uint8_t bcd) {
	uint8_t value;
	asm volatile (
		"outb %1, $0x70\n\t"
		"inb $0x71, %0"
		: "=a"(value)
		: "a"(bcd)
	);
	return value;
}

static uint8_t bcd_to_bin(uint8_t bcd) {
	return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

static int is_leap(uint16_t year) {
	return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));		// literally fuck you
}

uint32_t date_to_unix(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
	static const uint8_t month_days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
	uint32_t days = 0;
	for (uint16_t y = 1970; y < year; y++)
		days += is_leap(y) ? 366 : 365;
	for (uint8_t m = 1; m < month; m++) {
		days += month_days[m-1];
		if (m == 2 && is_leap(year))
			days++;
	}
	days += day - 1;
	return (days * 86400) + (hour * 3600) + (minute * 60) + second;
}

uint32_t get_unix_timestamp(void) {
	while (read_cmos(0x0A) & 0x80) ;	// waiting on cmos gods to bless us

	uint8_t sec_bcd  = read_cmos(0x00);
	uint8_t min_bcd  = read_cmos(0x02);
	uint8_t hour_bcd = read_cmos(0x04);
	uint8_t day_bcd  = read_cmos(0x07);
	uint8_t mon_bcd  = read_cmos(0x08);
	uint8_t year_bcd = read_cmos(0x09);
	uint8_t regB     = read_cmos(0x0B);

	if (!(regB & 0x04)) {
		sec_bcd  = bcd_to_bin(sec_bcd);
		min_bcd  = bcd_to_bin(min_bcd);
		day_bcd  = bcd_to_bin(day_bcd);
		mon_bcd  = bcd_to_bin(mon_bcd);
		year_bcd = bcd_to_bin(year_bcd);
		if (regB & 0x02) { // 24‑hour mode
			hour_bcd = bcd_to_bin(hour_bcd);
		} else {
			uint8_t hour_12 = bcd_to_bin(hour_bcd & 0x7F);
			if (hour_bcd & 0x80) { // PM
				hour_bcd = (hour_12 == 12) ? 12 : hour_12 + 12;
			} else { // AM
				hour_bcd = (hour_12 == 12) ? 0 : hour_12;
			}
		}
	}

	uint16_t year = 2000 + year_bcd;
	return date_to_unix(year, mon_bcd, day_bcd, hour_bcd, min_bcd, sec_bcd);
}
