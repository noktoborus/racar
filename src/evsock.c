/* vim: ft=c ff=unix fenc=utf-8
 * file: src/evsock.c
 */
#include <string.h>
#include <errno.h>

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

struct evs_desc *
evs_bind(TL_V, struct evs *evm, const char *address, evs_accept_cb_t event_cb)
{
	tlog_trace("(evm=%p, address=%p [%s], event_cb=%p)",
			(void*)evm, (void*)address, (address ? address : ""), (void*)((uintptr_t)event_cb));

	return NULL;
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


