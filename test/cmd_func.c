#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "cmd_comm.h"

#include "list_cntr.h"
#include "hash_cntr.h"
#include "rbtree_cntr.h"
#include "common.h"

typedef struct {
	cmd_list_t base;
	char *file_name;
} my_cmdlist_t;

static int print_help(const cmd_list_t *pcmd, const params_t *param)
{
	int cmd_idx;

	if (param && 2 == param->cmd_num) {
		for (cmd_idx = 0; cmd_idx < cmdlist_num; ++cmd_idx) {
			const cmd_list_t *elem = CMD_LIST_ELEM(cmd_idx);
			if (!strncmp(elem->name, param->cmd_params[1], PARAM_LEN))
				break;
			else {
				int id = strtoul(param->cmd_params[1], NULL, 0);
				if (0 != id && id == elem->id)
					break;
			}
		}
		if (cmd_idx < cmdlist_num) {
			const cmd_list_t *elem = CMD_LIST_ELEM(cmd_idx);
			pr_output("\ncommand:\n");
			pr_output("  %-4d:    %s\n", elem->id, elem->name);
			if (elem->help) elem->help();
			return 0;
		}
	}

	if (cmdlist_num > 0)
		pr_output("\ncommand list:\n");
	for (cmd_idx = 0; cmd_idx < cmdlist_num; ++cmd_idx) {
		const cmd_list_t *elem = CMD_LIST_ELEM(cmd_idx);
		pr_output("  %-4d:    %s\n", elem->id, elem->name);
	}

	int flag = 0;
	for (cmd_idx = 0; cmd_idx < cmdlist_num; ++cmd_idx) {
		const cmd_list_t *elem = CMD_LIST_ELEM(cmd_idx);
		if (elem->help) {
			if (0 == flag) {
				pr_output("command params:\n");
				flag = 1;
			}
			elem->help();
		}
	}
	return 0;
}

typedef struct {
	int id;
	float score;
	int rsv[10];
} test_obj;

#define OBJ_NUM    (10000000)
#define BUCKET_NUM (1000)
static size_t gobj_num = OBJ_NUM;
static size_t gbucket_num = BUCKET_NUM;

int test_obj_getkey(const void *obj)
{
	if (!obj)
		return -1;
	test_obj *pobj = (test_obj *)obj;
	return pobj->id;
}

void test_obj_show(const void *obj, size_t size, const char *ptitle)
{
	if (!obj || size != sizeof(test_obj))
		return;
	test_obj *pobj = (test_obj *)obj;
	if (ptitle && strlen(ptitle) > 0) {
		printf(">%s\n", ptitle);
	}
	printf("  id: %04d, score: %.02f\n", pobj->id, pobj->score);
}

static int test_obj_cmp(const void *obj1, const void *obj2, size_t size)
{
	if (NULL == obj1 || NULL == obj2 || sizeof(test_obj) != size)
		return 0;
	test_obj *p1 = (test_obj *)obj1;
	test_obj *p2 = (test_obj *)obj2;
	return (p1->id < p2->id) ? -1 :
			(p1->id > p2->id) ? 1 : 0;
}

static size_t test_obj_calc(const void *obj, size_t size)
{
	size_t ret = -1;
	if (NULL == obj || sizeof(test_obj) != size)
		return -1;
	if (gbucket_num > 0) {
		test_obj *p = (test_obj *)obj;
		ret = p->id % gbucket_num;
	}
	return ret;
}


static int list_test(const cmd_list_t *pcmd, const params_t *param)
{
#if 0
	pr_debug("%s() enter.\n", __func__);
	for (int i = 0; i < param->cmd_num; ++i) {
		if (0 == i)
			pr_debug("num  param\n");
		pr_debug("%-4d %s\n", i, param->cmd_params[i]);
		if (i == (param->cmd_num - 1))
			pr_output("\n");
	}
#endif
	int ret;
	list_cntr hdl;
	test_obj tmp_obj;
	int i;
	long obj_num = 0;
	if (2 == param->cmd_num) {
		char *endptr = NULL;
		unsigned long val = strtoul(param->cmd_params[1], &endptr, 0);
		if (*endptr == '\0') obj_num = val;
	}
	gobj_num = (obj_num > 0 ? obj_num : OBJ_NUM);

	/* init */
	if (0 != list_cntr_create(&hdl, sizeof(test_obj), gobj_num, test_obj_cmp, ORDER_UP)) {
		return -1;
	}

	//list_cntr_dump(&hdl);
	//printf("after create.\n");
	//getchar();

//RUN_TIME_START
	for (i = 0; i < gobj_num; ++i) {
		float _h = rand() % 100;
		float _l = rand() % 100;
		test_obj obj = {i, _h + _l * 0.01};
		list_cntr_insert(&hdl, &obj, sizeof(obj));

		//list_cntr_dump(&hdl);
		//printf("after insert.\n");
		//getchar();
	}
//RUN_TIME_END

	//list_cntr_show(&hdl, test_obj_show, &tmp_obj, sizeof(tmp_obj));
	//getchar();

	printf("\nremove\n");
	tmp_obj.id = gobj_num / 2;
	RUN_TIME_START
	list_cntr_remove(&hdl, &tmp_obj, sizeof(tmp_obj));
	RUN_TIME_END

	printf("\ninsert\n");
	tmp_obj.id = gobj_num / 2;
	RUN_TIME_START
	list_cntr_insert(&hdl, &tmp_obj, sizeof(tmp_obj));
	RUN_TIME_END

	//list_cntr_show(&hdl, test_obj_show, &tmp_obj, sizeof(tmp_obj));
	//getchar();

	printf("\nsearch\n");
	tmp_obj.id = gobj_num / 2;
	RUN_TIME_START
	ret = list_cntr_search(&hdl, &tmp_obj, sizeof(tmp_obj), &tmp_obj);
	RUN_TIME_END
	if (0 == ret) test_obj_show(&tmp_obj, sizeof(tmp_obj), NULL);
	else printf("Not found!\n");

#if 0
	tmp_obj.id = gobj_num / 2 + 1;
	RUN_TIME_START
	ret = list_cntr_search(&hdl, &tmp_obj, sizeof(tmp_obj), &tmp_obj);
	RUN_TIME_END
	if (0 == ret) test_obj_show(&tmp_obj, sizeof(tmp_obj), NULL);
	else printf("Not found!\n");

	tmp_obj.id = gobj_num / 2 - 1;
	RUN_TIME_START
	ret = list_cntr_search(&hdl, &tmp_obj, sizeof(tmp_obj), &tmp_obj);
	RUN_TIME_END
	if (0 == ret) test_obj_show(&tmp_obj, sizeof(tmp_obj), NULL);
	else printf("Not found!\n");
#endif

	list_cntr_dump(&hdl);
	list_cntr_destroy(&hdl);
	return 0;
}

static void list_test_h(void)
{
	pr_output("  [11]list-test : The list test command.\n");
	pr_output("    '11|list-test [<obj_num>]'\n");
	//pr_output("    cmd: 1 - insert\n");
}

static int hash_test(const cmd_list_t *pcmd, const params_t *param)
{
#if 0
	pr_debug("%s() enter.\n", __func__);
	for (int i = 0; i < param->cmd_num; ++i) {
		if (0 == i)
			pr_debug("num  param\n");
		pr_debug("%-4d %s\n", i, param->cmd_params[i]);
		if (i == (param->cmd_num - 1))
			pr_output("\n");
	}
#endif
	int ret;
	hash_cntr hdl;
	test_obj tmp_obj;
	long obj_num = 0;
	long bucket_num = 0;
	int param_idx = 1;
	while (param_idx < param->cmd_num) {
		if (0 == strncmp("objnum=", param->cmd_params[param_idx], 7)) {
			char *endptr = NULL;
			unsigned long val = strtoul(&param->cmd_params[param_idx][7], &endptr, 0);
			if (*endptr == '\0') obj_num = val;
		}
		else if (0 == strncmp("bucket=", param->cmd_params[param_idx], 7)) {
			char *endptr = NULL;
			unsigned long val = strtoul(&param->cmd_params[param_idx][7], &endptr, 0);
			if (*endptr == '\0') bucket_num = val;
		}

		param_idx++;
	}
	gobj_num = (obj_num > 0 ? obj_num : OBJ_NUM);
	gbucket_num = (bucket_num > 0 ? bucket_num : BUCKET_NUM);

	/* init */
	if (0 != hash_cntr_create(&hdl, sizeof(test_obj), gobj_num, gbucket_num, test_obj_calc, test_obj_cmp)) {
		return -1;
	}

	//hash_cntr_dump(&hdl);
	//printf("after create.\n");
	//getchar();

	for (int i = 0; i < gobj_num; ++i) {
		float _h = rand() % 100;
		float _l = rand() % 100;
		test_obj obj = {i, _h + _l * 0.01};
		hash_cntr_insert(&hdl, &obj, sizeof(test_obj));

		//hash_cntr_dump(&hdl);
		//printf("after insert.\n");
		//getchar();
	}

	for (int i = 0; i < gobj_num; ++i) {
		if (!(2 == i || 5 == i)) continue;
		test_obj obj = {.id = i};
		printf("\nremove\n");
		RUN_TIME_START
		hash_cntr_remove(&hdl, &obj, sizeof(obj));
		RUN_TIME_END

		//hash_cntr_dump(&hdl);
		//printf("after remove.\n");
		//getchar();
	}

	for (int i = 0; i < gobj_num; ++i) {
		if (!(2 == i || 5 == i)) continue;
		test_obj obj = {.id = i};
		printf("\ninsert\n");
		RUN_TIME_START
		hash_cntr_insert(&hdl, &obj, sizeof(obj));
		RUN_TIME_END

		//hash_cntr_dump(&hdl);
		//printf("after insert2.\n");
		//getchar();
	}

	printf("\nsearch\n");
	tmp_obj.id = gobj_num / 2;
	RUN_TIME_START
	ret = hash_cntr_search(&hdl, &tmp_obj, sizeof(tmp_obj), &tmp_obj);
	RUN_TIME_END
	if (0 == ret) test_obj_show(&tmp_obj, sizeof(tmp_obj), NULL);
	else printf("Not found!\n");

	hash_cntr_destroy(&hdl);
	return 0;
}

static void hash_test_h(void)
{
	pr_output("  [21]hash-test : The hash test command.\n");
	pr_output("    '21|hash-test [objnum=<num>] [bucket=<num>]'\n");
}

static int rbtree_test(const cmd_list_t *pcmd, const params_t *param)
{
#if 0
	pr_debug("%s() enter.\n", __func__);
	for (int i = 0; i < param->cmd_num; ++i) {
		if (0 == i)
			pr_debug("num  param\n");
		pr_debug("%-4d %s\n", i, param->cmd_params[i]);
		if (i == (param->cmd_num - 1))
			pr_output("\n");
	}
#endif
	int ret;
	test_obj tmp_obj;
	rbtree_cntr hdl;
	int num = 0;
	if (2 == param->cmd_num) {
		char *endptr = NULL;
		unsigned long val = strtoul(param->cmd_params[1], &endptr, 0);
		if (*endptr == '\0') num = val;
	}
	gobj_num = (num > 0 ? num : gobj_num);

	if (0 != rbtree_cntr_create(&hdl, sizeof(test_obj), gobj_num, test_obj_cmp))
		return -1;

	//rbtree_cntr_dump(&hdl);
	//printf("after create.\n");
	///getchar();

	for (int i = 0; i < gobj_num; ++i) {
		float _h = rand() % 100;
		float _l = rand() % 100;
		test_obj obj = {i , _h + _l * 0.01};
		rbtree_cntr_insert(&hdl, &obj, sizeof(obj));

		//rbtree_cntr_dump(&hdl);
		//printf("after insert.\n");
		//getchar();
	}
	//rbtree_cntr_fprint(&hdl, "test-1.dot", test_obj_getkey);

//RUN_TIME_START
	for (int i = 0; i < gobj_num; ++i) {
		if (i % 2) continue;
		test_obj obj = {.id = (i )};
		if (i != gobj_num / 2)
			rbtree_cntr_remove(&hdl, &obj, sizeof(obj));
		else {
			printf("\nremove\n");
			RUN_TIME_START
			rbtree_cntr_remove(&hdl, &obj, sizeof(obj));
			RUN_TIME_END
		}

		//rbtree_cntr_dump(&hdl);
		//printf("after remove.\n");
		//getchar();
	}
//RUN_TIME_END

	//rbtree_cntr_fprint(&hdl, "test-2.dot", test_obj_getkey);

	for (int i = 0; i < gobj_num; ++i) {
		if (i % 2) continue;
		test_obj obj = {.id = (i )};
		if (i != gobj_num / 2)
			rbtree_cntr_insert(&hdl, &obj, sizeof(obj));
		else {
			printf("\ninsert\n");
			RUN_TIME_START
			rbtree_cntr_insert(&hdl, &obj, sizeof(obj));
			RUN_TIME_END
		}
	}

	//rbtree_cntr_dump(&hdl);
	//printf("after insert 2.\n");
	//getchar();

	//rbtree_cntr_fprint(&hdl, "test-3.dot", test_obj_getkey);

	printf("\nsearch\n");
	tmp_obj.id = gobj_num / 2;
	RUN_TIME_START
	ret = rbtree_cntr_search(&hdl, &tmp_obj, sizeof(tmp_obj), &tmp_obj);
	RUN_TIME_END
	if (0 == ret) test_obj_show(&tmp_obj, sizeof(tmp_obj), NULL);
	else printf("Not found!\n");

	//rbtree_cntr_fprint(&hdl, "test-final.dot", test_obj_getkey);
	rbtree_cntr_destroy(&hdl);
	return 0;
}
static void rbtree_test_h(void)
{
	pr_output("  [31]rbtree-test : The rbtree test command.\n");
	pr_output("    '31|rbtree-test'\n");
}

#if 0
static int test2_fn(const cmd_list_t *pcmd, const params_t *param)
{
	pr_debug("%s() enter.\n", __func__);
	for (int i = 0; i < param->cmd_num; ++i) {
		if (0 == i)
			pr_debug("num  param\n");
		pr_debug("%-4d %s\n", i, param->cmd_params[i]);
		if (i == (param->cmd_num - 1))
			pr_output("\n");
	}

	return 0;
}

static void test2_h(void)
{
	pr_output("  test2 : The test command.\n");
}
#endif


static void init_cmd(void)
{
	pr_debug("%s() enter.\n", __func__);
}


static my_cmdlist_t mylist[] = {
	{{1,   "help",            print_help,       NULL},               NULL},
	{{11,  "list-test",       list_test,        list_test_h},        NULL},
	{{21,  "hash-test",       hash_test,        hash_test_h},        NULL},
	{{31,  "rbtree-test",     rbtree_test,      rbtree_test_h},      NULL},
};
static unsigned int num_mylist = sizeof(mylist) / sizeof(mylist[0]);
__attribute((constructor))
static void mylist_cmdlist_init(void) {
	pcmdlist    = (cmd_list_t *)mylist;
	cmdlist_num = num_mylist;
	cmdlist_size = sizeof(my_cmdlist_t);
	init_cmd();
}
