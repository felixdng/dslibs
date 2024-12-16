#ifndef __LIST_CNTR_H
#define __LIST_CNTR_H

#include "list.h"
#include "cntr_comm.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum {
	ORDER_NONE = 0, /* 无序 */
	ORDER_UP   = 1, /* 升序 */
	ORDER_DOWN = 2, /* 降序 */
};

typedef struct list_cntr {
	size_t              obj_size;  /* 存储对象的大小 */
	size_t              obj_num;   /* 存储对象的最大数量 */
	void                *obj_mem;  /* 内存池起始地址 */
	cmp_f               fn_cmp;    /* 存储对象比较函数 */
	struct list_head    used;      /* 已使用的对象链表 */
	struct list_head    free;      /* 空闲的对象链表 */
	int                 order;     /* 插入顺序 */
} list_cntr;

int list_cntr_create(list_cntr *ph, size_t obj_size, size_t obj_num, cmp_f fn_cmp, int order);
void list_cntr_destroy(list_cntr *ph);
int list_cntr_insert(list_cntr *ph, void *obj, size_t obj_size);
int list_cntr_remove(list_cntr *ph, void *obj, size_t obj_size);
int list_cntr_search(list_cntr *ph, void *obj, size_t obj_size, void *out_obj);
void list_cntr_show(list_cntr *ph, show_f fn_show, void *obj, size_t obj_size);

void list_cntr_dump(list_cntr *ph);


#ifdef __cplusplus
}
#endif
#endif /* __LIST_CNTR_H */
