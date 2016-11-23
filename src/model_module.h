/* vim: ft=c ff=unix fenc=utf-8
 * file: src/model_module.h
 */
#ifndef _SRC_MODEL_MODULE_1479896182_H_
#define _SRC_MODEL_MODULE_1479896182_H_

#include "model.h"

#define MODULE_NAME_LEN 80

typedef void (*module_refresh)(struct mdl *m, struct mdl_node *node);

typedef void (*module_add)(struct mdl *m, struct mdl_node *node);
typedef void (*module_del)(struct mdl *m, struct mdl_node *node);

typedef bool (*module_get_int)(struct mdl *m, struct mdl_node *node, struct mmp *mmp, long *value);
typedef bool (*module_get_uint)(struct mdl *m, struct mdl_node *node, struct mmp *mmp, unsigned long *value);
typedef bool (*module_get_str)(struct mdl *m, struct mdl_node *node, struct mmp *mmp, const char **value);

typedef bool (*module_set_int)(struct mdl *m, struct mdl_node *node, struct mmp *mmp, long value);
typedef bool (*module_set_uint)(struct mdl *m, struct mdl_node *node, struct mmp *mmp, unsigned long value);
typedef bool (*module_set_str)(struct mdl *m, struct mdl_node *node, struct mmp *mmp, const char *value);

enum module_type {
	MODULE_REFRESH = 1,

	MODULE_ADD,
	MODULE_DEL,

	MODULE_GET,
	MODULE_SET,
};

enum module_data_type {
	MODULE_STR = 1,
	MODULE_INT,
	MODULE_UINT
};

#define MODULE_NONE 0

/* init */
void mm_initialize(TL_V);
void mm_deinitialize(TL_V);

/* return func and data type */
void *mm_get_func(TL_V, enum module_type mt, const char name[MODULE_NAME_LEN], enum module_data_type *data_type);

/* register */
void mm_reg_refresh(const char name[MODULE_NAME_LEN], module_refresh *func);

void mm_reg_add(const char name[MODULE_NAME_LEN], module_add *func);
void mm_reg_del(const char name[MODULE_NAME_LEN], module_del *func);

void mm_reg_get();
void mm_reg_set();

/* modules' utils */
void mm_log(const char *format);

#endif /* _SRC_MODEL_MODULE_1479896182_H_ */
