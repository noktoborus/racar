/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model_loader.c
 */
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "tlog.h"
#include "mempool.h"
#include "model.h"
#include "model_module.h"
#include "model_loader.h"

static bool
_mload_load(TL_V, struct mmp *mmp, struct mdl *mdl, FILE *f)
{
	tlog_trace("(mdl=%p, mdl=%p, f=%p)", (void*)mmp, (void*)mdl, (void*)f);
	/* TODO */
	return false;
}

bool
mload_load(TL_V, struct mdl *mdl, const char *file)
{
	struct mmp *mmp = NULL;
	FILE *f = NULL;
	bool r = false;

	tlog_trace("(mdl=%p, file=%p [%s])", (void*)mdl, (void*)file, file);

	if (!(mmp = mmp_create())) {
		tlog_warn("Load model from file '%s' failed: %s",
				file, strerror(errno));
		return false;
	}

	f = (FILE*)mmp_assign(mmp, (void*)fopen(file, "r"), (void(*)(void*))fclose);
	if (f) {
		r = _mload_load(TL_A, mmp, mdl, f);
	}

	mmp_destroy(mmp);
	return r;
}

