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

