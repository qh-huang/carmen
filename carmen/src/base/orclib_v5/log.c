#include <stdio.h>
#include <sys/time.h>
#include <stdarg.h>

#include "log.h"

static int logLevel = 4;
static struct timeval _starttv;

static char *LOGLEVELS[] = {"ERROR", "WARN", "INFO", "VERBOSE", "DEBUG"};
static int logfd = -1;

#define VPRINTALLOC                                                \
	int sz = 4096;						   \
	char *buf = NULL;                                          \
	_revprintf:                                                \
        buf = (char*) realloc(buf,sz);	               		   \
	va_list ap;                                                \
	int len;                                                   \
	va_start(ap, fmt);                                         \
	len = vsnprintf(buf, 4000, fmt, ap);                       \
	if (len < 0 || len>=sz)                                    \
            { sz*=2; goto _revprintf; }                            \
	va_end(ap);                                                


void _dolog(int level, const char *file, int line, const char *format, ...)
{
	if (level >= logLevel)
		return;

	struct timeval _tv;
	VPRINTALLOC; // this sets up 'buf' and 'len'

	gettimeofday(&_tv, NULL);
	long t = _tv.tv_sec*1000 + _tv.tv_usec/1000 - _starttv.tv_sec*1000 + _starttv.tv_usec/1000;
	writef(FILENO_STDOUT, "%8s %6li.%03li %10s:%4i   ", LOGLEVELS[level], t/1000, t%1000, file, line);
	vprintf(format, va);
	printf("\n");

	free(buf);
}
    
void log_init()
{
	gettimeofday(&_starttv, NULL);
}

void log_set_level(int level)
{
	logLevel = level;
}
