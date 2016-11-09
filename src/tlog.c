/* vim: ft=c ff=unix fenc=utf-8
 * file: src/tlog.c
 */
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "tlog.h"

static FILE *log;

void
tlog_open()
{
	log = stderr;
}

void
_tlog(const char *parent_file, const char *parent_func, const char *format, ...)
{
	time_t t = 0u;
	char tm_str[24] = {};
	struct tm *tm = NULL;
	va_list ap = {};
	if (!log)
		return;

	time(&t);
	tm = gmtime(&t);
	strftime(tm_str, sizeof(tm_str), "%b %e %T", tm);
	/* begin */
	fprintf(log, "[%s] [parent %s:%s] ", tm_str, parent_file, parent_func);
	/* data */
	va_start(ap, format);
	vfprintf(log, format, ap);
	va_end(ap);
	/* end */
	fprintf(log, "\n");
	fflush(log);
}

void
tlog_close()
{
	/* ... */
}

