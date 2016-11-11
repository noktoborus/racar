/* vim: ft=c ff=unix fenc=utf-8
 * file: src/base.h
 */
#ifndef _SRC_BASE_1478530092_H_
#define _SRC_BASE_1478530092_H_

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "tlog.h"

#define NAME_SIZE 128
#define NOID ((unsigned)-1)

struct rcr_agroup {
	bool alive;

	unsigned id;
	char name[NAME_SIZE];
};

struct rcr_agroup_acl {
	bool alive;

	unsigned id;
	unsigned gate_id;
	unsigned agroup_id;

	bool allow_write;
	bool allow_read;
};

struct rcr_team_member {
	unsigned id;
	unsigned team_id;
	char name[NAME_SIZE];
	bool is_captain;
};

struct rcr_team_gate {
	time_t time;
	uint32_t penalty;
	struct rcr_team_gate *next;
};

struct rcr_team_attempt {
	unsigned id;
	unsigned team_id;
	time_t start;
	time_t finish;
	struct rcr_team_attempt *next;
	struct rcr_team_gate *gate;
};

struct rcr_team {
	unsigned id;
	bool alive;
	char name[NAME_SIZE];

	unsigned attempts;
	struct rcr_team_attempt *attempt;
};

struct rcr_gate {
	bool alive;

	unsigned id;
	struct gate *parent;

	char name[NAME_SIZE];
};

struct rcr {
	unsigned gates;
	struct rcr_gate *gate;

	unsigned group_acls;
	struct rcr_agroup_acl *group_acl;

	unsigned agroups;
	struct rcr_agroup *agroup;

	unsigned teams;
	struct rcr_team *team;

	unsigned members;
	struct rcr_team_member *member;
};

struct rcr_gate *rcr_add_gate(TL_V, struct rcr *r, const char *name);
struct rcr_agroup *rcr_add_agroup(TL_V, struct rcr *r, const char *name);

struct rcr_team *rcr_add_team(TL_V, struct rcr *r, const char *name);
struct rcr_team *rcr_add_team_member(TL_V, struct rcr *r, const char *name);

struct rcr_gate *rcr_get_gate(TL_V, struct rcr *r, unsigned id);
struct rcr_agroup *rcr_get_agroup(TL_V, struct rcr *r, unsigned id);
struct rcr_team *rcr_get_team(TL_V, struct rcr *r, unsigned id);
struct rcr_agroup_acl *rcr_get_acl(TL_V, struct rcr *r, unsigned agroup_id, unsigned gate_id);

bool rcr_team_start(TL_V, struct rcr *r, unsigned id, time_t time);
void rcr_team_passage(TL_V, struct rcr *r, unsigned id, unsigned gate_id, time_t time, uint32_t penalty);
void rcr_team_finish(TL_V, struct rcr *r, unsigned id, time_t time);

void rcr_print(TL_V, struct rcr *r);

#endif /* _SRC_BASE_1478530092_H_ */

