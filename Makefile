#
# top makefile
#
NO_COL=\033[0m
GR_COL=\033[32;01m

TOPDIR:=$(shell pwd)
export TOPDIR

# 编译输出目录
BUILD_DIR:=$(TOPDIR)/build
export BUILD_DIR

# 头文件目录
INC_DIR:=inc
INC_DIR:=$(addprefix $(TOPDIR)/,$(INC_DIR))

ifeq ($(ARCH),aarch64)
CROSS_COMPILE=aarch64-linux-gnu-
else #x86
CROSS_COMPILE=
endif

AS=$(CROSS_COMPILE)as
LD=$(CROSS_COMPILE)ld
CC=$(CROSS_COMPILE)gcc
CPP=$(CC) -E
AR=$(CROSS_COMPILE)ar
NM=$(CROSS_COMPILE)nm
STRIP=$(CROSS_COMPILE)strip
OBJCOPY=$(CROSS_COMPILE)objcopy
OBJDUMP=$(CROSS_COMPILE)objdump
export AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP

CFLAGS:=-Wall -fPIC -std=gnu99 -Werror
ifneq ($(release),)
CFLAGS+=-O2
else
CFLAGS+=-O0 -g3 -gdwarf-2
endif
CFLAGS+=-fvisibility=hidden
CFLAGS+=$(addprefix -I,$(INC_DIR))

LDFLAGS:=
LDFLAGS+=-shared
export CFLAGS LDFLAGS

OBJS_PREFIX:=$(BUILD_DIR)
export OBJS_PREFIX

# 最终目标
TARGET:=$(OBJS_PREFIX)/libdstool.so

# 需要编译的子目录(注意不能省略/)
obj-y += src/



include $(TOPDIR)/Makefile.config



.PHONY:all prebuild recursive_build clean distclean

all:prebuild recursive_build $(TARGET)
	@printf "$(GR_COL)[$(shell date +%s%3N)] '$(notdir $(TARGET))' has been built!$(NO_COL)\n"

prebuild:
	@printf "$(GR_COL)[$(shell date +%s%3N)] '$(notdir $(TARGET))' will be built.$(NO_COL)\n"
	@mkdir -p $(BUILD_DIR)

recursive_build:
	make -C ./ -f $(TOPDIR)/Makefile.build

$(TARGET):$(OBJS_PREFIX)/built-in.o
	$(CC) $^ $(LDFLAGS) -o $@
ifneq ($(release),)
	$(STRIP) $@ -o $(dir $@)$(basename $(notdir $@))-release$(suffix $@)
endif

files_clean=$(shell find $(OBJS_PREFIX) -name "*.o" -o -name "*.so*")
clean:
ifneq ($(wildcard $(OBJS_PREFIX)),)
ifneq ($(wildcard $(files_clean)),)
	rm -f $(files_clean)
endif
endif

files_distclean=$(shell find $(OBJS_PREFIX) -name "*.o.d")
distclean:clean
ifneq ($(wildcard $(OBJS_PREFIX)),)
ifneq ($(wildcard $(files_distclean)),)
	rm -f $(files_distclean)
endif
endif

