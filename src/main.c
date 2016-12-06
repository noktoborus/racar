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

	/* init */
	mdl_init(TL_A, &mdl);
	mm_initialize(TL_A);
	mm_attach(TL_A, &mdl);

	/* work */
	mload_load(TL_A, &mdl, "etc/model.txt");

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

