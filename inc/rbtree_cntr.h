#ifndef __RBTREE_CNTR_H
#define __RBTREE_CNTR_H

#include "list.h"
#include "rbtree.h"
#include "cntr_comm.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct {
	size_t              obj_size;  /* 存储对象的大小 */
	size_t              obj_num;   /* 存储对象的最大数量 */
	void                *obj_mem;  /* 内存池起始地址 */
	cmp_f               fn_cmp;    /* 存储对象比较函数 */
	struct rb_root      root;      /* rbtree root */
	struct list_head    free;      /* 空闲的对象链表 */
} rbtree_cntr;

int rbtree_cntr_create(rbtree_cntr *ph, size_t obj_size, size_t obj_num, cmp_f fn_cmp);
void rbtree_cntr_destroy(rbtree_cntr *ph);
int rbtree_cntr_insert(rbtree_cntr *ph, void *obj, size_t obj_size);
int rbtree_cntr_remove(rbtree_cntr *ph, void *obj, size_t obj_size);
int rbtree_cntr_search(rbtree_cntr *ph, void *obj, size_t obj_size, void *out_obj);

/* for debug */
void rbtree_cntr_fprint(rbtree_cntr *ph, const char *fname, get_key_f fn_getkey);
void rbtree_cntr_dump(rbtree_cntr *ph);

#ifdef __cplusplus
}
#endif
#endif /* __RBTREE_CNTR_H */