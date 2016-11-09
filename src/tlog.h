/* vim: ft=c ff=unix fenc=utf-8
 * file: src/tlog.h
 */
#ifndef _SRC_TLOG_1478609591_H_
#define _SRC_TLOG_1478609591_H_

#define TL_V const char *tlog_parent_func
#define TL_A __func__

void tlog_open();

#define tlog(format, ...) _tlog(tlog_parent_func, __func__, format, __VA_ARGS__)
void _tlog(const char *parent_func, const char *func, const char *format, ...);

void tlog_close();


#endif /* _SRC_TLOG_1478609591_H_ */

