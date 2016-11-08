/* vim: ft=c ff=unix fenc=utf-8
 * file: src/base.h
 */
#ifndef _SRC_BASE_1478530092_H_
#define _SRC_BASE_1478530092_H_

#include <stdbool.h>

#include "tlog.h"

#define NAME_SIZE 128
#define NOID ((unsigned)-1)

struct rcr_auth_group {
	unsigned id;
	char name[NAME_SIZE];
};

struct rcr_auth_group_perms {
	unsigned id;
	unsigned gate_id;
	unsigned group_id;
	bool write;
	bool read;
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
	unsigned id;
	struct gate *parent;

	char name[NAME_SIZE];
};

struct rcr {
	unsigned gates;
	struct rcr_gate *gate;

	unsigned perms;
	struct rcr_auth_group_perms *perm;

	unsigned groups;
	struct rcr_auth_group *group;

	unsigned teams;
	struct rcr_team *team;

	unsigned members;
	struct rcr_team_member *member;
};

struct rcr_gate *rcr_add_gate(TL_V, struct rcr *r, const char *name);
struct rcr_auth_group *rcr_add_group(TL_V, struct rcr *r, const char *name);

#endif /* _SRC_BASE_1478530092_H_ */

