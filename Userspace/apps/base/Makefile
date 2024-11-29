# Config file
include ../../../config.mk

cwd := $(CURDIR)
CACHE_DIR := $(cwd)/../../cache
PREFIX := $(cwd)/../../out/
TARGET := x86_64-fennix

export CC := $(cwd)/../../../tools/cross/bin/$(TARGET)-gcc
export LD := $(cwd)/../../../tools/cross/bin/$(TARGET)-ld
export AR := $(cwd)/../../../tools/cross/bin/$(TARGET)-ar
export STRIP := $(cwd)/../../../tools/cross/bin/$(TARGET)-strip
export RANLIB := $(cwd)/../../../tools/cross/bin/$(TARGET)-ranlib
export LD_LIBRARY_PATH := $(cwd)/../../out/lib/

ifeq ($(DEBUG), 1)
export CFLAGS := --sysroot=$(cwd)/../../out/ -I$(cwd)/../../out/include/ -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fverbose-asm
export LDFLAGS := -ggdb3 -O0
else
export CFLAGS := --sysroot=$(cwd)/../../out/ -I$(cwd)/../../out/include/
endif

build:
	make -C utest build
	make -C cross_test build
	make -C echo build
	make -C fsh build

clean:
	make -C utest clean
	make -C cross_test clean
	make -C echo clean
	make -C fsh clean
