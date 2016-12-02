/* vim: ft=c ff=unix fenc=utf-8
 * file: src/main.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "tlog.h"
#include "base.h"
#include "model.h"
#include "mempool.h"

#include "model_loader.h"
#include "model_module.h"

void
begin(TL_V)
{
	struct rcr rcr = {0};
	struct mdl mdl = {0};

	struct mdl_node *mn_dst = NULL;
	struct mdl_node *mn_src = NULL;

	/* init */
	mdl_init(TL_A, &mdl);
	mm_initialize(TL_A);
	mdl_set_allocator(TL_A, &mdl, mm_model_allocator, mm_model_deallocator, mm_model_copier, mdl.mmp);

	/* work */
	mdl_add_path(TL_A, &mdl, NULL, "Gate.1.Path.3");
	mn_dst = mdl_add_path(TL_A, &mdl, NULL, "Gate");
	mdl_add_path(TL_A, &mdl, NULL, "Gate.Start");
	mdl_add_path(TL_A, &mdl, NULL, "Gate.Finish");

	mn_src = mdl_add_path(TL_A, &mdl, NULL, "User");
	mdl_add_path(TL_A, &mdl, NULL, "User.Manage");

	mdl_add_path(TL_A, &mdl, NULL, "Gate.1");

	mload_load(TL_A, &mdl, "etc/model.txt");

	mdl_copy_node(TL_A, &mdl, mn_dst, "X", mn_src);

	mdl_log_tree(TL_A, &mdl, NULL);

	/* deinit */
	mdl_deinit(TL_A, &mdl);
	mm_deinitialize(TL_A);
	rcr_free_all(TL_A, &rcr);
}

int
main(int argc, char *argv[])
{
    tlog_open();
    begin(TL_A);
    tlog_close();
	return EXIT_SUCCESS;
}

