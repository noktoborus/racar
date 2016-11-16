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
	tmp = realloc(r->group_acl, (r->group_acls + r->agroups) * sizeof(*r->group_acl));
	if (!tmp) {
		tlog("realloc(%d) permissions fail: %s",
				(r->group_acls + r->agroups) * sizeof(*r->group_acl), strerror(errno));
		return NULL;
	}
	r->group_acl = tmp;

	/* update permission info */
	for (id = 0u; id < r->agroups; id++) {
		memset(&r->group_acl[r->group_acls], 0u, sizeof(*r->group_acl));
		if (r->agroup[id].alive) {
			r->group_acl[r->group_acls].alive = true;
			r->group_acl[r->group_acls].agroup_id = r->agroup[id].id;
		}

		/* default permissions */
		r->group_acl[r->group_acls].allow_write = false;
		r->group_acl[r->group_acls].allow_read = false;
		/* system info */
		r->group_acl[r->group_acls].gate_id = r->gates;
		r->group_acl[r->group_acls].id = r->group_acls;
		r->group_acls++;
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
	memset(p, 0u, sizeof(*p));

	p->alive = true;
	p->id = r->gates;
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
	tmp = realloc(r->group_acl, (r->group_acls + r->gates) * sizeof(*r->group_acl));
	if (!tmp) {
		tlog("relloc(%d) permissions fail: %s",
				(r->group_acls + r->agroups) * sizeof(*r->group_acl), strerror(errno));
		return NULL;
	}
	r->group_acl = tmp;

	for (id = 0u; id < r->gates; id++) {
		memset(&r->group_acl[r->group_acls], 0u, sizeof(*r->group_acl));
		if (r->gate[id].alive) {
			r->group_acl[r->group_acls].alive = true;
			r->group_acl[r->group_acls].gate_id = r->gate[id].id;
		}
		/* default permissions */
		r->group_acl[r->group_acls].allow_write = false;
		r->group_acl[r->group_acls].allow_read = false;
		/* system */
		r->group_acl[r->group_acls].gate_id = r->gates;
		r->group_acl[r->group_acls].id = r->group_acls;
		r->group_acls++;

	}

	/* alloc new */
	tmp = realloc(r->agroup, (r->agroups + 1) * sizeof(*r->gate));
	if (!tmp) {
		tlog("realloc(%d) groups fail: %s",
				((r->agroups + 1) * sizeof(*r->agroup)), strerror(errno));
		return NULL;
	}

	p = r->agroup + r->agroups;
	memset(p, 0u, sizeof(*p));

	p->alive = true;
	p->id = r->agroups;
	snprintf(p->name, sizeof(p->name), "%s", name);
	r->agroups++;

	return p;
}

struct rcr_gate *
rcr_get_gate(TL_V, struct rcr *r, unsigned id)
{
	if (id != NOID) {
		if (id >= r->gates) {
			tlog("invalid gate id: %u, max id: %u", id, r->gates);
			return NULL;
		}
	}

	return &r->gate[id];
}

struct rcr_agroup *
rcr_get_agroup(TL_V, struct rcr *r, unsigned id)
{
	if (id == NOID) {
		return NULL;
	}

	if (id  >= r->agroups) {
		tlog("invalid auth group id: %u, max id: %u", id, r->agroups);
		return NULL;
	}

	return &r->agroup[id];
}

struct rcr_agroup_acl *
rcr_get_acl(TL_V, struct rcr *r, unsigned agroup_id, unsigned gate_id)
{
	unsigned id = 0u;

	if (!rcr_get_agroup(TL_A, r, agroup_id)) {
		return NULL;
	}

	if (!rcr_get_gate(TL_A, r, agroup_id)) {
		return NULL;
	}

	for (; id < r->group_acls; id++) {
		if (r->group_acl[id].agroup_id != agroup_id)
			continue;
		if (r->group_acl[id].gate_id != gate_id)
			continue;
		if (!r->group_acl[id].alive) {
			return &r->group_acl[id];
		}
	}

	return NULL;
}

struct rcr_team *
rcr_add_team(TL_V, struct rcr *r, const char *name)
{
	unsigned id = 0u;
	void *tmp = NULL;
	struct rcr_team *p = NULL;
	/* check exists */
	for (id = 0u; id < r->teams; id++) {
		if (!strcmp(r->gate[id].name, name)) {
			tlog("team '%s' already exists (id: %u)", name, id);
			return NULL;
		}
	}

	tmp = realloc(r->team, (r->teams + 1) * sizeof(*r->team));
	if (!tmp) {
		tlog("realloc(%d) teams fail: %s",
				(r->teams + 1) * sizeof(*r->team), strerror(errno));
		return NULL;
	}

	r->team = tmp;
	p = r->team + r->teams;
	memset(p, 0u, sizeof(*p));

	p->alive = true;
	p->id = r->teams;
	snprintf(p->name, sizeof(p->name), "%s", name);
	r->teams++;

	return p;
}

struct rcr_team *
rcr_get_team(TL_V, struct rcr *r, unsigned id)
{
	if (id == NOID)
		return NULL;

	if (id >= r->teams) {
		tlog("invalid team id: %u, max id: %u", id, r->teams);
		return NULL;
	}

	return &r->team[id];
}

static struct rcr_team_attempt *
rcr_team_new_attempt(TL_V, struct rcr *r, struct rcr_team *team)
{
	struct rcr_team_attempt *ta = NULL;
	void *tmp = NULL;

	tmp = realloc(team->attempt, (team->attempts + 1) * sizeof(*team->attempt));
	if (!tmp) {
		tlog("team[%u:%s] realloc(%d) failed: %s",
				team->id, team->name, ta->id,
				(team->attempts + 1) * sizeof(*team->attempt),
				strerror(errno));
		return NULL;
	}
	team->attempt = tmp;

	ta = &team->attempt[team->attempts];
	team->attempts++;

	return ta;
}

bool
rcr_team_start(TL_V, struct rcr *r, unsigned id, time_t time)
{
	struct rcr_team_attempt *ta = NULL;
	struct rcr_team *team = NULL;

	team = rcr_get_team(TL_A, r, id);
	if (!team) {
		tlog("unknown team #%u", id);
		return false;
	}

	if (team->attempts) {
		ta = &team->attempt[team->attempts - 1];
		if (ta->start && !ta->finish) {
			tlog("team[%u:%s] attempt#%u not finished",
					team->id, team->name, ta->id);
			return false;
		} else if (!ta->start) {
			tlog("team[%u:%s] attempt#%u reinit: not started",
					team->id, team->name, ta->id);
			memcpy(&ta->start, &time, sizeof(time));
		}
	}

	if (!ta || team->attempt[team->attempts - 1].finish) {
		/* start new */
		if ((ta = rcr_team_new_attempt(TL_A, r, team)) != NULL) {
			memcpy(&ta->start, &time, sizeof(time));
		}
	}

	if (!ta)
		return false;

	return true;
}

bool
rcr_team_passage(TL_V, struct rcr *r, unsigned id, unsigned gate_id, time_t time, uint32_t penalty)
{
	struct rcr_team_attempt *ta = NULL;
	struct rcr_team *team = NULL;
	struct rcr_gate *gate = NULL;
	struct rcr_team_gate *tg = NULL;
	struct rcr_team_gate *tg_prev = NULL;

	/* get team */
	if (!(team = rcr_get_team(TL_A, r, id))) {
		tlog("unknown team #%u", id);
		return false;
	}

	if (!team->attempts) {
		tlog("team[%u:%s] no attempts", team->id, team->name);
		return false;
	}

	/* get attempt */
	if (!(ta = &team->attempt[team->attempts - 1])) {
		tlog("team[%u:%s] attempt #%u not started",
			   	team->id, team->name, ta->id);
		return false;
	}

	/* check gate */
	if (!(gate = rcr_get_gate(TL_A, r, gate_id))) {
		tlog("team[%u:%s] can't get gate #%u for attempt #%u",
				team->id, team->name, gate_id, ta->id);
		return false;
	}

	/* check gate in attempt */
	for (tg = ta->gate; tg; tg_prev = tg, tg = tg->next) {
		if (tg->gate_id == gate_id) {
			break;
		}
	}

	/* TODO: check order rule (tg_prev) */

	if (!tg) {
		/* alloc new gate */
		tg = calloc(1, sizeof(*tg));
		if (!tg) {
			tlog("team[%u:%s] can't allocate memory for gate #%u, attempt #%u: %s",
					team->id, team->name, gate_id, ta->id, strerror(errno));
			return false;
		}
		tg->gate_id = gate_id;
		/* add to list */
		if (tg_prev) {
			tg_prev->next = tg;
		} else {
			ta->gate = tg;
		}
	}

	memcpy(&tg->time, &time, sizeof(tg->time));
	tg->penalty = penalty;

	return true;
}

bool
rcr_team_finish(TL_V, struct rcr *r, unsigned id, time_t time)
{
	struct rcr_team_attempt *ta = NULL;
	struct rcr_team *team = NULL;

	team = rcr_get_team(TL_A, r, id);
	if (!team) {
		tlog("unknown team #%u", id);
		return false;
	}

	if (!team->attempts) {
		tlog("team[%u:%s] no attempts", team->id, team->name);
		return false;
	}

	ta = &team->attempt[team->attempts - 1];
	if (!ta->start) {
		tlog("team[%u:%s] attempt #%u not started",
			   	team->id, team->name, ta->id);
		return false;
	}

	memcpy(&ta->finish, &time, sizeof(ta->finish));

	return true;
}

void
rcr_print(TL_V, struct rcr *r)
{
	/* TODO: ... */
}

