/* vim: ft=c ff=unix fenc=utf-8
 * file: src/modules/racar.c
 */

#include <racar/base.h>
#include <racar/model.h>
#include <racar/model_module.h>
#include <racar/model_session.h>

/* *** global vars */
static struct rcr rcr;

/* *** library functions */
static enum msess_rc
attempt_gate_add(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path)
{
	struct mdl_node *template = NULL;

	template = mdl_get_node(TL_A, m, NULL, "Template.Gate");
	if (!template) {
		mm_log(TLOG_NOTICE, "Template.Gate not found");
		return MSESS_INT;
	}
	/* copy template */
	/* TODO */
	return MSESS_OK;
}

static enum msess_rc
team_attempt_add(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path)
{
	return MSESS_OK;
}

static enum msess_rc
team_member_add(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path)
{
	return MSESS_OK;
}

static enum msess_rc
team_add(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path)
{
	return MSESS_OK;
}

static enum msess_rc
gate_add(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path)
{
	return MSESS_OK;
}

static enum msess_rc
attempt_gate_del(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path)
{
	return MSESS_OK;
}

static enum msess_rc
attempt_del(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path)
{
	return MSESS_OK;
}

static enum msess_rc
team_member_del(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path)
{
	return MSESS_OK;
}

static enum msess_rc
team_del(struct msess *ms, struct mdl *m, struct mdl_node *node, const char *node_path)
{
		return MSESS_OK;
}

/* *** initialization */
void
module_destroy()
{
	mm_log(TLOG_INFO, "racar destroyed");
	rcr_free_all(TL_A, &rcr);
}

mm_destroy
module_init()
{
	mm_log(TLOG_INFO, "racar initialized");
	mm_reg_add("attempt_gate_add", attempt_gate_add);
	mm_reg_add("team_attempt_add", team_attempt_add);
	mm_reg_add("team_member_add", team_member_add);
	mm_reg_add("team_add", team_add);
	mm_reg_add("gate_add", gate_add);

	mm_reg_del("attempt_gate_del", attempt_gate_del);
	mm_reg_del("attempt_del", attempt_del);
	mm_reg_del("team_member_del", team_member_del);
	mm_reg_del("team_del", team_del);
	return module_destroy;
}

