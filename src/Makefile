EXTRA_CFLAGS:=
#ifneq ($(CONFIG_TEST),)
#CFLAGS_main.o:=-DTEST
#endif

obj-y += version.o
obj-y += mem_alloc.o

obj-y += list_cntr.o
obj-y += hash_cntr.o
obj-y += rbtree.o
obj-y += rbtree_cntr.o
#obj-$(CONFIG_TEST) += test.o

#obj-y += mymath/


__ver_file:=$(shell touch version.c)

ifeq ($(CONFIG_MEM_DEBUG),y)
CFLAGS_mem_alloc.o:=-DMEM_DEBUG
endif
