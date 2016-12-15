/* vim: ft=c ff=unix fenc=utf-8
 * file: src/evsock.c
 */
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "evsock.h"

struct evs *
evs_setup(TL_V, struct evs *evm, struct ev_loop *loop)
{
	struct mmp *mmp = NULL;

	tlog_trace("(evm=%p)", (void*)evm);

	if (!(mmp = mmp_create())) {
		tlog_error("mmp allocation failed: %s", strerror(errno));
		return NULL;
	}

	if (evm) {
		/* initialize passed struct */
		memset(evm, 0, sizeof(*evm));
	} else {
		/* allocate new */
		if (!(evm = mmp_calloc(mmp, sizeof(*evm)))) {
			tlog_error("calloc(%d) failed: %s",
					sizeof(*evm), strerror(errno));
			mmp_destroy(mmp);
			return NULL;
		}
	}

	evm->mmp = mmp;

	if (loop) {
		evm->loop = loop;
	} else {
		evm->loop = ev_default_loop(0);
		evm->allocated_loop = true;
	}

	return evm;
}

void
evs_destroy(TL_V, struct evs *evm)
{
	struct mmp *mmp = NULL;

	tlog_trace("(evm=%p)", (void*)evm);

	mmp = evm->mmp;

	if (evm->allocated_loop) {
		ev_loop_destroy(evm->loop);
	}

	memset(evm, 0u, sizeof(*evm));
	mmp_destroy(mmp);
}


static void
evs_desc_destroy(struct evs_desc *d)
{
	ev_async_stop(d->evm->loop, &d->async);

	if (d->fd != -1) {
		close(d->fd);
	}
}

static void
evs_bind_async_cb(struct ev_loop *loop, struct ev_async *w, int revents)
{
	TL_X;
	char host[EVS_MAX_ADDRESS] = {0};
	char port[6] = "0";
	char *end = NULL;

	struct addrinfo hints = {0};
	struct addrinfo *result = NULL;
	struct addrinfo *rp = NULL;
	int rval = 0;

	struct evs_desc *d = (void*)(((char*)w) - offsetof(struct evs_desc, async));

	tlog_trace("(loop=%p, w=%p, revents=%d)",
			(void*)loop, (void*)w, revents);

	if (d->fd != -1) {
		tlog_notice("invalid fd for (re)bind: %d", d->fd);
		return;
	}

	if (!(end = strrchr(d->addr, ':'))) {
		tlog_notice("not port passed in addr: '%s', using default value: %s", d->addr, port);
	} else {
		snprintf(host, sizeof(host), "%*s", (int)(port - d->addr), d->addr);
		snprintf(port, sizeof(port), "%s", end);
	}

	/* list addresses */
	rval = getaddrinfo(host, port, &hints, &result);
	if (rval != 0) {
		tlog_notice("getaddrinfo(%s, %s) failed: %s",
				host, port, gai_strerror(rval));
		/* TODO: delay... */
		return;
	}
}

struct evs_desc *
evs_bind(TL_V, struct evs *evm, const char *address, evs_accept_cb_t event_cb)
{
	struct evs_desc *d = NULL;

	tlog_trace("(evm=%p, address=%p [%s], event_cb=%p)",
			(void*)evm, (void*)address, (address ? address : ""), (void*)((uintptr_t)event_cb));

	/* allocate */
	if (!(d = mmp_calloc(evm->mmp, sizeof(*d)))) {
		tlog_error("calloc(%d) failed: %s", sizeof(*d), strerror(errno));
		return NULL;
	}

	strncat(d->addr, address, sizeof(d->addr) - 1);
	d->fd = -1;
	d->evm = evm;
	d->type = EVS_SERVER;

	mmp_modify(evm->mmp, (void*)d, (void(*)())evs_desc_destroy);

	/* init events */
	ev_async_init(&d->async, evs_bind_async_cb);
	ev_async_start(evm->loop, &d->async);

	/* begin event */
	ev_async_send(evm->loop, &d->async);
	return d;
}

struct evs_desc *
evs_connect(TL_V, struct evs *evm, const char *address, evs_connect_cb_t event_cb)
{
	tlog_trace("(evm=%p, address=%p [%s], event_cb=%p)",
			(void*)evm, (void*)address, (address ? address : ""), (void*)((uintptr_t)event_cb));

	return NULL;
}

bool
evs_set_event(TL_V, struct evs_desc *d, evs_event_cb_t event_cb)
{
	tlog_trace("(d=%p, event_cb=%p)",
			(void*)d, (void*)((uintptr_t)event_cb));
	return false;
}

bool
evs_set_ready(TL_V, struct evs_desc *d, enum evs_event t)
{
	tlog_trace("(d=%p, t=%d)", (void*)d, t);
	return false;
}

bool
evs_set_busy(TL_V, struct evs_desc *d, enum evs_event t)
{
	tlog_trace("(d=%p, t=%d)", (void*)d, t);
	return false;
}


