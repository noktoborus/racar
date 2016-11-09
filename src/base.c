/* vim: ft=c ff=unix fenc=utf-8
 * file: src/base.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "base.h"

struct rcr_gate *
rcr_add_gate(TL_V, struct rcr *r, const char *name)
{
	unsigned id = 0u;
	struct rcr_gate *p = NULL;
	void *tmp = NULL;

	/* check exists */
	for (id = 0u; id < r->gates; id++) {
		if (!strcmp(r->gate[id].name, name)) {
			tlog("gate '%s' already exists (id: %u)", name, id);
			return NULL;
		}
	}

	/* alloc new perms */
	tmp = realloc(r->aperm, (r->aperms + r->agroups) * sizeof(*r->aperm));
	if (!tmp) {
		tlog("realloc(%d) permissions fail: %s",
				(r->aperms + r->agroups) * sizeof(*r->aperm), strerror(errno));
		return NULL;
	}
	r->aperm = tmp;

	/* update permission info */
	for (id = 0u; id < r->agroups; id++) {
		memset(&r->aperm[r->aperms], 0u, sizeof(*r->aperm));
		if (r->agroup[id].alive) {
			r->aperm[r->aperms].alive = true;
			r->aperm[r->aperms].agroup_id = r->agroup[id].id;
		}

		/* default permissions */
		r->aperm[r->aperms].allow_write = false;
		r->aperm[r->aperms].allow_read = false;
		/* system info */
		r->aperm[r->aperms].gate_id = r->gates;
		r->aperm[r->aperms].id = r->aperms;
		r->aperms++;
	}

	/* alloc new */
	tmp = realloc(r->gate, (r->gates + 1) * sizeof(*r->gate));
	if (!tmp) {
		tlog("realloc(%d) gates fail: %s",
			   ((r->gates + 1) * sizeof(*r->gate)), strerror(errno));
		return NULL;
	}
	r->gate = tmp;

	p = r->gate + r->gates;
	p->id = r->gates;
	p->alive = true;
	snprintf(p->name, sizeof(p->name), "%s", name);

	r->gates++;

	return p;
}

struct rcr_agroup *
rcr_add_agroup(TL_V, struct rcr *r, const char *name)
{
	unsigned id = 0u;
	struct rcr_agroup *p = NULL;
	void *tmp = NULL;
	/* check exists */
	for (id = 0u; id < r->agroups; id++) {
		if (!strcmp(r->gate[id].name, name)) {
			tlog("auth group '%s' already exists (id: %u)", name, id);
			return NULL;
		}
	}

	/* add permissions */
	tmp = realloc(r->aperm, (r->aperms + r->gates) * sizeof(*r->aperm));
	if (!tmp) {
		tlog("relloc(%d) permissions fail: %s",
				(r->aperms + r->agroups) * sizeof(*r->aperm), strerror(errno));
		return NULL;
	}
	r->aperm = tmp;

	for (id = 0u; id < r->gates; id++) {
		memset(&r->aperm[r->aperms], 0u, sizeof(*r->aperm));
		if (r->gate[id].alive) {
			r->aperm[r->aperms].alive = true;
			r->aperm[r->aperms].gate_id = r->gate[id].id;
		}
		/* default permissions */
		r->aperm[r->aperms].allow_write = false;
		r->aperm[r->aperms].allow_read = false;
		/* system */
		r->aperm[r->aperms].gate_id = r->gates;
		r->aperm[r->aperms].id = r->aperms;
		r->aperms++;

	}

	/* alloc new */
	tmp = realloc(r->agroup, (r->agroups + 1) * sizeof(*r->gate));
	if (!tmp) {
		tlog("realloc(%d) groups fail: %s",
				((r->agroups + 1) * sizeof(*r->agroup)), strerror(errno));
		return NULL;
	}

	p = r->agroup + r->agroups;
	p->alive = true;
	r->agroups++;

	return p;
}

void
rcr_print(TL_V, struct rcr *r)
{
	/* TODO: ... */
}

