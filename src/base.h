/* vim: ft=c ff=unix fenc=utf-8
 * file: src/base.h
 */
#ifndef _SRC_BASE_1478530092_H_
#define _SRC_BASE_1478530092_H_

#define NAME_SIZE 128

struct user {
	char name[NAME_SIZE];
};

struct member {
	char name[NAME_SIZE];
	bool is_captain;
};

struct team {
	char name[NAME_SIZE];
	unsigned members;
	struct member[];
};

struct gate {
	char name[NAME_SIZE];
};

#endif /* _SRC_BASE_1478530092_H_ */

