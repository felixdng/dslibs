#ifndef __HASH_CNTR_H
#define __HASH_CNTR_H

#include "list.h"
#include "cntr_comm.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct hash_cntr {
	size_t              obj_size;      /* 存储对象的大小 */
	size_t              obj_num;       /* 存储对象的最大数量 */
	size_t              bucket_size;   /* hash桶的大小 */
	calc_f              fn_calc;       /* hash值计算函数 */
	cmp_f               fn_cmp;        /* 存储对象比较函数 */
	void                *obj_mem;      /* 内存池起始地址 */
	struct list_head    free;          /* 空闲的对象链表 */
	struct hlist_head   *head;         /* hash链表头 */
} hash_cntr;

int hash_cntr_create(hash_cntr *ph, size_t obj_size, size_t obj_num, size_t bucket_size,
						calc_f fn_calc, cmp_f fn_cmp);
void hash_cntr_destroy(hash_cntr *ph);
int hash_cntr_insert(hash_cntr *ph, void *obj, size_t obj_size);
int hash_cntr_remove(hash_cntr *ph, void *obj, size_t obj_size);
int hash_cntr_search(hash_cntr *ph, const void *obj, size_t obj_size, void *out_obj);
void hash_cntr_show(hash_cntr *ph, show_f fn_show, void *obj, size_t obj_size);

void hash_cntr_dump(hash_cntr *ph);

#ifdef __cplusplus
}
#endif
#endif /* __HASH_CNTR_H */