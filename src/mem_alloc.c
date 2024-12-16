#include <stdlib.h>
#include <stdio.h>
#include "list.h"


#ifdef MEM_DEBUG
typedef struct {
	const void       *addr;
	size_t           size;
	unsigned int     alloc_times;
	unsigned int     free_times;
	struct list_head list;
} minfo;

struct list_head info_head;

__attribute((constructor))
static void mem_init(void)
{
	INIT_LIST_HEAD(&info_head);
}

static inline minfo *info_search(const void *addr, size_t size)
{
	struct list_head *p;
	list_for_each(p, &info_head) {
		minfo *node = list_entry(p, minfo, list);
		if (-1 == size && addr == node->addr) {
			return node;
		}
		if (-1 != size && addr == node->addr && size == node->size) {
			return node;
		}
	}
	return NULL;
}

static void mem_info_alloc(const void *addr, size_t size)
{
	minfo *target = NULL;
	target = info_search(addr, size);
	if (target) {
		target->alloc_times++;
	}
	else {
		target = calloc(1, sizeof(minfo));
		target->addr = addr;
		target->size = size;
		target->alloc_times = 1;
		target->free_times = 0;
		list_add_tail(&target->list, &info_head);
	}
}

static void mem_info_free(const void *addr)
{
	minfo *target = NULL;
	target = info_search(addr, -1);
	if (target) {
		target->free_times++;
	}
	else {
		target = calloc(1, sizeof(minfo));
		target->addr = addr;
		target->size = 0;
		target->alloc_times = 0;
		target->free_times = 1;
		list_add_tail(&target->list, &info_head);
	}
}
#endif /* MEM_DEBUG */

void mem_info_show(void)
{
#ifdef MEM_DEBUG
	size_t cnt = 0;
	struct list_head *p;
	printf("\nmem info dump:\n");
	list_for_each(p, &info_head) {
		minfo *node = list_entry(p, minfo, list);
		printf("addr=%p, size=%ld, alloc times=%u, free times=%u\n",
				node->addr, node->size, node->alloc_times, node->free_times);
		cnt++;
	}
	printf("total: %ld\n\n", cnt);
#endif
}

void mem_free(void *ptr)
{
	if (ptr) {
#ifdef MEM_DEBUG
		mem_info_free(ptr);
#endif /* MEM_DEBUG */
		free(ptr);
	}
}

void *mem_alloc(size_t size)
{
	if (size > 0) {
		void *ptr = calloc(1, size);
#ifdef MEM_DEBUG
		mem_info_alloc(ptr, size);
#endif /* MEM_DEBUG */
		return ptr;
	}
	return NULL;
}

