#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_cntr.h"
#include "common.h"
#include "mem_alloc.h"


#ifdef DEBUG
#define pr_dbg(fmt, ...) \
	printf("[debug] "fmt, ##__VA_ARGS__)
#else
#define pr_dbg(fmt, ...)
#endif

typedef struct cntr_node {
	union {
		struct hlist_node   hlist;
		struct list_head    list;
	} un;
	char obj[];
} cntr_node;
#define NODE_SIZE(_sz) (sizeof(cntr_node) + _sz)

#define IS_HEAD_VALID(_phead) \
	(_phead->obj_size > 0 && _phead->obj_num > 0 && _phead->bucket_size > 0 \
	&& _phead->fn_calc && _phead->fn_cmp && _phead->obj_mem)


static cntr_node *hash_search(struct hlist_head *head, const void *obj, size_t obj_size, cmp_f fn_cmp)
{
	struct hlist_node *p = NULL;
	hlist_for_each(p, head) {
		cntr_node *this_node = hlist_entry(p, cntr_node, un.hlist);
		if (0 == fn_cmp(obj, this_node->obj, obj_size)) {
			return this_node;
		}
	}
	return NULL;
}

PUBLIC int hash_cntr_create(hash_cntr *ph, size_t obj_size, size_t obj_num, size_t bucket_size,
								calc_f fn_calc, cmp_f fn_cmp)
{
	if (NULL == ph || obj_size <= 0 || obj_num <= 0 || bucket_size <= 0
		|| NULL == fn_calc || NULL == fn_cmp) return -1;

	memset(ph, 0, sizeof(hash_cntr));
	/* -----------------------------------------------------
	*  | node1, node2, ... nodeN | head1, head2, ... headM |
	*  -----------------------------------------------------
	*/
	ph->obj_mem = mem_alloc(obj_num * NODE_SIZE(obj_size) + bucket_size * sizeof(struct hlist_head));
	if (NULL == ph->obj_mem) return -1;

	ph->obj_size = obj_size;
	ph->obj_num = obj_num;
	ph->bucket_size = bucket_size;
	ph->fn_calc = fn_calc;
	ph->fn_cmp = fn_cmp;

	INIT_LIST_HEAD(&ph->free);
	for (int i = 0; i < ph->obj_num; ++i) {
		cntr_node *free_obj = (cntr_node *)(ph->obj_mem + i * NODE_SIZE(obj_size));
		list_add_tail(&free_obj->un.list, &ph->free);
	}

	ph->head = ph->obj_mem + obj_num * NODE_SIZE(obj_size);
	for (int i = 0; i < ph->bucket_size; ++i) {
		INIT_HLIST_HEAD(&ph->head[i]);
	}
	return 0;
}

PUBLIC void hash_cntr_destroy(hash_cntr *ph)
{
	if (!ph) return;
	if (ph->obj_mem) mem_free(ph->obj_mem);
	memset(ph, 0, sizeof(hash_cntr));
	mem_info_show();
}

PUBLIC int hash_cntr_insert(hash_cntr *ph, void *obj, size_t obj_size)
{
	/* check params */
	if (NULL == ph || NULL == obj || obj_size <= 0) return -1;
	if (!IS_HEAD_VALID(ph)) return -1;
	if (obj_size != ph->obj_size) return -1;

	size_t idx = ph->fn_calc(obj, obj_size);
	if (idx >= ph->bucket_size) return -1;
	struct hlist_head *h = &ph->head[idx];

	if (list_empty(&ph->free)) return -2;
	cntr_node *new_obj = list_entry(ph->free.prev, cntr_node, un.list);
	memcpy(new_obj->obj, obj, ph->obj_size);

	list_del(&new_obj->un.list);
	INIT_HLIST_NODE(&new_obj->un.hlist);
	hlist_add_head(&new_obj->un.hlist, h);
	return 0;
}

PUBLIC int hash_cntr_remove(hash_cntr *ph, void *obj, size_t obj_size)
{
	/* check params */
	if (NULL == ph || NULL == obj || obj_size <= 0) return -1;
	if (!IS_HEAD_VALID(ph)) return -1;
	if (obj_size != ph->obj_size) return -1;

	size_t idx = ph->fn_calc(obj, obj_size);
	if (idx >= ph->bucket_size) return -1;
	struct hlist_head *h = &ph->head[idx];

	cntr_node *ret_node = hash_search(h, obj, obj_size, ph->fn_cmp);
	if (ret_node) {
		hlist_del(&ret_node->un.hlist);
		list_add_tail(&ret_node->un.list, &ph->free);
		return 0;
	}
	return -2;
}

PUBLIC int hash_cntr_search(hash_cntr *ph, const void *obj, size_t obj_size, void *out_obj)
{
	/* check params */
	if (NULL == ph || NULL == obj || obj_size <= 0) return -1;
	if (!IS_HEAD_VALID(ph)) return -1;
	if (obj_size != ph->obj_size) return -1;

	size_t idx = ph->fn_calc(obj, obj_size);
	if (idx >= ph->bucket_size) return -1;
	struct hlist_head *h = &ph->head[idx];

	cntr_node *ret_node = hash_search(h, obj, obj_size, ph->fn_cmp);
	if (ret_node) {
		if (out_obj) memcpy(out_obj, ret_node->obj, ph->obj_size);
		return 0;
	}
	return -1;
}

PUBLIC void hash_cntr_show(hash_cntr *ph, show_f fn_show, void *obj, size_t obj_size)
{
	/* check params */
	if (NULL == ph || NULL == fn_show || NULL == obj || obj_size <= 0) return;
	if (!IS_HEAD_VALID(ph)) return;
	if (obj_size != ph->obj_size) return;

	for (int i = 0; i < ph->bucket_size; ++i) {
		struct hlist_head *h = &ph->head[i];
		struct hlist_node *p = NULL;
		char str_line[50] = {};
		snprintf(str_line, sizeof(str_line), "hash head %u", i);
		const char *pstr = str_line;
		hlist_for_each(p, h) {
			cntr_node *pnode = hlist_entry(p, cntr_node, un.hlist);
			memcpy(obj, pnode->obj, obj_size);
			fn_show(obj, obj_size, pstr);
			if (pstr) pstr = NULL;
		}
	}
}


PUBLIC void hash_cntr_dump(hash_cntr *ph)
{
	if (!ph) return;
	printf("\ndump hash cntr info\n");
	printf("  obj size    : %ld\n", (long)ph->obj_size);
	printf("  obj num     : %ld\n", (long)ph->obj_num);
	printf("  bucket size : %ld\n", (long)ph->bucket_size);
	printf("  obj mem     : %p - %p\n", ph->obj_mem, ph->obj_mem + (ph->obj_num * NODE_SIZE(ph->obj_size)) - 1);
	printf("  node size   : %ld\n", (long)NODE_SIZE(ph->obj_size));

	size_t index = 0;
	struct list_head *pos = NULL;
	printf("  free list:\n");
	list_for_each(pos, &ph->free) {
		index++;
		cntr_node *pnode = list_entry(pos, cntr_node, un.list);
		if (1 == index)
			printf("    %p->%p", &ph->free, pnode);
		else if (index < 5)
			printf("->%p", pnode);
		else if (5 == index) {
			if (pos != ph->free.prev)
				printf("-> ... ");
			printf("->%p", list_entry(ph->free.prev, cntr_node, un.list));
		}
	}
	if (index > 0) printf("\n");
	printf("  free list cnt:%ld\n", (long)index);

	size_t total = 0;
	printf("  head list:\n");
	for (int i = 0; i < ph->bucket_size; ++i) {
		index = 0;
		struct hlist_node *p, *last = NULL;
		printf("    head[%08d] ", i);
		hlist_for_each(p, &ph->head[i]) {
			index++;
			cntr_node *pnode = hlist_entry(p, cntr_node, un.hlist);
			if (1 == index)
				printf("%p->%p", &ph->head[i], pnode);
			else if (index < 5)
				printf("->%p", pnode);

			last = p;
		}
		if (index > 0) {
			if (last) {
				if (index > 5) printf("-> ... ");
				if (index >= 5) printf("->%p", hlist_entry(last, cntr_node, un.hlist));
			}
		}
		printf(" cnt:%ld\n", index);
		total += index;
	}
	printf("  head list cnt:%ld\n", total);
	printf("\n");
}

