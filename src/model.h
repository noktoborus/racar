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

typedef void*(*mdl_allocator)(void *data);
typedef void(*mdl_deallocator)(void *ptr, void *data);
typedef void*(*mdl_copier)(void *ptr, void *data);

struct mdl {
	mdl_allocator allocator;
	mdl_deallocator deallocator;
	mdl_copier copier;
	void *allocator_data;

	struct mdl_node *child;
	struct mmp *mmp;
};

/* configure */
void mdl_init(TL_V, struct mdl *m);
void mdl_deinit(TL_V, struct mdl *m);

bool mdl_set_allocator(TL_V, struct mdl *m, mdl_allocator al, mdl_deallocator dal, mdl_copier cop, void *allocator_data);

/*
 * add node to *root by path or name
 * add to m->child when root is NULL
 */
struct mdl_node *mdl_add_node(TL_V, struct mdl *m, struct mdl_node *root, const char *name);
struct mdl_node *mdl_add_path(TL_V, struct mdl *m, struct mdl_node *root, const char *path);
/*
 * delete *node with all childs
 */
void mdl_del_node(TL_V, struct mdl *m, struct mdl_node *node);

/*
 * copy and attach node *source to parent with new_name by node or by path
 * attach to m->child when *parent is NULL
 */
struct mdl_node *mdl_copy_node(TL_V, struct mdl *m, struct mdl_node *parent, const char *new_name, struct mdl_node *source);
struct mdl_node *mdl_copy_path(TL_V, struct mdl *m, struct mdl_node *parent, const char *new_name, const char *source);

/* get node by path */
struct mdl_node *mdl_get_node(TL_V, struct mdl *m, struct mdl_node *root, const char *path);
/*
 * return string, allocated in mempool module
 * must be mmp_free'd or mmp_destroy
 * if mmp == NULL, using m->mmp
 *
 */
const char *mdl_get_path(TL_V, struct mdl *m, struct mdl_node *root, struct mdl_node *node, struct mmp *mmp);
/* print tree to log */
void mdl_log_tree(TL_V, struct mdl *m, struct mdl_node *root);

#endif /* _SRC_MODEL_1479373216_H_ */

