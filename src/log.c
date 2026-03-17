#include "../inc/bootix.h"


// TODO : ADD COLORS
void log(LOG_LVL loglvl, char *fmt, ...){
	va_list args;
	va_start(args, fmt);


	if (loglvl == DBG) {
#ifdef DBGX
		printf("[%d] - ", get_unix_timestamp());
		vprintf(fmt, args);
		puts("");
#endif

	} else {
		printf("[%d] - ", get_unix_timestamp());
		vprintf(fmt, args);
		puts("");
		if (loglvl == ERR){
			hang();
		}
	}
	va_end(args);
}
