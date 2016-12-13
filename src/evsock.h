/* vim: ft=c ff=unix fenc=utf-8
 * file: src/evsock.h
 */
#ifndef _SRC_EVSOCK_1481544106_H_
#define _SRC_EVSOCK_1481544106_H_

#include <stdbool.h>
#include <ev.h>

#include "mempool.h"
#include "tlog.h"

#define EVS_MAX_ADDRESS 80

enum evs_event {
	/* ready to accept new connection */
	EVS_ACCEPT,
	/* connected to remote host */
	EVS_CONNECT,
	/* ready to read */
	EVS_READ,
	/* ready to write (not used?) */
	EVS_WRITE
};

struct evs {
	struct mmp *mmp;
	struct ev_loop *loop;
	bool allocated_loop;
};

struct evs_desc {
	struct evs *evm;
	/* TODO */
};

typedef void(*evs_event_cb_t)(struct evs_desc *d);

/* initialize loop and over structures
 * if loop == NULL, setup called ev_default_loop()
 * returns
 * 	NULL when failed
 * 	pointer to *evm
 * 	pointer to new struct evs when evm is NULL
 */
struct evs *evs_setup(TL_V, struct evs *evm, struct ev_loop *loop);
/* destroy evs loop */
void evs_destroy(TL_V, struct evs *evm);


/*
 * bind socket to address
 * call to event_cb when socket connected or close
 */
struct evs_desc *evs_bind(TL_V, struct evs *evm, const char *address, evs_event_cb_t event_cb);
struct evs_desc *evs_connect(TL_V, struct evs *evm, const char *address, evs_event_cb_t event_cb);

/* accept connection */
struct evs_desc *evs_accept(TL_V, struct evs_desc *d, char address[EVS_MAX_ADDRESS]);

/* set event callback on structure */
bool evs_set_event(TL_V, struct evs_desc *d, evs_event_cb_t event_cb);

bool evs_write(TL_V, struct evs_desc *d, const char *data, size_t len);
bool evs_read(TL_V, struct evs_desc *d, const char *data, size_t len);

/* close descriptor */
bool evs_close(TL_V, struct evs_desc *d);

#endif /* _SRC_EVSOCK_1481544106_H_ */

