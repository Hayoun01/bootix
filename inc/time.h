#ifndef TIME_H
#define TIME_H

uint32_t get_unix_timestamp(void);
uint32_t date_to_unix(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

#endif
