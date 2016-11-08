/* vim: ft=c ff=unix fenc=utf-8
 * file: src/base.c
 */
#include <stdlib.h>
#include "base.h"

struct rcr_gate *
rcr_add_gate(TL_V, struct rcr *r, const char *name)
{
	void *tmp = NULL;
	tmp = realloc(r->gate, (r->gates + 1) * sizeof(*r->gate));
	if (!tmp) {
		return NULL;
	}
	return NULL;
}

