/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model.h
 */
#ifndef _SRC_MODEL_1479373216_H_
#define _SRC_MODEL_1479373216_H_

#include "tlog.h"
#include "mempool.h"

#define MDL_NAME_LEN 128

struct mdl_node {
	/* level */
	struct mdl_node *parent;
	struct mdl_node *child;
	/* sibling */
	struct mdl_node *next;
	struct mdl_node *prev;

	char name[MDL_NAME_LEN];
	size_t name_len;
};

struct mdl {
	struct mdl_node *child;
	struct mmp *mmp;
};

struct mdl_node *mdl_add_node(TL_V, struct mdl *m, struct mdl_node *root, const char *name);
struct mdl_node *mdl_add_path(TL_V, struct mdl *m, struct mdl_node *root, const char *path);
void mdl_del_node(TL_V, struct mdl_node *node);

struct mdl_node *mdl_get_node(TL_V, struct mdl *m, struct mdl_node *root, const char *path);
/*
 * return string, allocated in mempool module
 * must be mmp_free'd or mmp_destroy
 * if mmp == NULL, using m->mmp
 *
 */
const char *mdl_get_path(TL_V, struct mdl *m, struct mdl_node *root, struct mdl_node *node, struct mmp *mmp);

#endif /* _SRC_MODEL_1479373216_H_ */
