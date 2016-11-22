/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model.c
 */
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "model.h"

struct mdl_node *
mdl_add_node(TL_V, struct mdl *m, struct mdl_node *root, const char *name)
{
	struct mdl_node *mn = NULL;

	tlog_trace("(m=%p, root=%p, name=%p [\"%s\"])",
			(void*)m, (void*)root, (void*)name, name);

	/* check exists */
	for (mn = (root ? root->child : m->child); mn; mn = mn->next) {
		if (!strcmp(mn->name, name)) {
			break;
		}
	}

	if (mn) {
		return mn;
	}

	/* allocate new */
	if (!(mn = mmp_calloc(m->mmp, sizeof(*mn)))) {
		tlog("calloc(%d) failed: %s", sizeof(*mn), strerror(errno));
		return NULL;
	}

	if (root) {
		/* attach to parent */
		mn->parent = root;
		if (root->child) {
			mn->next = root->child;
			mn->next->prev = mn;
		}
		root->child = mn;
	} else {
		/* add to root */
		if ((mn->next = m->child) != NULL) {
			mn->next->prev = mn;
		}
		m->child = mn;
	}

	mn->name_len = snprintf(mn->name, sizeof(mn->name), "%s", name);

	return mn;
}

static size_t
mdl_iterate_path(TL_V, const char *path, const char **end, char name[MDL_NAME_LEN])
{
	const char *_end = NULL;
	const char *_begin = NULL;

	tlog_trace("(path=%p [\"%s\"], end=%p [*end=%p], name=\"%s\")",
			(void*)path, path, (void*)end, (end ? *end : NULL), name);

	*name = '\0';

	if (end && *end) {
		_begin = *end;
	} else {
		_begin = path;
	}

	if (*_begin == '.') {
		_begin++;
	} else if (!*_begin) {
		return 0u;
	}

	_end = (const char*)strchr(_begin, '.');
	if (!_end) {
		_end = (_begin + snprintf(name, MDL_NAME_LEN, "%s", _begin));
	} else {
		/* copy name */
		snprintf(name, MDL_NAME_LEN, "%.*s", (int)(_end - _begin), _begin);
	}

	if (end) {
		*end = _end;
	}
	return strlen(name);
}

struct mdl_node *
mdl_add_path(TL_V, struct mdl *m, struct mdl_node *root, const char *path)
{
	const char *end = NULL;
	char name[MDL_NAME_LEN] = {};
	struct mdl_node *mn = NULL;

	tlog_trace("(m=%p, root=%p, path=\"%s\")", (void*)m, (void*)root, path);

	mn = root;
	while (mdl_iterate_path(TL_A, path, &end, name) != 0u) {
		/* add to tree and switch to new node */
		if (!(mn = mdl_add_node(TL_A, m, mn, name))) {
			break;
		}
	}

	return mn;
}

void
mdl_del_node(TL_V, struct mdl_node *node)
{
	/* delete tree */
	if (!node)
		return;

	tlog_trace("(node=%p)", (void*)node);

	/* unlink */
	if (node->parent != NULL) {
		if (node->parent->child == node) {
			node->parent->child = node->next;
		}
	}
	if (node->next) {
		node->next->prev = node->prev;
	}
	if (node->prev) {
		node->prev->next = node->next;
	}

	/* delete childs */
	while (node->child) {
		mdl_del_node(TL_A, node->child);
	}

	free(node);
}

struct mdl_node *
mdl_get_node(TL_V, struct mdl *m, struct mdl_node *root, const char *path)
{
	const char *end = NULL;
	char name[MDL_NAME_LEN] = {};
	struct mdl_node *mn = NULL;

	tlog_trace("(m=%p, root=%p, path=\"%s\")", (void*)m, (void*)root, path);

	if (root) {
		/* search in root children's */
		mn = root;
		mn = mn->child;
	} else {
		/* or search in main root */
		mn = m->child;
	}

	while (mdl_iterate_path(TL_A, path, &end, name) != 0u) {
		/* deep, in childrens */
		if (!mn) {
			if (root) {
				/* go to next of root */
				mn = root;
				mn = mn->child;
			} else if (m->child) {
				/* start of root */
				mn = m->child;
			}
		}
		/* search in sibling */
		for (; mn; mn = mn->next) {
			if (!strcmp(mn->name, name)) {
				break;
			}
		}
		/* no node, exit */
		if (!mn) {
			break;
		}
	}

	return mn;
}


static size_t
mdl_str_reverse(char *buffer, size_t len)
{
	register char *begin = NULL;
	register char *end = NULL;
	register char tmp;

	begin = buffer;
	end = buffer + len;

	for (; begin < end; begin++, end--) {
		tmp = *begin;
		*begin = *end;
		*end = tmp;
	}

	return len;
}

/*
 * return number of copied bytes
 */
static size_t
mdl_strcpy_reverse(char *dst, const char *src, size_t len)
{
	register size_t i = 0u;
	for (; i <= len; i++) {
		dst[i] = src[len - i];
	}
	return i;
}

const char *
mdl_get_path(TL_V, struct mdl *m, struct mdl_node *root, struct mdl_node *node, struct mmp *mmp)
{
	size_t size = 0u;
	size_t path_size = 0u;
	struct mdl_node *mn = NULL;
	char *path = NULL;

	tlog_trace("(m=%p, root=%p, node=%p, mmp=%p)",
			(void*)m, (void*)root, (void*)node, (void*)mmp);

	/* calc buffer size */
	for (path_size = 0u, mn = node; mn; mn = mn->parent) {
		path_size += (mn->name_len + 1);
		if (mn == root)
		   break;
	}

	/* allocate buffer */
	path_size += 2;
	path = mmp_calloc((mmp ? mmp : m->mmp), path_size);
	if (!path) {
		tlog("calloc(%d) failed: %s", path_size, strerror(errno));
		return NULL;
	}

	/* copy name */
	for (size = 0u, mn = node; mn; mn = mn->parent) {
		path[size++] = '.';
		size += mdl_strcpy_reverse(&path[size], mn->name, mn->name_len - 1);
		if (mn == root)
			break;
	}

	size = mdl_str_reverse(path, size - 1);
	path[size] = '\0';

	return path;
}


static void
mdl_log_dive_deep(TL_V, struct mdl *m, struct mdl_node *node, unsigned level, unsigned child)
{
	const char *path = NULL;
	if (!level) {
		tlog_info("Model tree:", NULL);
		mdl_log_dive_deep(TL_A, m, node, level + 1, 1u);
		return;
	}

	path = mdl_get_path(TL_A, m, NULL, node, NULL);
	tlog_info("%*s %u. %s [%s]", 3 * (level - 1), "", child, node->name, path);
	mmp_free((void*)path);

	if (node->child) {
		mdl_log_dive_deep(TL_A, m, node->child, level + 1, 1u);
	}

	if (node->next) {
		mdl_log_dive_deep(TL_A, m, node->next, level, child + 1);
	}
}

void
mdl_log_tree(TL_V, struct mdl *m, struct mdl_node *root)
{
	tlog_trace("(m=%p, root=%p [%s])",
			(void*)m, (void*)root, root ? root->name : "");
	if (!root) {
		root = m->child;
	}

	mdl_log_dive_deep(TL_A, m, root, 0u, 0u);
}

