/* vim: ft=c ff=unix fenc=utf-8
 * file: src/mempool.c
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "mempool.h"

static void
_clean_node(struct mmp_node *mn)
{
	if (!mn) {
		return;
	}

	switch (mn->type) {
		case MMP_NORMAL:
			break;
		case MMP_GENERIC:
			if (mn->v.gen.free) {
				mn->v.gen.free(mn->v.gen.data);
			}
			break;
		case MMP_FD:
			if (mn->v.fd.close) {
				mn->v.fd.close(mn->v.fd.fd);
			}
			break;
	}
}

struct mmp *
mmp_create()
{
	struct mmp *m = NULL;
	m = calloc(1, sizeof(*m));
	return m;
}

void mmp_clean(struct mmp *m)
{
	struct mmp_node *mn = NULL;
	struct mmp_node *mn_next = NULL;

	if (!m)
		return;

	mn = m->next;
	m->next = NULL;
	for (; mn; mn = mn_next) {
		mn_next = mn->next;
		_clean_node(mn);
		free(mn);
	}
}

void
mmp_destroy(struct mmp *m)
{
	mmp_clean(m);
	free(m);
}

static void
_assign_node(struct mmp *m, struct mmp_node *mn)
{
	mn->protector = mn;

	if ((mn->next = m->next) != NULL) {
		mn->next->prev = mn;
	}
	m->next = mn;

	mn->m = m;
}

void *
mmp_malloc(struct mmp *m, size_t size)
{
	struct mmp_node *mn = NULL;

	if (!(mn = malloc(sizeof(*mn) + size))) {
		return NULL;
	}

	/* init */
	memset(mn, 0, sizeof(*mn));
	mn->type = MMP_NORMAL;
	mn->v.gen.data = (void*)(mn + 1);
	mn->size = size;

	_assign_node(m, mn);

	return mn->v.gen.data;
}

void *mmp_calloc(struct mmp *m, size_t size)
{
	struct mmp_node *mn = NULL;

	if (!(mn = calloc(1, sizeof(*mn) + size))) {
		return NULL;
	}
	mn->type = MMP_NORMAL;
	mn->v.gen.data = (void*)(mn + 1);
	mn->size = size;

	_assign_node(m, mn);

	return mn->v.gen.data;
}

void *
mmp_assign(struct mmp *m, void *data, void(*pfree)(void*))
{
	struct mmp_node *mn = NULL;

	if (!data)
		return NULL;

	if (!(mn = calloc(1, sizeof(*mn)))) {
		pfree(data);
		return NULL;
	}

	mn->type = MMP_GENERIC;
	mn->v.gen.data = data;
	mn->v.gen.free = pfree;
	mn->size = sizeof(void*);

	_assign_node(m, mn);

	return mn->v.gen.data;
}

int mmp_assign_fd(struct mmp *m, int fd, void(*pclose)(int fd))
{
	struct mmp_node *mn = NULL;

	if (fd == -1)
		return false;

	if (!(mn = calloc(1, sizeof(*mn) + sizeof(int)))) {
		pclose(fd);
		return -1;
	}

	mn->type = MMP_FD;
	mn->v.fd.fd = fd;
	mn->v.fd.close = pclose;
	mn->size = sizeof(int);

	_assign_node(m, mn);

	return fd;
}

static struct mmp_node *
_get_header(void *data)
{
	struct mmp_node *mn = NULL;

	mn = ((struct mmp_node*)data) - 1;
	if (mn->protector != mn) {
		return NULL;
	}

	return mn;
}

void *
mmp_realloc(struct mmp *m, void *data, size_t size)
{
	register struct mmp_node *mn_prev = NULL;
	register struct mmp_node *mn_next = NULL;

	struct mmp_node *mn = NULL;
	register void *tmp = NULL;

	if (!data) {
		return mmp_malloc(m, size);
	}

	if (!(mn = _get_header(data))) {
		fprintf(stderr, "get_header failed\n");
		return NULL;
	}

	if (mn->type != MMP_NORMAL) {
		fprintf(stderr, "not a normal\n");
		return NULL;
	}

	/* temporary unlink */
	if ((mn_prev = mn->prev) != NULL) {
		mn->prev->next = mn->next;
	}
	if ((mn_next = mn->next) != NULL) {
		mn_next->prev = mn->prev;
	}
	if ((m = mn->m)->next == mn) {
		m->next = mn->next;
	}

	/* realloc */
	tmp = realloc(mn, sizeof(*mn) + size);
	if (!tmp) {
		return NULL;
	} else {
		mn = tmp;
	}

	mn->v.gen.data = (void*)(mn + 1);
	mn->size = size;

	_assign_node(m, mn);

	return mn->v.gen.data;
}

void
mmp_free(void *data)
{
	struct mmp *m = NULL;
	struct mmp_node *mn = NULL;

	if (!(mn = _get_header(data))) {
		return;
	}
	_clean_node(mn);

	/* unlink */
	m = mn->m;

	if (m->next == mn) {
		m->next = mn->next;
	}

	if (mn->next) {
		mn->next->prev = mn->prev;
	}
	if (mn->prev) {
		mn->prev->next = mn->next;
	}

	free(mn);
}

char*
mmp_strdup(struct mmp *m, char *src)
{
	char *data = NULL;
	size_t len = 0u;

	len = strlen(src);

	if (!(data = mmp_malloc(m, len + 1))) {
		return NULL;
	}

	memcpy(data, src, len);
	data[len] = '\0';

	return data;
}

void*
mmp_memdup(struct mmp *m, void *src, size_t size)
{
	void *data = NULL;

	if (!(data = mmp_malloc(m, size))) {
		return NULL;
	}

	memcpy(data, src, size);
	return data;
}

