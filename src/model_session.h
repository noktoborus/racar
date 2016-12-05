/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model_session.h
 */
#ifndef _SRC_MODEL_SESSION_1480940723_H_
#define _SRC_MODEL_SESSION_1480940723_H_

#include "tlog.h"
#include "model.h"
#include "mempool.h"

#define MSESS_USERNAME_LEN 128

enum msess_result {
	MSESS_OK = 0,
};

struct msess {
	/* auth data, "\0" is anonym */
	char username[MSESS_USERNAME_LEN];
	/* session allocator */
	struct mmp *mmp;
	/* result code */
	enum msess_result rc;
};

/* create or init *mss,
 * return new allocated struct if mss is NULL
 * or return pointer to mss if ok
 * return NULL when init failed
 */
struct msess *msess_init(TL_V, struct msess *mss, char username[MSESS_USERNAME_LEN]);
void msess_destroy(TL_V, struct msess *mss);

/* set result code */
void msess_result(struct msess *mss, enum msess_result rc);

#endif /* _SRC_MODEL_SESSION_1480940723_H_ */

