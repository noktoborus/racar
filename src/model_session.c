/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model_session.c
 */
#include <string.h>

#include "model_session.h"

struct msess *
msess_init(TL_V, struct msess *mss, char username[MSESS_USERNAME_LEN])
{
	struct mmp *mmp = NULL;

	tlog_trace("(mss=%p, username=%p [%s])",
			(void*)mss, (void*)username, username ? username : "");

	if (!(mmp = mmp_create())) {
		return NULL;
	}

	if (mss) {
		memset(mss, 0, sizeof(*mss));
	} else {
		/* allocate new */
		if (!(mss = mmp_calloc(mmp, sizeof(*mss)))) {
			mmp_destroy(mmp);
			return NULL;
		}
	}

	mss->mmp = mmp;

	return mss;
}

void
msess_destroy(TL_V, struct msess *mss)
{
	struct mmp *mmp = mss->mmp;

	tlog_trace("(mss=%p [%s])",
			(void*)mss, mss ? mss->username : "");

	memset(mss, 0u, sizeof(*mss));

	if (mmp) {
		mmp_destroy(mmp);
	}
}

