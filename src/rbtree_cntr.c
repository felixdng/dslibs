#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rbtree_augmented.h"
#include "rbtree_cntr.h"
#include "common.h"
#include "mem_alloc.h"


#define ALIGNED_LONG(_sz) ((_sz) + (((_sz) % sizeof(long)) ? (sizeof(long) - ((_sz) % sizeof(long))) : 0))

typedef struct {
	struct rb_node      rb;
	struct list_head    list;
	char                obj[];
} cntr_node;
#define NODE_SIZE(_sz) ALIGNED_LONG(sizeof(cntr_node) + (_sz))

#define IS_HEAD_VALID(_phead) \
	(_phead->obj_size > 0 && _phead->obj_num > 0 \
	&& _phead->obj_mem && _phead->fn_cmp)

static cntr_node *rbtree_search(struct rb_root *root, const void *obj, size_t obj_size, cmp_f fn_cmp)
{
	struct rb_node *node = root->rb_node;
	while (node) {
		cntr_node *this_node = rb_entry(node, cntr_node, rb);
		int result = fn_cmp(obj, this_node->obj, obj_size);
		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return this_node;
	}
	return NULL;
}

static int rbtree_insert(rbtree_cntr *ph, cntr_node *pnode)
{
    struct rb_node **new = &(ph->root.rb_node);
    struct rb_node *parent = NULL;
    while (*new) {
		void *this_obj = rb_entry(*new, cntr_node, rb)->obj;
        int result = ph->fn_cmp(pnode->obj, this_obj, ph->obj_size);

        parent = *new;
        if (result < 0)
            new = &((*new)->rb_left);
        else if (result > 0)
            new = &((*new)->rb_right);
        else
            return -1;
    }

    rb_link_node(&pnode->rb, parent, new);
    rb_insert_color(&pnode->rb, &ph->root);
    return 0;
}

PUBLIC int rbtree_cntr_create(rbtree_cntr *ph, size_t obj_size, size_t obj_num, cmp_f fn_cmp)
{
	if (NULL == ph || obj_size <= 0 || obj_num <= 0 || NULL == fn_cmp)
		return -1;

	memset(ph, 0, sizeof(rbtree_cntr));
	ph->obj_mem = mem_alloc(obj_num * NODE_SIZE(obj_size));
	if (NULL == ph->obj_mem) return -1;

	ph->obj_size = obj_size;
	ph->obj_num = obj_num;
	ph->fn_cmp = fn_cmp;
	ph->root = RB_ROOT;
	INIT_LIST_HEAD(&ph->free);

	for (int i = 0; i < ph->obj_num; ++i) {
		cntr_node *free_obj = (cntr_node *)(ph->obj_mem + i * NODE_SIZE(obj_size));
		list_add(&free_obj->list, &ph->free);
	}
	return 0;
}

PUBLIC void rbtree_cntr_destroy(rbtree_cntr *ph)
{
	if (NULL == ph) return;
	if (ph->obj_mem)
		mem_free(ph->obj_mem);
	memset(ph, 0, sizeof(rbtree_cntr));
	mem_info_show();
}

PUBLIC int rbtree_cntr_insert(rbtree_cntr *ph, void *obj, size_t obj_size)
{
	/* check params */
	if (NULL == ph || NULL == obj || obj_size <= 0) return -1;
	if (!IS_HEAD_VALID(ph)) return -1;
	if (obj_size != ph->obj_size) return -1;

	if (list_empty(&ph->free)) return -2;
	cntr_node *new_obj = list_entry(ph->free.next, cntr_node, list);
	memcpy(new_obj->obj, obj, ph->obj_size);
	if (0 != rbtree_insert(ph, new_obj)) return -3;
	list_del(&new_obj->list);
	return 0;
}

PUBLIC int rbtree_cntr_remove(rbtree_cntr *ph, void *obj, size_t obj_size)
{
	/* check params */
	if (NULL == ph || NULL == obj || obj_size <= 0) return -1;
	if (!IS_HEAD_VALID(ph)) return -1;
	if (obj_size != ph->obj_size) return -1;

	cntr_node *ret_node = rbtree_search(&ph->root, obj, obj_size, ph->fn_cmp);
	if (ret_node) {
		rb_erase(&ret_node->rb, &ph->root);
		list_add(&ret_node->list, &ph->free);
		return 0;
	}
	return -1;
}

PUBLIC int rbtree_cntr_search(rbtree_cntr *ph, void *obj, size_t obj_size, void *out_obj)
{
	/* check params */
	if (NULL == ph || NULL == obj || obj_size <= 0) return -1;
	if (!IS_HEAD_VALID(ph)) return -1;
	if (obj_size != ph->obj_size) return -1;

	cntr_node *ret_node = rbtree_search(&ph->root, obj, obj_size, ph->fn_cmp);
	if (ret_node) {
		if (out_obj) memcpy(out_obj, ret_node->obj, obj_size);
		return 0;
	} else {
		return -1;
	}
}

/* 生成dot文件 */
static void rbtree_fprint_node(struct rb_node *node, FILE *fp, get_key_f fn_getkey)
{
	if (rb_is_black(node)) {
		fprintf(fp, "node[shape=record,style=filled,color=black,fontcolor=white];\n");
	}else{
		fprintf(fp, "node[shape=record,style=filled,color=red,fontcolor=white];\n");
	}
	int val = fn_getkey(rb_entry(node, cntr_node, rb)->obj);
	fprintf(fp, "%d[label=\"<f0> | <f1> %d | <f2> \"];\n", val, val);
}

static void rbtree_fprint_tree(struct rb_node *node, FILE *fp, get_key_f fn_getkey)
{
	if (NULL == node) return;
	if (NULL == rb_parent(node)) { //root
		rbtree_fprint_node(node, fp, fn_getkey);
	}
	if (NULL != node->rb_left) {
		rbtree_fprint_node(node->rb_left, fp, fn_getkey);
		int this_val = fn_getkey(rb_entry(node, cntr_node, rb)->obj);
		int left_val = fn_getkey(rb_entry(node->rb_left, cntr_node, rb)->obj);
		fprintf(fp, "%d:f0:sw->%d:f1;\n", this_val, left_val);
	}
	if (NULL != node->rb_right) {
		rbtree_fprint_node(node->rb_right, fp, fn_getkey);
		int this_val = fn_getkey(rb_entry(node, cntr_node, rb)->obj);
		int right_val = fn_getkey(rb_entry(node->rb_right, cntr_node, rb)->obj);
		fprintf(fp, "%d:f2:se->%d:f1;\n", this_val, right_val);
	}
	rbtree_fprint_tree(node->rb_left, fp, fn_getkey);
	rbtree_fprint_tree(node->rb_right, fp, fn_getkey);
}

PUBLIC void rbtree_cntr_fprint(rbtree_cntr *ph, const char *fname, get_key_f fn_getkey)
{
	/* check params */
	if (NULL == ph || NULL == fname || NULL == fn_getkey) return;
	if (!IS_HEAD_VALID(ph)) return;
	if (strlen(fname) <= 0) return;

	FILE *fp = fopen(fname, "w");
	if (NULL == fp) return;
	fprintf(fp, "digraph G{\n");
	rbtree_fprint_tree(ph->root.rb_node, fp, fn_getkey);
	fprintf(fp, "}");
	fclose(fp);
}

PUBLIC void rbtree_cntr_dump(rbtree_cntr *ph)
{
	if (!ph) return;
	printf("\ndump rbtree cntr info\n");
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
	struct rb_node *node, *last = NULL;
	printf("  rbtree list:\n");
	for (node = rb_first(&ph->root); node; node = rb_next(node)) {
		index++;
		cntr_node *pnode = rb_entry(node, cntr_node, rb);
		if (1 == index)
			printf("    %p->%p", &ph->root, pnode);
		else if (index < 5)
			printf("->%p", pnode);

		last = node;
	}
	if (index > 0) {
		if (last) {
			if (index > 5) printf("-> ... ");
			if (index >= 5) printf("->%p", rb_entry(last, cntr_node, rb));
		}
		printf("\n");
	}
	printf("  rbtree list cnt:%ld\n", index);
	printf("\n");
}


