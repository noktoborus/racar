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

void
begin(TL_V)
{
	struct rcr rcr = {};
	struct mdl mdl = {};

	mdl.mmp = mmp_create();

	mdl_add_path(TL_A, &mdl, NULL, "Gate.1.Path.3");
	mdl_add_path(TL_A, &mdl, NULL, "Gate");
	mdl_add_path(TL_A, &mdl, NULL, "Gate.Start");
	mdl_add_path(TL_A, &mdl, NULL, "Gate.Finish");
	mdl_add_path(TL_A, &mdl, NULL, "User.Manage");
	mdl_add_path(TL_A, &mdl, NULL, "User");
	mdl_add_path(TL_A, &mdl, NULL, "Gate.1");

	mdl_log_tree(TL_A, &mdl, NULL);

	mmp_destroy(mdl.mmp);
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

