/* vim: ft=c ff=unix fenc=utf-8
 * file: src/modules/simple.c
 */
#include <racar/model_module.h>

static void
module_destroy()
{
	mm_log(TLOG_INFO, "simple module destroyed");
}

mm_destroy
module_init()
{
	mm_log(TLOG_INFO, "simple module initialized");
	return module_destroy;
}

