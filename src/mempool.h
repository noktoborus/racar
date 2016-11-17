/* vim: ft=c ff=unix fenc=utf-8
 * file: src/mempool.h
 */
#ifndef _SRC_MEMPOOL_1479374773_H_
#define _SRC_MEMPOOL_1479374773_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

enum mmp_node_type {
	/* use in mmp_malloc, mmp_calloc,
	 * mmp_strdup, mmp_memdup
	 */
	MMP_NORMAL = 0,
	/* use in mmp_assign */
	MMP_GENERIC = 1,
	/* use in mmp_assign_fd */
	MMP_FD = 2,
};

struct mmp_node {
	/* pointer to self */
	struct mmp_node *protector;

	struct mmp *m;

	char type;

	size_t size;

	struct mmp_node *prev;
	struct mmp_node *next;

	union {
		struct {
			void *data;
			void (*free)(void*);
		} gen;
		struct {
			int fd;
			void (*close)(int);
		} fd;
	} v;
};

struct mmp {
	size_t elements;
	struct mmp_node *next;
};

/* create new mmp structure */
struct mmp *mmp_create();
/* clean mmp structure */
void mmp_clean(struct mmp *m);
/* clean and destroy mmp structure */
void mmp_destroy(struct mmp *m);

/* allocate memory block */
void *mmp_malloc(struct mmp *m, size_t size);
/* allocate and fill zeros */
void *mmp_calloc(struct mmp *m, size_t size);
/* assign generic data
 * return data or NULL when fail
 * pfree(data) when fail
 */
void *mmp_assign(struct mmp *m, void *data, void(*pfree)(void*));
/* assign file descriptor,
 * return fd or -1 when fail
 * pclose(fd) when fail
 */
int mmp_assign_fd(struct mmp *m, int fd, void(*pclose)(int fd));
/* realloc memory block
 * return new pointer to data or NULL if fail
 * return NULL when data not have a mmp header
 * ->free(data) when fail
 * if node type != MMP_NORMAL, return NULL and not not deallocated
 */
void *mmp_realloc(void *data, size_t size);
/* free data with mmp header */
void mmp_free(void *data);


/*
 * return pointer to string or NULL when fail
 */
char *mmp_strdup(struct mmp *m, char *src);
/*
 * return pointer to data or NULL when fail
 */
void *mmp_memdup(struct mmp *m, void *src, size_t size);

#endif /* _SRC_MEMPOOL_1479374773_H_ */

