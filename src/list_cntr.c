#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list_cntr.h"
#include "common.h"
#include "mem_alloc.h"


typedef struct cntr_node {
	struct list_head    list;
	char                obj[];
} cntr_node;
#define NODE_SIZE(_sz) (sizeof(cntr_node) + _sz)

#define IS_HEAD_VALID(_phead) \
	(_phead->obj_size > 0 && _phead->obj_num > 0 \
	&& _phead->obj_mem && _phead->fn_cmp)


static cntr_node *list_search(struct list_head *head, const void *obj, size_t obj_size, cmp_f fn_cmp)
{
	struct list_head *p = NULL;
	list_for_each(p, head) {
		cntr_node *this_node = list_entry(p, cntr_node, list);
		int result = fn_cmp(obj, this_node->obj, obj_size);
		if (0 == result) {
			return this_node;
		}
	}
	return NULL;
}

PUBLIC int list_cntr_create(list_cntr *ph, size_t obj_size, size_t obj_num, cmp_f fn_cmp, int order)
{
	if (NULL == ph || obj_size <= 0 || obj_num <= 0 || NULL == fn_cmp)
		return -1;

	memset(ph, 0, sizeof(list_cntr));
	ph->obj_mem = mem_alloc(obj_num * NODE_SIZE(obj_size));
	if (NULL == ph->obj_mem) return -1;

	ph->obj_size = obj_size;
	ph->obj_num = obj_num;
	ph->fn_cmp = fn_cmp;
	INIT_LIST_HEAD(&ph->free);
	INIT_LIST_HEAD(&ph->used);

	ph->order = ORDER_NONE;
	if (ORDER_UP == order) ph->order = ORDER_UP;
	else if (ORDER_DOWN == order) ph->order = ORDER_DOWN;

	for (int i = 0; i < ph->obj_num; ++i) {
		cntr_node *free_obj = (cntr_node *)(ph->obj_mem + i * NODE_SIZE(obj_size));
		list_add_tail(&free_obj->list, &ph->free);
	}
	return 0;
}

PUBLIC void list_cntr_destroy(list_cntr *ph)
{
	if (!ph) return;
	if (ph->obj_mem) mem_free(ph->obj_mem);
	memset(ph, 0, sizeof(list_cntr));
	mem_info_show();
}

PUBLIC int list_cntr_insert(list_cntr *ph, void *obj, size_t obj_size)
{
	/* check params */
	if (NULL == ph || NULL == obj || obj_size <= 0) return -1;
	if (!IS_HEAD_VALID(ph)) return -1;
	if (obj_size != ph->obj_size) return -1;

	if (list_empty(&ph->free)) return -2;
	cntr_node *new_obj = list_entry(ph->free.prev, cntr_node, list);
	memcpy(new_obj->obj, obj, ph->obj_size);

	if (ORDER_UP == ph->order || ORDER_DOWN == ph->order) {
		struct list_head *p1 = ph->used.next;
		struct list_head *p2 = ph->used.prev;
		struct list_head *prev = &ph->used;
		while (1) {
			if (p1 == p2) {
				prev = p1;
				break;
			}

			int ret1 = ph->fn_cmp(new_obj->obj, list_entry(p1, cntr_node, list)->obj, ph->obj_size);
			if ((ret1 <= 0 && ORDER_UP == ph->order) ||
				(ret1 >= 0 && ORDER_DOWN == ph->order)) {
				prev = p1->prev;
				break;
			}

			int ret2 = ph->fn_cmp(new_obj->obj, list_entry(p2, cntr_node, list)->obj, ph->obj_size);
			if ((ret2 >= 0 && ORDER_UP == ph->order) ||
				(ret2 <= 0 && ORDER_DOWN == ph->order)) {
				prev = p2;
				break;
			}

			if (p1->next == p2) {
				prev = p1;
				break;
			}
			p1 = p1->next;
			p2 = p2->prev;
		}
		list_del(&new_obj->list);
		list_add(&new_obj->list, prev);
	}
	else {
		list_move_tail(&new_obj->list, &ph->used);
	}

	return 0;
}

PUBLIC int list_cntr_remove(list_cntr *ph, void *obj, size_t obj_size)
{
	/* check params */
	if (NULL == ph || NULL == obj || obj_size <= 0) return -1;
	if (!IS_HEAD_VALID(ph)) return -1;
	if (obj_size != ph->obj_size) return -1;

	cntr_node *ret_node = list_search(&ph->used, obj, obj_size, ph->fn_cmp);
	if (ret_node) {
		list_move_tail(&ret_node->list, &ph->free);
		return 0;
	}
	return -2;
}

PUBLIC int list_cntr_search(list_cntr *ph, void *obj, size_t obj_size, void *out_obj)
{
	/* check params */
	if (NULL == ph || NULL == obj || obj_size <= 0) return -1;
	if (!IS_HEAD_VALID(ph)) return -1;
	if (obj_size != ph->obj_size) return -1;

	cntr_node *ret_node = list_search(&ph->used, obj, obj_size, ph->fn_cmp);
	if (ret_node) {
		if (out_obj) memcpy(out_obj, ret_node->obj, ph->obj_size);
		return 0;
	}
	return -2;
}

PUBLIC void list_cntr_show(list_cntr *ph, show_f fn_show, void *obj, size_t obj_size)
{
	/* check params */
	if (NULL == ph || NULL == obj || obj_size <= 0 || NULL == fn_show) return;
	if (!IS_HEAD_VALID(ph)) return;
	if (obj_size != ph->obj_size) return;

	struct list_head *p = NULL;
	list_for_each(p, &ph->used) {
		void *target_obj = list_entry(p, cntr_node, list)->obj;
		if (target_obj) {
			memcpy(obj, target_obj, obj_size);
			fn_show(obj, obj_size, NULL);
		}
	}
}

PUBLIC void list_cntr_dump(list_cntr *ph)
{
	if (!ph) return;
	printf("\ndump list cntr info\n");
	printf("  obj size: %ld\n", (long)ph->obj_size);
	printf("  obj num:  %ld\n", (long)ph->obj_num);
	printf("  obj mem:  %p - %p\n", ph->obj_mem, ph->obj_mem + (ph->obj_num * NODE_SIZE(ph->obj_size)) - 1);
	printf("  node size: %ld\n", (long)NODE_SIZE(ph->obj_size));

	size_t index = 0;
	struct list_head *pos = NULL;
	printf("  free list:\n");
	list_for_each(pos, &ph->free) {
		index++;
		cntr_node *pnode = list_entry(pos, cntr_node, list);
		if (1 == index)
			printf("    %p->%p", &ph->free, pnode);
		else if (index < 5)
			printf("->%p", pnode);
		else if (5 == index) {
			if (pos != ph->free.prev)
				printf("-> ... ");
			printf("->%p", list_entry(ph->free.prev, cntr_node, list));
		}
	}
	if (index > 0) printf("\n");
	printf("  free list cnt:%ld\n", index);

	index = 0;
	printf("  used list:\n");
	list_for_each(pos, &ph->used) {
		index++;
		cntr_node *pnode = list_entry(pos, cntr_node, list);
		if (1 == index)
			printf("    %p->%p", &ph->used, pnode);
		else if (index < 5)
			printf("->%p", pnode);
		else if (5 == index) {
			if (pos != ph->used.prev)
				printf("-> ... ");
			printf("->%p", list_entry(ph->used.prev, cntr_node, list));
		}
	}
	if (index > 0) printf("\n");
	printf("  used list cnt:%ld\n", index);

	printf("\n");
}

