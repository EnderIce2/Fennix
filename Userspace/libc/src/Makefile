# Config file
include ../../../Makefile.conf

NAME=c

ifeq ($(USERSPACE_STATIC_LIBS), 1)
OBJECT_NAME := lib$(NAME).a
else
OBJECT_NAME := lib$(NAME).so
endif

OUTPUT_DIR=../../out/lib/
SYSROOT = --sysroot=../../out/

CC = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)gcc
AS = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)as
AR = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)ar

C_SOURCES = $(shell find ./ -type f -name '*.c')
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp')
S_SOURCES = $(shell find ./ -type f -name '*.S')
OBJ = ${C_SOURCES:.c=.o} ${CPP_SOURCES:.cpp=.o} ${S_SOURCES:.S=.o}

ifeq ($(OSARCH), amd64)
ASM_ARCH := elf64
else ifeq ($(OSARCH), i386)
ASM_ARCH := elf32
endif

CFLAGS := -fvisibility=hidden -fPIC -I../include -I../../out/include

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fverbose-asm
endif

build: $(OBJECT_NAME)

$(OBJECT_NAME): $(OBJ)
	$(info Linking $@)
ifeq ($(USERSPACE_STATIC_LIBS), 1)
	$(AR) rcs $(OUTPUT_DIR)$@ $(OBJ)
else
	$(CC) -nostdlib -shared -fPIC -fPIE -Wl,-soname,$(OBJECT_NAME) $(SYSROOT) $(OBJ) -o $(OUTPUT_DIR)$@
endif

%.o: %.c
	$(info Compiling $<)
	$(CC) $(CFLAGS) -std=c17 -c $< -o $@

%.o: %.cpp
	$(info Compiling $<)
	$(CC) $(CFLAGS) -std=c++20 -c $< -o $@

%.o: %.S
	$(info Compiling $<)
	$(AS) -c $< -o $@

clean:
	rm -f $(OBJ)
