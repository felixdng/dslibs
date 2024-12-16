#ifndef __CMD_COMM_H
#define __CMD_COMM_H

#if defined(_WINDOWS)
#define FILE_NAME(__x) ( strrchr((__x), '\\') ? (strrchr((__x), '\\') + 1) : (__x) )
#else
#define FILE_NAME(__x) ( strrchr((__x), '/') ? (strrchr((__x), '/') + 1) : (__x) )
#endif

#define PARAM_LEN    32
#define PARAM_CNT    16

typedef struct _params {
	char cmd_params[PARAM_CNT][PARAM_LEN];
	int cmd_num;
} params_t;

typedef struct _cmd_list {
	int id;
	char name[PARAM_LEN];
	int (*handler)(const struct _cmd_list *cmd, const params_t *param);
	void (*help)();
} cmd_list_t;

extern cmd_list_t *pcmdlist;
extern unsigned int cmdlist_num;
extern unsigned int cmdlist_size;

#define CMD_LIST_ELEM(_idx) \
(const cmd_list_t *)((const char *)pcmdlist + _idx * cmdlist_size)

#define pr_output(format, ...) \
printf(format, ##__VA_ARGS__)

#define pr_debug(format, ...) \
printf("[debug] "format, ##__VA_ARGS__)


#endif /* __CMD_COMM_H */
