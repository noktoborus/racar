/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model_loader.c
 */
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>

#include "tlog.h"
#include "mempool.h"
#include "model.h"
#include "model_module.h"
#include "model_loader.h"

static char *
_mload_load_attribute(TL_V, struct mmp *mmp, struct mdl *mdl, struct mdl_node *mn, char *begin)
{
	char *end = NULL;

	tlog_trace("(mmp=%p, mdl=%p, mn=%p [%s], begin=%p [%s])",
			(void*)mmp, (void*)mdl, (void*)mn, mn ? mn->name : "",
			(void*)begin, (begin ? begin : ""));

	if (!begin) {
		return NULL;
	}

	/* skip spaces */
	if (isblank(*begin)) {
		while (isblank(*(++begin)));
	}

	/* eol */
	if (!*begin) {
		return NULL;
	}

	/* get end */
	if ((end = strpbrk(begin, " \t")) == NULL) {
		end = begin + strlen(begin);
	}

	/* TODO: parse attribute */
	tlog_debug("got attribute: %s", begin);

	*end = '\0';

	return end;
}

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
		/* chop line */
		if ((end = strpbrk(begin, "\r\n")) != NULL) {
			*end = '\0';
			end = NULL;
		}
		if (!isblank(*begin)) {
			/* get model path */
			if ((end = strpbrk(begin, " \t")) != NULL) {
				*(end++) = '\0';
				mn = mdl_add_path(TL_A, mdl, NULL, begin);
			} else {
				mn = mdl_add_path(TL_A, mdl, NULL, begin);
				end = begin + strlen(begin);
			}
			begin = end;
		} else if (!mn) {
			tlog_notice("invalid line %"PRIuPTR": node path not defined", lineno);
			continue;
		}
		/* process arguments */
		while ((begin = _mload_load_attribute(TL_A, mmp, mdl, mn, begin)) != NULL);
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
	} else {
		tlog_notice("can't open '%s': %s", file, strerror(errno));
	}

	mmp_destroy(mmp);
	return r;
}

