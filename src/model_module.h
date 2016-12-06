/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model_module.h
 */
#ifndef _SRC_MODEL_MODULE_1479896182_H_
#define _SRC_MODEL_MODULE_1479896182_H_

#include "tlog.h"
#include "model.h"
#include "model_session.h"

#define MODULE_NAME_LEN 80

typedef void (*mm_void)(void);

typedef enum msess_rc (*mm_refresh)(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path);

typedef enum msess_rc (*mm_add)(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path);
typedef enum msess_rc (*mm_del)(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path);

typedef enum msess_rc (*mm_get_int)(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path, long *value);
typedef enum msess_rc (*mm_get_uint)(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path, unsigned long *value);
typedef enum msess_rc (*mm_get_str)(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path, const char **value);

typedef enum msess_rc (*mm_set_int)(struct msess *ms, struct mdl *m, struct mdl_node *node, long value);
typedef enum msess_rc (*mm_set_uint)(struct msess *ms, struct mdl *m, struct mdl_node *node, unsigned long value);
typedef enum msess_rc (*mm_set_str)(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *value);

/* init and destroy modules handle */
typedef void (*mm_destroy)();
typedef mm_destroy (*mm_init)();

enum mm_type {
	MODULE_REFRESH = 1,

	MODULE_ADD,
	MODULE_DEL,

	MODULE_GET,
	MODULE_SET,
};

enum mm_data_type {
	MODULE_STR = 1,
	MODULE_INT,
	MODULE_UINT
};

struct mm_func_link {
	size_t epoch;

	mm_void func;

	enum mm_type mt;
	enum mm_data_type mdt;
	char name[MODULE_NAME_LEN];
};

struct mm_model_ext {
	struct mdl_node model;

	struct mm_func_link refresh;
	struct mm_func_link get;
	struct mm_func_link set;
	struct mm_func_link add;
	struct mm_func_link del;
};

#define MODULE_NONE 0

/* init */
void mm_initialize(TL_V, const char *path);
void mm_deinitialize(TL_V);
/* attach to model tree */
bool mm_attach(TL_V, struct mdl *m);

/* model allocator */
void *mm_model_allocator(void *data);
void mm_model_deallocator(void *ptr, void *data);
void *mm_model_copier(void *src, void *data);

/* set pointer to func */
bool mm_link_func(TL_V, struct mm_func_link *link, enum mm_type mt, const char name[MODULE_NAME_LEN]);
/* safety get func for call by link, return data type in mdt */
mm_void mm_get_func(TL_V, struct mm_func_link *link, enum mm_data_type *mdt);

/* register */
void mm_reg_refresh(const char name[MODULE_NAME_LEN], mm_refresh func);

void mm_reg_add(const char name[MODULE_NAME_LEN], mm_add func);
void mm_reg_del(const char name[MODULE_NAME_LEN], mm_del func);

void mm_reg_get();
void mm_reg_set();

/* modules' utils */
#define mm_log(level, ...) (_mm_log(level, __VA_ARGS__))
void _mm_log(enum tlog_level level, const char *format, ...);

#endif /* _SRC_MODEL_MODULE_1479896182_H_ */

