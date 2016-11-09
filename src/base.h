/* vim: ft=c ff=unix fenc=utf-8
 * file: src/base.h
 */
#ifndef _SRC_BASE_1478530092_H_
#define _SRC_BASE_1478530092_H_

#include <stdbool.h>

#include "tlog.h"

#define NAME_SIZE 128
#define NOID ((unsigned)-1)

struct rcr_agroup {
	bool alive;

	unsigned id;
	char name[NAME_SIZE];
};

struct rcr_agroup_perms {
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

struct rcr_team {
	unsigned id;
	char name[NAME_SIZE];
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

	unsigned aperms;
	struct rcr_agroup_perms *aperm;

	unsigned agroups;
	struct rcr_agroup *agroup;

	unsigned teams;
	struct rcr_team *team;

	unsigned members;
	struct rcr_team_member *member;
};

struct rcr_gate *rcr_add_gate(TL_V, struct rcr *r, const char *name);
struct rcr_agroup *rcr_add_agroup(TL_V, struct rcr *r, const char *name);

void rcr_print(TL_V, struct rcr *r);

#endif /* _SRC_BASE_1478530092_H_ */

