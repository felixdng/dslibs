#ifndef __TYPE_H
#define __TYPE_H

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#ifndef __always_inline
#define __always_inline inline
#endif

#ifdef NULL
#undef NULL
#endif
#define NULL ((void *)0)
typedef int bool;
enum {
	false	= 0,
	true	= 1
};

#define WRITE_ONCE(_a, _b) ({ (_a) = (_b); })

#define READ_ONCE(x) (x)

#define offsetof(type, member) \
	( (size_t) &((type *)0)->member)
#define container_of(ptr, type, member) ({ \
	void *__mptr = (void *)(ptr); \
	((type *)(__mptr - offsetof(type, member))); })

#define is_digit_string(_str, _d) ({ \
	char *_endptr = NULL; \
	_d = strtoul(_str, &_endptr, 0); \
	*_endptr == '\0'; })


#endif /* __TYPE_H */
