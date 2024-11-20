# Config file
include ../../../Makefile.conf

NAME=ld

OBJECT_NAME=$(NAME).so
SO_NAME=$(OBJECT_NAME)

OUTPUT_DIR=../../out/lib/
SYSROOT = --sysroot=../../out/

CC = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)gcc
AS = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)as
AR = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)ar
OBJDUMP = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)objdump
NASM = /usr/bin/nasm

C_SOURCES = $(shell find ./ -type f -name '*.c')
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp')
S_SOURCES = $(shell find ./ -type f -name '*.S')
ASM_SOURCES = $(shell find ./ -type f -name '*.asm')
OBJ = ${C_SOURCES:.c=.o} ${CPP_SOURCES:.cpp=.o} ${ASM_SOURCES:.asm=.o} ${S_SOURCES:.S=.o}

INCLUDE = ../include

SIMD_FLAGS := -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2 -mno-sse4 -mno-avx -mno-avx2 -mno-avx512f

CFLAGS := -I$(INCLUDE) $(SIMD_FLAGS) -fPIC
LDFLAGS := -nostartfiles -nostdlib -Wl,-soname,$(SO_NAME) $(SYSROOT) -fno-pie -fno-PIC

ifeq ($(OSARCH), amd64)
ASM_ARCH := elf64
LDFLAGS += -Ttext=0xFFFFFFFFF0001000
CFLAGS += -m64
else ifeq ($(OSARCH), i386)
ASM_ARCH := elf32
LDFLAGS += -fixme
CFLAGS += -m32
endif

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fverbose-asm
	LDFLAGS += -ggdb3 -O0
endif

build: $(OBJECT_NAME)

$(OBJECT_NAME): $(OBJ)
	$(info Linking $@)
	$(CC) $(LDFLAGS) $(OBJ) -o $(OUTPUT_DIR)$@
	$(OBJDUMP) -d $(OUTPUT_DIR)$@ > file_dump.map

%.o: %.c
	$(info Compiling $<)
	$(CC) $(CFLAGS) -std=c17 -c $< -o $@

%.o: %.cpp
	$(info Compiling $<)
	$(CC) $(CFLAGS) -std=c++20 -c $< -o $@

%.o: %.S
	$(info Compiling $<)
	$(AS) -c $< -o $@

%.o: %.asm
	$(info Compiling $<)
	$(NASM) $< -f $(ASM_ARCH) -o $@

clean:
	rm -f file_dump.map $(OBJ)
