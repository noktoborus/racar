/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model.c
 */
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "model.h"

struct mdl_node *
mdl_add_node(TL_V, struct mdl *m, struct mdl_node *root, const char *name)
{
	struct mdl_node *mn = NULL;

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

	return mn;
}

static size_t
mdl_iterate_path(TL_V, const char *path, const char **end, char name[MDL_NAME_LEN])
{
	const char *_end = NULL;
	const char *_begin = NULL;

	*name = '\0';

	if (end && *end) {
		_begin = *end;
	} else {
		_begin = path;
	}

	_end = (const char*)strchr(_begin, '.');
	if (!_end) {
		snprintf(name, MDL_NAME_LEN, "%s", _begin);
	} else {
		/* copy name */
		snprintf(name, MDL_NAME_LEN, "%.*s", (int)(_end - _begin), _begin);
		/* skip '.' */
		_end++;
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
	struct mdl_node *mn_next = NULL;

	mn = root;
	while (mdl_iterate_path(TL_A, path, &end, name) != 0u) {
		/* check exists */
		for (mn_next = mn; mn_next; mn_next = mn_next->next) {
			if (!strcmp(mn_next->name, name)) {
				break;
			}
		}
		/* exists, switch to new node */
		if (mn_next) {
			mn = mn_next;
			continue;
		}
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

const char *
mdl_get_path(TL_V, struct mdl *m, struct mdl_node *root, struct mdl_node *node)
{
	return NULL;
}

