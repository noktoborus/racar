/* vim: ft=c ff=unix fenc=utf-8
 * file: src/modules/racar.c
 */

#include <racar/base.h>
#include <racar/model_module.h>

void
module_destroy()
{
	mm_log(TLOG_INFO, "racar destroyed");
}

mm_destroy
module_init()
{
	mm_log(TLOG_INFO, "racar initialized");
	return module_destroy;
}

