/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model_module.c
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "model_module.h"

struct module_root {
	struct mmp *mmp;

	struct module_node *refresh;

	struct module_node *get;
	struct module_node *set;

	struct module_node *add;
	struct module_node *del;

	size_t refresh_count;
	size_t get_count;
	size_t set_count;
	size_t add_count;
	size_t del_count;
};

struct module_node {
	bool active;
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
	mm_deinitialize(TL_A);
	if (!root.mmp) {
		root.mmp = mmp_create();
	}
}

void
mm_deinitialize(TL_V)
{
	if (!root.mmp) {
		mmp_destroy(root.mmp);
	}
	memset(&root, 0, sizeof(root));
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
		tlog("realloc(%d) failed: %s",
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
		tlog("register func failed", NULL);
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

void *
mm_get_func(TL_V, enum module_type mt, const char name[MODULE_NAME_LEN], enum module_data_type *data_type)
{
	size_t i = 0u;
	size_t count = 0;
	struct module_node *mn = NULL;

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
			if (data_type) {
				*data_type = mn[i].data_type;
			}
			return mn[i].func;
		}
	}

	tlog_warn("module '%s' not found for type %s", name, mm_strtype(mt));

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

