/*
* command main
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmd_comm.h"

/* global */
cmd_list_t *pcmdlist = NULL;
unsigned int cmdlist_num = 0;
unsigned int cmdlist_size = sizeof(cmd_list_t);

#define CMD_STR        "[CMD INPUT <] "
#define BUFF_SIZE      1024

static struct {
	char cmd_buf[BUFF_SIZE];
	params_t params;
} gparam;
#define gcmd_buf        gparam.cmd_buf
#define gcmd_params     gparam.params.cmd_params
#define gcmd_num        gparam.params.cmd_num

static int parse_params(const char *buf)
{
	gcmd_num = 0;
	int pos = 0;
	const char *ptr = buf;
	while (NULL != ptr) {
		if ('\0' == *ptr || ' ' == *ptr || '\t' == *ptr
			|| '\n' == *ptr || '\r' == *ptr) {
			if (pos > 0) {
				gcmd_params[gcmd_num][pos] = '\0';
				gcmd_num++;
				pos = 0;
			}
			if ('\0' == *ptr)
				break;
		}
		else if (gcmd_num < PARAM_CNT && pos < (PARAM_LEN - 1)) {
			gcmd_params[gcmd_num][pos] = *ptr;
			pos++;
		}
		ptr++;
	}
	return gcmd_num;
}

static void pr_params(int flag)
{
	if (!flag) return;
	int i;
	for (i = 0; i < gcmd_num; ++i) {
		if (0 == i)
			pr_output("[debug info] %s\n", "----> params <----");
		pr_output("    [%02d]: %s\n", i, gcmd_params[i]);
	}
}

static void handle_params(const params_t *params)
{
	if (!params || params->cmd_num <= 0)
		return;
	if (!pcmdlist || cmdlist_num <= 0 || cmdlist_size < sizeof(cmd_list_t))
		return;

	const cmd_list_t *elem = NULL;
	int cmd_idx;
	for (cmd_idx = 0; cmd_idx < cmdlist_num; ++cmd_idx) {
		elem = CMD_LIST_ELEM(cmd_idx);
		if (!strncmp(elem->name, params->cmd_params[0], PARAM_LEN))
			break;
		else {
			int id = strtoul(params->cmd_params[0], NULL, 0);
			if (0 != id && id == elem->id)
				break;
		}
	}
	if (!(cmd_idx < cmdlist_num))
		return;

	//pr_debug("handle params: id(%d), cmd(%s), param num(%d)\n", elem->id, elem->name, params->cmd_num - 1);
	if (elem->handler) elem->handler(elem, params);
}

int main(int argc, char *argv[])
{
	pr_output("%s", CMD_STR);
	while (fgets(gcmd_buf, BUFF_SIZE, stdin)) {
		parse_params(gcmd_buf);
		if (gcmd_num > 0) {
			if (!strncmp(gcmd_params[0], "exit", 4) ||
				!strncmp(gcmd_params[0], "quit", 4)) break;
			pr_params(0);
			handle_params(&gparam.params);
		}
		pr_output("%s", CMD_STR);
	}
	return 0;
}

