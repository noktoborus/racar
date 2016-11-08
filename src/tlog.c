/* vim: ft=c ff=unix fenc=utf-8
 * file: src/tlog.c
 */
#include <stdio.h>
#include <stdarg.h>

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
	va_list ap = {};
	if (!log)
		return;
	/* begin */
	fprintf(log, "[] [parent %s:%s] ", parent_file, parent_func);
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

