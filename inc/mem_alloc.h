#ifndef __MEM_ALLOC_H
#define __MEM_ALLOC_H

/******* include head *******/


#ifdef __cplusplus
extern "C"
{
#endif
/******* user's define start *******/


void mem_info_show(void);
void mem_free(void *ptr);
void *mem_alloc(size_t size);


/******* user's define end *******/
#ifdef __cplusplus
}
#endif

#endif /* __MEM_ALLOC_H */
