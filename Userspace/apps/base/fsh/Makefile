WORKSPACE := ../../../

# Config file
include ../$(WORKSPACE)Makefile.conf

CC = ../$(WORKSPACE)$(COMPILER_PATH)/$(COMPILER_ARCH)gcc
CPP = ../$(WORKSPACE)$(COMPILER_PATH)/$(COMPILER_ARCH)g++
LD = ../$(WORKSPACE)$(COMPILER_PATH)/$(COMPILER_ARCH)ld
AS = ../$(WORKSPACE)$(COMPILER_PATH)/$(COMPILER_ARCH)as
OBJDUMP = ../$(WORKSPACE)$(COMPILER_PATH)/$(COMPILER_ARCH)objdump

GIT_COMMIT = $(shell git rev-parse HEAD)
GIT_COMMIT_SHORT = $(shell git rev-parse --short HEAD)

ifeq ($(OSARCH), amd64)
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./arch/i386/*" -not -path "./arch/aarch64/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./arch/i386/*" -not -path "./arch/aarch64/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./arch/i386/*" -not -path "./arch/aarch64/*")
else ifeq ($(OSARCH), i386)
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./arch/amd64/*" -not -path "./arch/aarch64/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./arch/amd64/*" -not -path "./arch/aarch64/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./arch/amd64/*" -not -path "./arch/aarch64/*")
else ifeq ($(OSARCH), aarch64)
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./arch/amd64/*" -not -path "./arch/i386/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./arch/amd64/*" -not -path "./arch/i386/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./arch/amd64/*" -not -path "./arch/i386/*")
endif
OBJ = $(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o) $(ASM_SOURCES:.asm=.o) $(S_SOURCES:.S=.o) $(PSF_SOURCES:.psf=.o) $(BMP_SOURCES:.bmp=.o)

SYSROOT = --sysroot=$(WORKSPACE)out/
FILENAME = sh

HEADERS = $(sort $(dir $(wildcard $(WORKSPACE)out/include/*)))

LDFLAGS =
CFLAGS = -I$(WORKSPACE)out/include				\
	-DGIT_COMMIT='"$(GIT_COMMIT)"'				\
	-DGIT_COMMIT_SHORT='"$(GIT_COMMIT_SHORT)"'
WARNCFLAG = -Wall -Wextra

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fverbose-asm
	LDFLAGS += -ggdb3 -O0
endif

build: $(FILENAME)
	mv $(FILENAME) $(WORKSPACE)out/bin/$(FILENAME)

$(FILENAME): $(OBJ)
	$(info Linking $@)
	$(CC) $(LDFLAGS) $(SYSROOT) $(OBJ) -o $@

%.o: %.c $(HEADERS)
	$(info Compiling $<)
	$(CC) $(CFLAGS) $(WARNCFLAG) -std=c17 -c $< -o $@

%.o: %.cpp $(HEADERS)
	$(info Compiling $<)
	$(CPP) $(CFLAGS) $(WARNCFLAG) -std=c++20 -fexceptions -c $< -o $@ -fno-rtti

%.o: %.S
	$(info Compiling $<)
	$(AS) -o $@ $<

clean:
	rm -f $(OBJ)
