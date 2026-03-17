#ifndef LOG_H
#define LOG_H

typedef enum bcd {
	SECS = 0x0,
	MINS = 0x2,
	HOUR = 0x4,
	STAT = 0xb,
}bcd;

typedef enum LOG_LVL {
	DBG,
	WARN,
	SUCCESS,
	ERR,
} LOG_LVL;


void log(LOG_LVL loglvl, char *fmt, ...);

#endif
