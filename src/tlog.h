/* vim: ft=c ff=unix fenc=utf-8
 * file: src/tlog.h
 */
#ifndef _SRC_TLOG_1478609591_H_
#define _SRC_TLOG_1478609591_H_

#include <stdarg.h>

/* initializators */
#define TL_X const char *tlog_parent_func = NULL
/* arguments */
#define TL_V const char *tlog_parent_func
#define TL_A __func__

enum tlog_level {
	TLOG_NONE = 0,
	TLOG_ALERT,
	TLOG_CRIT,
	TLOG_ERR,
	TLOG_WARN,
	TLOG_NOTICE,
	TLOG_INFO,
	TLOG_DEBUG,
	TLOG_TRACE
};

void tlog_open();

void _tlog(enum tlog_level tl, const char *parent_func, const char *func, const char *format, ...);
void _vtlog(enum tlog_level tl, const char *parent_func, const char *func, const char *format, va_list ap);

#define tlog(format, ...) _tlog(TLOG_NONE, tlog_parent_func, __func__, format, __VA_ARGS__)
#define tlog_alert(format, ...) _tlog(TLOG_ALERT, tlog_parent_func, __func__, format, __VA_ARGS__)
#define tlog_critical(format, ...) _tlog(TLOG_CRIT, tlog_parent_func, __func__, format, __VA_ARGS__)
#define tlog_error(format, ...) _tlog(TLOG_ERROR, tlog_parent_func, __func__, format, __VA_ARGS__)
#define tlog_warn(format, ...) _tlog(TLOG_WARN, tlog_parent_func, __func__, format, __VA_ARGS__)
#define tlog_notice(format, ...) _tlog(TLOG_NOTICE, tlog_parent_func, __func__, format, __VA_ARGS__)
#define tlog_info(format, ...) _tlog(TLOG_INFO, tlog_parent_func, __func__, format, __VA_ARGS__)
#define tlog_debug(format, ...) _tlog(TLOG_DEBUG, tlog_parent_func, __func__, format, __VA_ARGS__)
#define tlog_trace(format, ...) _tlog(TLOG_TRACE, tlog_parent_func, __func__, format, __VA_ARGS__)

void tlog_close();


#endif /* _SRC_TLOG_1478609591_H_ */

