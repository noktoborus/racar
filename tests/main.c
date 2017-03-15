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
#include "racar/model.h"

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

CHEAT_TEST(mdl_populate,
		int i = 0;
		int ri = 0;
		char path[1024] = {0};
		const char *rpath = NULL;
		struct mdl_node *mn = NULL;
		struct mdl_node *xmn = NULL;
		struct mdl mdl = {0};
		mdl_init(TL_A, &mdl);

		for (i = 0; i < 128; i++) {
			snprintf(path, sizeof(path), "Lorem.ipsum.dolor.sit.amet.consectetur.adipiscing.elit.%d", i);
			cheat_assert((mn = mdl_add_path(TL_A, &mdl, NULL, path)) != NULL);
			/* attach new layer */
			for (ri = 192; ri > 0; ri--) {
				snprintf(path, sizeof(path), "sed.do.eiusmod.tempor.incididunt.ut.labore.et.dolore.magna.aliqua.%d", ri);
				cheat_assert((xmn = mdl_add_path(TL_A, &mdl, mn, path)) != NULL);
			}
		}

		cheat_assert((mn = mdl_get_node(TL_A, &mdl, NULL, "Lorem.ipsum.dolor.sit.amet.consectetur.adipiscing.elit.126")) != NULL);

		mdl_del_node(TL_A, &mdl, mn);

		rpath = mdl_get_path(TL_A, &mdl, NULL, xmn, NULL);
		cheat_assert(!strcmp("Lorem.ipsum.dolor.sit.amet.consectetur.adipiscing.elit.127.sed.do.eiusmod.tempor.incididunt.ut.labore.et.dolore.magna.aliqua.1", rpath));

		mdl_deinit(TL_A, &mdl);
		)

