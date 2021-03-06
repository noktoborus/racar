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
_tlog(enum tlog_level tl, const char *parent_func, const char *func, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	_vtlog(tl, parent_func, func, format, ap);
	va_end(ap);
}

void
_vtlog(enum tlog_level tl, const char *parent_func, const char *func, const char *format, va_list ap)
{
	time_t t = 0u;
	char tm_str[24] = {0};
	struct tm *tm = NULL;
	char *lv = NULL;
	if (!log)
		return;

	switch(tl) {
		case TLOG_NONE:
			break;
		case TLOG_ALERT:
			lv = "ALERT: ";
			break;
		case TLOG_CRIT:
			lv = "CRITICAL: ";
			break;
		case TLOG_ERR:
			lv = "ERROR: ";
			break;
		case TLOG_WARN:
			lv = "WARNING: ";
			break;
		case TLOG_NOTICE:
			lv = "NOTICE: ";
			break;
		case TLOG_INFO:
			lv = "INFO: ";
			break;
		case TLOG_DEBUG:
			lv = "DEBUG: ";
			break;
		case TLOG_TRACE:
			lv = "TRACE: ";
			break;
	};

	time(&t);
	tm = gmtime(&t);
	strftime(tm_str, sizeof(tm_str), "%b %e %T", tm);
	/* begin */
	if (parent_func) {
		fprintf(log, "[%s] %s[%s {%s}] ",tm_str, lv ? lv : "", parent_func, func);
	} else {
		fprintf(log, "[%s] %s[%s] ",tm_str, lv ? lv : "", func);
	}
	/* data */
	vfprintf(log, format, ap);
	/* end */
	fprintf(log, "\n");
	fflush(log);
}

void
tlog_close()
{
	/* ... */
}

