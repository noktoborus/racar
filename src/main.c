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

#include "evsock.h"

ssize_t
echo_read_cb(struct evs_desc *d, const char *src, size_t src_size)
{
	return 0u;
}

ssize_t
echo_write_cb(struct evs_desc *d, char *dst, size_t dst_size)
{
	return 0u;
}

void
echo_accept_cb(struct evs_desc *d, struct evs_desc *accept, const char addr[EVS_MAX_ADDRESS])
{
	TL_X;

	tlog_info("echo accepted address: %s", addr);

	evs_set_event(TL_A, d, EVS_WRITE, (evs_event_cb_t)echo_write_cb);
	evs_set_event(TL_A, d, EVS_READ, (evs_event_cb_t)echo_read_cb);
}

void
begin(TL_V)
{
	struct mdl mdl = {0};
	struct evs evs = {0};

	/* init */
	mdl_init(TL_A, &mdl);
	mm_initialize(TL_A, "libmodule-racar.so");
	mm_attach(TL_A, &mdl);

	evs_setup(TL_A, &evs, NULL);

	/* work */
	mload_load(TL_A, &mdl, "etc/model.txt");

	mdl_log_tree(TL_A, &mdl, NULL);

	evs_bind(TL_A, &evs, "127.0.0.1:2234", echo_accept_cb);
	/*evs_connect(TL_A, &evs, "127.0.0.1:2234", NULL);*/

	evs_loop(TL_A, &evs);

	/* deinit */
	mdl_deinit(TL_A, &mdl);
	mm_deinitialize(TL_A);
	evs_destroy(TL_A, &evs);
}

int
main(int argc, char *argv[])
{
    tlog_open();
    begin(TL_A);
    tlog_close();
	return EXIT_SUCCESS;
}

