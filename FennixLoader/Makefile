# Config file
include ../../Makefile.conf

NASM_ARCH =

CC = ../../$(COMPILER_PATH)/i386-elf-gcc
CPP = ../../$(COMPILER_PATH)/i386-elf-g++
NASM = /usr/bin/nasm

ASM_SOURCES = $(shell find ./ -type f -name '*.asm')
C_SOURCES = $(shell find ./ -type f -name '*.c')
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp')

OBJ = $(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o) $(ASM_SOURCES:.asm=.o)

LDFLAGS = -shared -nostdlib -nodefaultlibs -nolibc -Tlinker.ld

CFLAGS = -fno-pic -fno-pie		\
		 -mno-red-zone -pipe	\
		 -fno-builtin -Wno-main	\
		 -march=pentium -I./include -Wall -Wextra

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fsanitize=undefined
	NASM_ARCH += -F dwarf -g
endif

build: $(OBJ)
	$(info Linking $@)
	$(CC) $(LDFLAGS) $(OBJ) -o FLDR.elf
	mv FLDR.elf ../FLDR.elf
 
%.o: %.c
	$(info Compiling $<)
	$(CC) $(CFLAGS) -std=c17 -c $< -o $@

%.o: %.cpp
	$(info Compiling $<)
	$(CPP) $(CFLAGS) -std=c++20 -fexceptions -c $< -o $@ -fno-rtti

%.o: %.asm
	$(info Compiling $<)
	$(NASM) $< $(NASM_ARCH) -f elf32 -o $@

clean_only_obj:
	$(info Removing objects $(OBJ))
	rm -f $(OBJ)

clean:
	rm -f $(OBJ)
