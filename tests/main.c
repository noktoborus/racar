/* vim: ft=c ff=unix fenc=utf-8
 * file: tests/main.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <cheat.h>

#include "racar/mempool.h"

CHEAT_TEST(mempool_calloc,
		struct mmp *m = NULL;
		void *p = NULL;
		char gage[1024] = {0};
		cheat_assert((m = mmp_create()) != NULL);
		cheat_assert((p = mmp_calloc(m, sizeof(gage))) != NULL);
		cheat_assert_not(memcmp(gage, p, sizeof(gage)));
		mmp_clean(m);
		mmp_destroy(m);
		)

CHEAT_TEST(mempool_free,
		struct mmp *m = NULL;
		void *v1 = NULL;
		void *v2 = NULL;
		cheat_assert((m = mmp_create()) != NULL);
		cheat_assert(mmp_malloc(m, 1024) != NULL);
		cheat_assert(mmp_malloc(m, 1024) != NULL);
		cheat_assert((v1 = mmp_malloc(m, 1024)) != NULL);
		cheat_assert((v2 = mmp_malloc(m, 1024)) != NULL);
		cheat_assert(mmp_malloc(m, 1024) != NULL);
		cheat_assert(m->elements == 5);
		mmp_free(v1);
		mmp_free(v2);
		cheat_assert(m->elements == 3);
		mmp_destroy(m);
		)

