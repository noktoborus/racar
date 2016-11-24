/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model_module.c
 */
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <linux/limits.h>
#include "model_module.h"

struct module_root {
	struct mmp *mmp;

	/* function link validating */
	size_t epoch;

	/* function lists */
	struct module_node *refresh;

	struct module_node *get;
	struct module_node *set;

	struct module_node *add;
	struct module_node *del;

	/* counters */
	size_t refresh_count;
	size_t get_count;
	size_t set_count;
	size_t add_count;
	size_t del_count;

	struct module_library *libs;
};

struct module_library {
	char path[PATH_MAX];
	void *handle;
	struct module_library *next;
};

struct module_node {
	bool active;
	struct module_library *lib;
	char name[MODULE_NAME_LEN];
	/* for getter and setter */
	enum module_data_type data_type;
	/* universal pointer */
	void (*func)(void*);
};

static struct module_root root;

void
mm_initialize(TL_V)
{
	/*struct module_library *ml = NULL;*/
	/* fix reinitialization */
	mm_deinitialize(TL_A);
	/* increment epoch */
	if (!(++root.epoch)) {
		/* fix 'zero' epoch */
		root.epoch++;
	}

	tlog_info("model module initialization, epoch: %"PRIuPTR, root.epoch);

	if (!root.mmp) {
		root.mmp = mmp_create();
	}

	/* TODO: load dlopen(...) to module_library */

}

void
mm_deinitialize(TL_V)
{
	size_t epoch = 0u;
	struct module_library *ml = NULL;

	/* save epoch */
	epoch = root.epoch;

	if (root.libs) {
		for (ml = root.libs; ml; ml = ml->next) {
			if (ml->handle) {
				dlclose(ml->handle);
				ml->handle = NULL;
			}
		}
	}
	if (!root.mmp) {
		mmp_destroy(root.mmp);
	}
	memset(&root, 0, sizeof(root));
	/* restore epoch */
	root.epoch = epoch;
}

static struct module_node *
module_add_list(TL_V, struct module_node **list, size_t *count)
{
	struct module_node *mn = NULL;
	size_t _count = *count;
	size_t i = 0u;
	void *tmp;

	for (i = 0u; i < _count; i++) {
		if (!(*list)[i].active) {
			return (*list) + i;
		}
	}

	tmp = mmp_realloc(root.mmp, *list, sizeof(struct module_node) * (_count + 1));
	if (!tmp) {
		tlog_warn("realloc(%d) failed: %s",
				sizeof(struct module_node) * (_count + 1), strerror(errno));
		return NULL;
	}
	list = tmp;

	mn = (*list) + _count;

	_count++;

	*count = _count;

	return mn;
}

static void
module_register_func(TL_V, enum module_type mt, const char name[MODULE_NAME_LEN], void (*func)(void*))
{
	struct module_node *mn = NULL;

	if (!root.mmp) {
		tlog_warn("module_model not initialized", NULL);
		return;
	}

	switch (mt) {
		case MODULE_ADD:
			mn = module_add_list(TL_A, &root.add, &root.add_count);
			break;
		case MODULE_DEL:
			mn = module_add_list(TL_A, &root.del, &root.del_count);
			break;
		case MODULE_GET:
			mn = module_add_list(TL_A, &root.get, &root.get_count);
			break;
		case MODULE_SET:
			mn = module_add_list(TL_A, &root.set, &root.set_count);
			break;
		case MODULE_REFRESH:
			mn = module_add_list(TL_A, &root.refresh, &root.refresh_count);
			break;
	};

	if (!mn) {
		tlog_warn("register func failed", NULL);
		return;
	}

	mn->active = true;
	mn->func = func;
	snprintf(mn->name, sizeof(mn->name), "%s", name);
}

static const char *
mm_strtype(enum module_type mt)
{

	switch (mt) {
		case MODULE_ADD:
			return "add";
			break;
		case MODULE_DEL:
			return "del";
			break;
		case MODULE_GET:
			return "get";
			break;
		case MODULE_SET:
			return "set";
			break;
		case MODULE_REFRESH:
			return "refresh";
			break;
	}
	return NULL;
}

void
mm_reg_refresh(const char name[MODULE_NAME_LEN], module_refresh *func)
{
	module_register_func(TL_A, MODULE_REFRESH, name, (void(*)(void*))func);
}

void
mm_reg_add(const char name[MODULE_NAME_LEN], module_add *func)
{
	module_register_func(TL_A, MODULE_ADD, name, (void(*)(void*))func);
}

void
mm_reg_del(const char name[MODULE_NAME_LEN], module_del *func)
{
	module_register_func(TL_A, MODULE_DEL, name, (void(*)(void*))func);
}

bool
mm_link_func(TL_V, struct module_func_link *link, enum module_type mt, const char name[MODULE_NAME_LEN])
{
	size_t i = 0u;
	size_t count = 0u;
	struct module_node *mn = NULL;

	/* initialization */
	link->epoch = root.epoch;
	link->mdt = MODULE_NONE;
	link->mt = mt;
	snprintf(link->name, sizeof(link->name), "%s", name);
	link->func = NULL;

	/* search */
	switch (mt) {
		case MODULE_ADD:
			mn = root.add;
			count = root.add_count;
			break;
		case MODULE_DEL:
			mn = root.del;
			count = root.del_count;
			break;
		case MODULE_GET:
			mn = root.get;
			count = root.get_count;
			break;
		case MODULE_SET:
			mn = root.set;
			count = root.set_count;
			break;
		case MODULE_REFRESH:
			mn = root.refresh;
			count = root.refresh_count;
			break;
	};

	for (i = 0u; i < count; i++) {
		if (!strncmp(mn[i].name, name, MODULE_NAME_LEN)) {
			link->mdt = mn[i].data_type;
			link->func = mn[i].func;
			link->mt = mt;
			return true;
		}
	}

	tlog_warn("module '%s:%s' not found in %"PRIuPTR" epoch",
		   	mm_strtype(mt), name, root.epoch);

	return false;
}

void *
mm_get_func(TL_V, struct module_func_link *link, enum module_data_type *mdt)
{
	if (!link->epoch) {
		/* not initialized */
		tlog_notice("call to uninitialized link", NULL);
		return NULL;
	}

	if (link->epoch != root.epoch) {
		if (!mm_link_func(TL_A, link, link->mt, link->name)) {
			tlog_warn("link to '%s:%s' not valid in %"PRIuPTR" epoch",
				   	mm_strtype(link->mt), link->name, root.epoch);
			return NULL;
		}
	}

	if (mdt) {
		*mdt = link->mdt;
	}

	return link->func;
}

void
mm_log(enum tlog_level level, const char *format, ...)
{
	/* TODO: add module name to log */
	va_list ap;
	va_start(ap, format);
	_vtlog(level, "model_module", "unknown", format, ap);
	va_end(ap);
}
