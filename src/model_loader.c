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

static void
_mload_parse_attribute(TL_V, struct mmp *mmp, struct mdl *mdl, struct mm_model_ext *m, const char *attrib, const char *filename, size_t lineno)
{
	const char *v = NULL;
	tlog_trace("(mmp=%p, mdl=%p, m=%p [%s], attrib=%p [%s], filename=%p [%s], lineno=%zu)",
			(void*)mmp, (void*)mdl, (void*)m, m ? m->model.name : "",
			(void*)attrib, (attrib ? attrib : ""),
			(void*)filename, filename ? filename : "", lineno);

	if (!strncmp(attrib, "get=", 4)) {
		v = attrib + 4;
		mm_link_func(TL_A, &m->get, MODULE_GET, v);
	} else if (!strncmp(attrib, "set=", 4)) {
		v = attrib + 4;
		mm_link_func(TL_A, &m->set, MODULE_SET, v);
	} else if (!strncmp(attrib, "refresh=", 8)) {
		v = attrib + 8;
		mm_link_func(TL_A, &m->refresh, MODULE_REFRESH, v);
	} else if (!strncmp(attrib, "del=", 4)) {
		v = attrib + 4;
		mm_link_func(TL_A, &m->del, MODULE_DEL, v);
	} else if (!strncmp(attrib, "add=", 4)) {
		v = attrib + 4;
		mm_link_func(TL_A, &m->add, MODULE_ADD, v);
	} else {
		tlog_warn("unknown attribute on %s:%zu: '%s'", filename, lineno, attrib);
		return;
	}
}

static char *
_mload_load_attribute(TL_V, struct mmp *mmp, struct mdl *mdl, struct mm_model_ext *m, char *begin, const char *filename, size_t lineno)
{
	char *end = NULL;

	tlog_trace("(mmp=%p, mdl=%p, m=%p [%s], begin=%p [%s], filename=%p [%s], lineno=%zu)",
			(void*)mmp, (void*)mdl, (void*)m, m ? m->model.name : "",
			(void*)begin, (begin ? begin : ""),
			(void*)filename, filename ? filename : "", lineno);

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
	} else {
		*(end++) = '\0';
	}
	_mload_parse_attribute(TL_A, mmp, mdl, m, begin, filename, lineno);

	return end;
}

static bool
_mload_load(TL_V, struct mmp *mmp, struct mdl *mdl, FILE *f, const char *filename)
{
	char line[4096] = {0};
	struct mdl_node *mn = NULL;
	char *begin = NULL;
	char *end = NULL;
	size_t lineno = 0u;

	tlog_trace("(mdl=%p, mdl=%p, f=%p, filename=%p [%s])", (void*)mmp, (void*)mdl, (void*)f,
			(void*)filename, filename ? filename : "");

	while (!feof(f)) {
		lineno++;
		if (!fgets(line, sizeof(line), f)) {
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
			tlog_notice("invalid line %s:%zu: node path not defined", filename, lineno);
			continue;
		}
		/* process arguments */
		while ((begin = _mload_load_attribute(TL_A, mmp, mdl, (struct mm_model_ext*)mn, begin, filename, lineno)) != NULL);
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

	if (mdl->allocator != mm_model_allocator || mdl->deallocator != mm_model_deallocator) {
		tlog_critical("invalid model allocator", NULL);
		return false;
	}

	if (!(mmp = mmp_create())) {
		tlog_warn("Load model from file '%s' failed: %s",
				file, strerror(errno));
		return false;
	}

	f = (FILE*)mmp_assign(mmp, (void*)fopen(file, "r"), (void(*)(void*))fclose);
	if (f) {
		r = _mload_load(TL_A, mmp, mdl, f, file);
	} else {
		tlog_notice("can't open '%s': %s", file, strerror(errno));
	}

	mmp_destroy(mmp);
	return r;
}

