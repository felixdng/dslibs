#ifndef __CNTR_COMM_H
#define __CNTR_COMM_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef size_t (*calc_f)(const void *obj, size_t size);
typedef int (*cmp_f)(const void *obj1, const void *obj2, size_t size);
typedef void (*show_f)(const void *obj, size_t size, const char *title);
typedef int (*get_key_f)(const void *obj);

#ifdef __cplusplus
}
#endif
#endif /* __CNTR_COMM_H */
