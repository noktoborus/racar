/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model_loader.c
 */
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "tlog.h"
#include "mempool.h"
#include "model.h"
#include "model_module.h"
#include "model_loader.h"

static bool
_mload_load(TL_V, struct mmp *mmp, struct mdl *mdl, FILE *f)
{
	char line[4096] = {};
	struct mdl_node *mn = NULL;

	char *begin;
	char *end;

	size_t lineno = 0u;
	tlog_trace("(mdl=%p, mdl=%p, f=%p)", (void*)mmp, (void*)mdl, (void*)f);
	while (!feof(f)) {
		lineno++;
		if (!fgets(line, sizeof(line), f)) {
			tlog("can't get line %"PRIuPTR, lineno);
			break;
		}
		begin = line;
		if (isblank(*begin)) {
			/* not a path, skip spaces */
			while (isblank(*(++begin)));
		} else {
			/* get model path */
			if (!(end = strpbrk(begin, " \t"))) {
				mn = mdl_add_path(TL_A, mdl, NULL, begin);
			} else {
				*end = '\0';
				mn = mdl_add_path(TL_A, mdl, NULL, begin);
			}
			begin = end + 1;
		}
		/* TODO: get args */
		/* skip empty line */
		if (!*begin) {
			continue;
		}
		/* TODO: ... */
	}
	return true;
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

