/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model.c
 */
#include "model.h"

struct mdl_node *
mdl_add_node(TL_V, struct mdl *m, struct mdl_node *parent, const char *name)
{
	struct mdl_node *mn = NULL;

	if (!(mn = mmp_calloc(m->mmp, sizeof(*mn)))) {
		tlog("calloc(%d) failed: %s", sizeof(*mn), strerror(errno));
		return NULL;
	}

	if (parent) {
		/* attach to parent */
		mn->parent = parent;
		if (parent->child) {
			mn->next = parent->child;
			mn->next->prev = mn;
		}
		parent->child = mn;
	} else {
		/* add to root */
		if ((mn->next = m->child) != NULL) {
			mn->next->prev = mn;
		}
		m->child = mn;
	}

	return mn;
}

void
mdl_del_node(TL_V, struct mdl_node *node)
{
	/* delete tree */
	/* TODO */
}

