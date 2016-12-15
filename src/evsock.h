/* vim: ft=c ff=unix fenc=utf-8
 * file: src/evsock.h
 */
#ifndef _SRC_EVSOCK_1481544106_H_
#define _SRC_EVSOCK_1481544106_H_

#include <stdbool.h>
#include <time.h>
#include <ev.h>

#include "mempool.h"
#include "tlog.h"

#define EVS_MAX_ADDRESS 80

struct evs_desc;

enum evs_event {
	/* ready to accept new connection */
	EVS_ACCEPT = 1,
	/* connected to remote host */
	EVS_CONNECT = 2,
	/* disconnected from host */
	EVS_DISCONNECT = 4,
	/* ready to read */
	EVS_READ = 8,
	/* ready to write */
	EVS_WRITE = 16,
	/* alarm event */
	EVS_ALARM = 32,
	/* error */
	EVS_ERROR = 64,
};

enum evs_type {
	EVS_SERVER, /* bind() */
	EVS_CONNECTION, /* connect() */
	EVS_ACCEPTION, /* listen() */
};

/* ### Event callbacks ### */

/* generic event */
typedef size_t(*evs_event_cb_t)(struct evs_desc *d);
/* copy from socket to callback
 * must be return stored data, if rvalue < src_size then unreaded data stored in buffer
 * and passed on next iteration
 */
typedef size_t(*evs_read_cb_t)(struct evs_desc *d, const char *src, size_t src_size);
/* copy from callback to socket
 * must be return count of writed to *dst bytes
 */
typedef size_t(*evs_write_cb_t)(struct evs_desc *d, char *dst, size_t dst_size);
/* execute on error (read or write) */
typedef void(*evs_error_cb_t)(struct evs_desc *d, enum evs_event e, const char *message);
/* if read/write callback not setted on *accept, then acception canceled */
typedef void(*evs_accept_cb_t)(struct evs_desc *d, struct evs_desc *accept, const char addr[EVS_MAX_ADDRESS]);
/* execute on connect to addr */
typedef void(*evs_connect_cb_t)(struct evs_desc *d, const char addr[EVS_MAX_ADDRESS]);
/* disconnectd from host */
typedef void(*evs_disconnect_cb_t)(struct evs_desc *d, const char addr[EVS_MAX_ADDRESS]);
/* alarm */
typedef void(*evs_alarm_cb_t)(struct evs_desc *d);

struct evs {
	struct mmp *mmp;

	struct ev_loop *loop;
	bool allocated_loop;
};

struct evs_desc {
	struct evs *evm;

	int fd;

	enum evs_type type;
	char addr[EVS_MAX_ADDRESS];

	struct ev_async async;
	struct ev_io io;
	/* TODO */

	evs_read_cb_t read;
	evs_write_cb_t write;
	evs_error_cb_t error;
	evs_accept_cb_t accept;
	evs_connect_cb_t connect;
};

/* ### Initialization functions ### */

/* initialize loop and over structures
 * if loop == NULL, setup called ev_default_loop()
 * returns
 * 	NULL when failed
 * 	pointer to *evm
 * 	pointer to new struct evs when evm is NULL
 */
struct evs *evs_setup(TL_V, struct evs *evm, struct ev_loop *loop);
/* run loop, alternative: ev_run(evm->loop) */
void evs_loop(TL_V, struct evs *evm);
/* destroy evs loop */
void evs_destroy(TL_V, struct evs *evm);
/* close descriptor */
bool evs_close(TL_V, struct evs_desc *d);

/* ### start functions ### */

/* bind socket to address
 * call to event_cb when socket connected or close
 */
struct evs_desc *evs_bind(TL_V, struct evs *evm, const char *address, evs_accept_cb_t event_cb);
/* connect to address
 */
struct evs_desc *evs_connect(TL_V, struct evs *evm, const char *address, evs_connect_cb_t event_cb);

/* ### configuration funcs ### */

/* set event callback */
bool evs_set_event(TL_V, struct evs_desc *d, evs_event_cb_t event_cb);
/* set ready for event, events may be multiplexed (EVS_WRITE | EVS_READ) */
bool evs_set_ready(TL_V, struct evs_desc *d, enum evs_event t);
/* set busy for event, similar to evs_set_ready() */
bool evs_set_busy(TL_V, struct evs_desc *d, enum evs_event t);

#endif /* _SRC_EVSOCK_1481544106_H_ */

