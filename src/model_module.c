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

struct mm_root {
	struct mmp *mmp;

	/* function link validating */
	size_t epoch;

	/* function lists */
	struct mm_node *refresh;

	struct mm_node *get;
	struct mm_node *set;

	struct mm_node *add;
	struct mm_node *del;

	/* counters */
	size_t refresh_count;
	size_t get_count;
	size_t set_count;
	size_t add_count;
	size_t del_count;

	struct mm_library *libs;
};

struct mm_library {
	char path[PATH_MAX];
	void *handle;

	struct mm_library *next;

	void (*init)();
	void (*destroy)();
};

struct mm_node {
	bool active;
	char name[MODULE_NAME_LEN];
	/* for getter and setter */
	enum mm_data_type data_type;
	/* universal pointer */
	mm_void func;
};

static struct mm_root root;

void *
mm_model_allocator(void *data)
{
	return mmp_calloc(data, sizeof(struct mm_model_ext));
}

void
mm_model_deallocator(void *ptr, void *data)
{
	mmp_free(ptr);
}

void *
mm_model_copier(void *src, void *data)
{
	void *ptr = NULL;
	if ((ptr = mmp_malloc(data, sizeof(struct mm_model_ext))) != NULL) {
		memcpy(ptr, src, sizeof(struct mdl_node));
	}

	return ptr;
}

static void
mm_model_printer(struct mm_model_ext *me, const char *path, size_t level, size_t child_no)
{
	TL_X;
	char buf[1152] = {0};
	size_t len = 0u;

	if (!me) {
		tlog_info("Model tree (epoch: %zu, registered funcs: %zu): ",
				root.epoch,
				root.refresh_count +
					root.get_count +
					root.set_count +
					root.add_count +
					root.del_count);
		return;
	}

	if (me->refresh.mt != MODULE_NONE) {
		len += snprintf(buf + len, sizeof(buf) - len,
				"refresh=%s:%zu",
				me->refresh.name, me->refresh.epoch);
	}

	if (me->get.mt != MODULE_NONE) {
		len += snprintf(buf + len, sizeof(buf) - len,
				"%sget=%s:%zu", *buf ? " ,": "",
				me->get.name, me->get.epoch);
	}

	if (me->set.mt != MODULE_NONE) {
		len += snprintf(buf + len, sizeof(buf) - len,
				"%sset=%s:%zu",*buf ? " ,": "",
				me->set.name, me->set.epoch);
	}

	if (me->add.mt != MODULE_NONE) {
		len += snprintf(buf + len, sizeof(buf) - len,
				"%sadd=%s:%zu", *buf ? " ,": "",
				me->add.name, me->add.epoch);
	}

	if (me->del.mt != MODULE_NONE) {
		len += snprintf(buf + len, sizeof(buf) - len,
				"%sdel=%s:%zu", *buf ? " ,": "",
				me->del.name, me->del.epoch);
	}

	tlog_info("%*s %u. %s [%s]", 3 * (level - 1), "",
		   	child_no, me->model.name, path);
	if (*buf)
		tlog_info("%*s {%s}", 3 * (level - 1), "", buf);
}

void
mm_initialize(TL_V, const char *path)
{
	void *handle = NULL;
	mm_init init = NULL;
	struct mm_library *ml = NULL;
	/* fix reinitialization */
	mm_deinitialize(TL_A);
	/* increment epoch */
	if (!(++root.epoch)) {
		/* fix 'zero' epoch */
		root.epoch++;
	}

	tlog_info("model module initialization, epoch: %zu", root.epoch);

	if (!root.mmp) {
		root.mmp = mmp_create();
	}

	tlog_info("trying load modules from '%s'", path);
	if (!(handle = dlopen(path, RTLD_LAZY))) {
		tlog_notice("dlopen(%s) failed: %s", path, dlerror());
	}

	ml = mmp_calloc(root.mmp, sizeof(*ml));
	if (!ml) {
		tlog("calloc(%d) failed: %s", sizeof(*ml), strerror(errno));
		dlclose(handle);
		return;
	}

	init = (mm_init)((uintptr_t)dlsym(handle, "module_init"));
	if (init) {
		ml->destroy = (*init)();
	}
	snprintf(ml->path, sizeof(ml->path), "%s", path);
	ml->handle = handle;
	ml->next = root.libs;
	root.libs = ml;
}

void
mm_deinitialize(TL_V)
{
	size_t epoch = 0u;
	struct mm_library *ml = NULL;

	tlog_trace("()", NULL);

	/* save epoch */
	epoch = root.epoch;

	if (root.libs) {
		for (ml = root.libs; ml; ml = ml->next) {
			tlog_info("unload library '%s'", ml->path);
			if (ml->destroy) {
				ml->destroy();
			}
			if (ml->handle) {
				dlclose(ml->handle);
				ml->handle = NULL;
			}
		}
	}
	if (root.mmp) {
		mmp_destroy(root.mmp);
	}
	memset(&root, 0, sizeof(root));
	/* restore epoch */
	root.epoch = epoch;
}

bool
mm_attach(TL_V, struct mdl *m)
{
	tlog_trace("(m=%p)", (void*)m);

	if (!mdl_set_allocator(TL_A,
				m, mm_model_allocator, mm_model_deallocator, mm_model_copier, m->mmp)) {
		return false;
	}

	m->printer = (mdl_printer)mm_model_printer;

	return true;
}

static struct mm_node *
mm_add_list(TL_V, struct mm_node **list, size_t *count)
{
	struct mm_node *mn = NULL;
	size_t _count = *count;
	size_t i = 0u;
	void *tmp;

	tlog_trace("(list=%p [%p], count=%p [%zu])",
			(void*)list,
			(void*)(list ? *list : NULL),
			(void*)count,
			count ? *count : 0u);

	/* find empty nodes */
	for (i = 0u; i < _count; i++) {
		if (!(*list)[i].active) {
			return (*list) + i;
		}
	}

	/* allocate new */
	tmp = mmp_realloc(root.mmp, *list, sizeof(struct mm_node) * (_count + 1));
	if (!tmp) {
		tlog_warn("realloc(%d) failed: %s",
				sizeof(struct mm_node) * (_count + 1), strerror(errno));
		return NULL;
	}
	*list = tmp;

	mn = (*list) + _count;
	*count = (_count + 1);

	memset(mn, 0u, sizeof(*mn));

	return mn;
}

static void
mm_register_func(TL_V, enum mm_type mt, const char name[MODULE_NAME_LEN], mm_void func)
{
	struct mm_node *mn = NULL;

	tlog_trace("(mt=%d, name=%p [%s], func=%p)",
			mt, (void*)name, name ? name : "", (void*)((uintptr_t)func));

	if (!root.mmp) {
		tlog_warn("mm_model not initialized", NULL);
		return;
	}

	switch (mt) {
		case MODULE_ADD:
			mn = mm_add_list(TL_A, &root.add, &root.add_count);
			break;
		case MODULE_DEL:
			mn = mm_add_list(TL_A, &root.del, &root.del_count);
			break;
		case MODULE_GET:
			mn = mm_add_list(TL_A, &root.get, &root.get_count);
			break;
		case MODULE_SET:
			mn = mm_add_list(TL_A, &root.set, &root.set_count);
			break;
		case MODULE_REFRESH:
			mn = mm_add_list(TL_A, &root.refresh, &root.refresh_count);
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
mm_strtype(enum mm_type mt)
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
mm_reg_refresh(const char name[MODULE_NAME_LEN], mm_refresh func)
{
	mm_register_func(TL_A, MODULE_REFRESH, name, (mm_void)func);
}

void
mm_reg_add(const char name[MODULE_NAME_LEN], mm_add func)
{
	mm_register_func(TL_A, MODULE_ADD, name, (mm_void)func);
}

void
mm_reg_del(const char name[MODULE_NAME_LEN], mm_del func)
{
	mm_register_func(TL_A, MODULE_DEL, name, (mm_void)func);
}

bool
mm_link_func(TL_V, struct mm_func_link *link, enum mm_type mt, const char name[MODULE_NAME_LEN])
{
	size_t i = 0u;
	size_t count = 0u;
	struct mm_node *mn = NULL;

	tlog_trace("(link=%p, mt=%d, name=%p [%s])",
			(void*)link, mt, name, (name ? name : ""));

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

	tlog_warn("module '%s:%s' not found in %zu epoch",
		   	mm_strtype(mt), name, root.epoch);

	return false;
}

mm_void
mm_get_func(TL_V, struct mm_func_link *link, enum mm_data_type *mdt)
{
	if (!link->epoch) {
		/* not initialized */
		tlog_notice("call to uninitialized link", NULL);
		return NULL;
	}

	if (link->epoch != root.epoch) {
		if (!mm_link_func(TL_A, link, link->mt, link->name)) {
			tlog_warn("link to '%s:%s' not valid in %zu epoch",
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
