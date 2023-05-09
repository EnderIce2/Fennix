# Config file
include ../Makefile.conf

KERNEL_FILENAME = kernel.fsys

CC = ../$(COMPILER_PATH)/$(COMPILER_ARCH)gcc
CPP = ../$(COMPILER_PATH)/$(COMPILER_ARCH)g++
LD = ../$(COMPILER_PATH)/$(COMPILER_ARCH)ld
AS = ../$(COMPILER_PATH)/$(COMPILER_ARCH)as
NM = ../$(COMPILER_PATH)/$(COMPILER_ARCH)nm
OBJCOPY = ../$(COMPILER_PATH)/$(COMPILER_ARCH)objcopy
OBJDUMP = ../$(COMPILER_PATH)/$(COMPILER_ARCH)objdump
GDB = ../$(COMPILER_PATH)/$(COMPILER_ARCH)gdb
RUSTC = /usr/bin/rustc
NASM = /usr/bin/nasm

RUST_TARGET_PATH = Architecture/$(OSARCH)/rust-target.json

GIT_COMMIT = $(shell git rev-parse HEAD)
GIT_COMMIT_SHORT = $(shell git rev-parse --short HEAD)

BMP_SOURCES = $(shell find ./ -type f -name '*.bmp')
PSF_SOURCES = $(shell find ./ -type f -name '*.psf')
ifeq ($(OSARCH), amd64)
ASM_SOURCES = $(shell find ./ -type f -name '*.asm' -not -path "./Architecture/i386/*" -not -path "./Architecture/aarch64/*")
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./Architecture/i386/*" -not -path "./Architecture/aarch64/*")
s_SOURCES = $(shell find ./ -type f -name '*.s' -not -path "./Architecture/i386/*" -not -path "./Architecture/aarch64/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./Architecture/i386/*" -not -path "./Architecture/aarch64/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./Architecture/i386/*" -not -path "./Architecture/aarch64/*")
RS_SOURCES = $(shell find ./ -type f -name '*.rs' -not -path "./Architecture/i386/*" -not -path "./Architecture/aarch64/*")
else ifeq ($(OSARCH), i386)
ASM_SOURCES = $(shell find ./ -type f -name '*.asm' -not -path "./Architecture/amd64/*" -not -path "./Architecture/aarch64/*")
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./Architecture/amd64/*" -not -path "./Architecture/aarch64/*")
s_SOURCES = $(shell find ./ -type f -name '*.s' -not -path "./Architecture/amd64/*" -not -path "./Architecture/aarch64/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./Architecture/amd64/*" -not -path "./Architecture/aarch64/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./Architecture/amd64/*" -not -path "./Architecture/aarch64/*")
RS_SOURCES = $(shell find ./ -type f -name '*.rs' -not -path "./Architecture/amd64/*" -not -path "./Architecture/aarch64/*")
else ifeq ($(OSARCH), aarch64)
ASM_SOURCES = $(shell find ./ -type f -name '*.asm' -not -path "./Architecture/amd64/*" -not -path "./Architecture/i386/*")
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./Architecture/amd64/*" -not -path "./Architecture/i386/*")
s_SOURCES = $(shell find ./ -type f -name '*.s' -not -path "./Architecture/amd64/*" -not -path "./Architecture/i386/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./Architecture/amd64/*" -not -path "./Architecture/i386/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./Architecture/amd64/*" -not -path "./Architecture/i386/*")
RS_SOURCES = $(shell find ./ -type f -name '*.rs' -not -path "./Architecture/amd64/*" -not -path "./Architecture/i386/*")
endif
HEADERS = $(sort $(dir $(wildcard ./include/*))) $(sort $(dir $(wildcard ./include_std/*)))
OBJ = $(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o) $(RS_SOURCES:.rs=.o) $(ASM_SOURCES:.asm=.o) $(S_SOURCES:.S=.o) $(s_SOURCES:.s=.o) $(PSF_SOURCES:.psf=.o) $(BMP_SOURCES:.bmp=.o)
STACK_USAGE_OBJ = $(C_SOURCES:.c=.su) $(CPP_SOURCES:.cpp=.su)
GCNO_OBJ = $(C_SOURCES:.c=.gcno) $(CPP_SOURCES:.cpp=.gcno)
INCLUDE_DIR = -I./include -I./include_std

LDFLAGS := -Wl,-Map kernel.map -shared -nostdlib -nodefaultlibs -nolibc

# Disable all warnings by adding "-w" in WARNCFLAG and if you want to treat the warnings as errors, add "-Werror"
WARNCFLAG = -Wall -Wextra \
			-Wfloat-equal -Wpointer-arith -Wcast-align \
			-Wredundant-decls -Winit-self -Wswitch-default \
			-Wstrict-overflow=5 -Wconversion -w

# https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html
CFLAGS :=										\
	$(INCLUDE_DIR)								\
	-DKERNEL_NAME='"$(OSNAME)"' 				\
	-DKERNEL_VERSION='"$(KERNEL_VERSION)"'		\
	-DGIT_COMMIT='"$(GIT_COMMIT)"'				\
	-DGIT_COMMIT_SHORT='"$(GIT_COMMIT_SHORT)"'

SIMD_FLAGS := -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -mavx -mavx2 -mavx512f

ifeq ($(OSARCH), amd64)

CFLAGS += -fno-pic -fno-pie					\
		  -mno-red-zone -march=core2 -pipe	\
		  -mcmodel=kernel -fno-builtin -Da64 -Da86
CFLAG_STACK_PROTECTOR := -fstack-protector-all
LDFLAGS += -TArchitecture/amd64/linker.ld 	\
	-fno-pic -fno-pie 						\
	-Wl,-static,--no-dynamic-linker,-ztext 	\
	-nostdlib -nodefaultlibs -nolibc  		\
	-zmax-page-size=0x1000					\
	-Wl,-Map kernel.map -shared

else ifeq ($(OSARCH), i386)

CFLAGS += -fno-pic -fno-pie -mno-80387 -mno-mmx -mno-3dnow	\
		  -mno-red-zone -march=pentium -pipe -msoft-float	\
		  -fno-builtin -Da32 -Da86
CFLAG_STACK_PROTECTOR := -fstack-protector-all
LDFLAGS += -TArchitecture/i386/linker.ld 	\
	-fno-pic -fno-pie 						\
	-Wl,-static,--no-dynamic-linker,-ztext 	\
	-nostdlib -nodefaultlibs -nolibc  		\
	-zmax-page-size=0x1000					\
	-Wl,-Map kernel.map -shared

else ifeq ($(OSARCH), aarch64)

CFLAGS += -pipe -fno-builtin -Wstack-protector -Daa64 -fPIC -mno-outline-atomics
CFLAG_STACK_PROTECTOR := -fstack-protector-all
LDFLAGS += -TArchitecture/aarch64/linker.ld -fPIC -pie \
	-Wl,-static,--no-dynamic-linker,-ztext \
	-nostdlib -nodefaultlibs -nolibc \
	-zmax-page-size=0x1000 \
	-Wl,-Map kernel.map -static

endif

ifeq ($(OSARCH), amd64)
NASMFLAGS := -f elf64
else ifeq ($(OSARCH), i386)
NASMFLAGS := -f elf32
else ifeq ($(OSARCH), aarch64)
NASMFLAGS :=
endif

# -finstrument-functions for __cyg_profile_func_enter & __cyg_profile_func_exit. Used for profiling and debugging.
ifeq ($(DEBUG), 1)
#	CFLAGS += --coverage
#	CFLAGS += -pg
#	CFLAGS += -finstrument-functions
	CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fverbose-asm -fstack-usage -fsanitize=undefined
ifneq ($(OSARCH), aarch64)
	CFLAGS += -fstack-check
endif
	LDFLAGS += -ggdb3 -O0
	NASMFLAGS += -F dwarf -g
	WARNCFLAG += -Wno-unused-function -Wno-maybe-uninitialized -Wno-builtin-declaration-mismatch -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-variable
ifeq ($(TESTING), 1)
	CFLAGS += -DTESTING
endif
endif

default:
	$(error Please specify a target)

prepare:
	$(info Nothing to prepare)

build: $(KERNEL_FILENAME)

dump:
ifeq (,$(wildcard $(KERNEL_FILENAME)))
	$(error $(KERNEL_FILENAME) does not exist)
endif
	$(info Dumping $(KERNEL_FILENAME) in AT T syntax...)
	$(OBJDUMP) -D -g -s -d $(KERNEL_FILENAME) > kernel_dump.map
	$(info Dumping $(KERNEL_FILENAME) in Intel syntax...)
	$(OBJDUMP) -M intel -D -g -s -d $(KERNEL_FILENAME) > kernel_dump_intel.map

$(KERNEL_FILENAME): $(OBJ)
	$(info Linking $@)
	$(CC) $(LDFLAGS) $(OBJ) -o $@

%.o: %.c $(HEADERS)
	$(info Compiling $<)
	$(CC) $(CFLAGS) $(CFLAG_STACK_PROTECTOR) $(WARNCFLAG) -std=c17 -c $< -o $@

# https://gcc.gnu.org/projects/cxx-status.html
%.o: %.cpp $(HEADERS)
	$(info Compiling $<)
	$(CPP) $(CFLAGS) $(CFLAG_STACK_PROTECTOR) $(WARNCFLAG) -std=c++20 -fexceptions -c $< -o $@ -fno-rtti

%.o: %.rs $(HEADERS) $(RUST_TARGET_PATH)
	$(info Compiling $<)
	$(RUSTC) $< -C panic=abort -C soft-float --emit=obj -o $@

%.o: %.asm
ifeq ($(OSARCH), aarch64)
	$(error aarch64 does not support NASM)
endif
	$(info Compiling $<)
	$(NASM) $< $(NASMFLAGS) -o $@

%.o: %.S
	$(info Compiling $<)
	$(AS) -c $< -o $@

%.o: %.s
	$(info Compiling $<)
	$(AS) -c $< -o $@

%.o: %.psf
ifeq ($(OSARCH), amd64)
	$(OBJCOPY) -O elf64-x86-64 -I binary $< $@
else ifeq ($(OSARCH), i386)
	$(OBJCOPY) -O elf32-i386 -I binary $< $@
else ifeq ($(OSARCH), aarch64)
	$(OBJCOPY) -O elf64-littleaarch64 -I binary $< $@
endif
	$(NM) $@

%.o: %.bmp
ifeq ($(OSARCH), amd64)
	$(OBJCOPY) -O elf64-x86-64 -I binary $< $@
else ifeq ($(OSARCH), i386)
	$(OBJCOPY) -O elf32-i386 -I binary $< $@
else ifeq ($(OSARCH), aarch64)
	$(OBJCOPY) -O elf64-littlearch64 -I binary $< $@
endif
	$(NM) $@

clean:
	rm -f *.bin *.o *.elf *.sym kernel.map kernel_dump.map kernel_dump_intel.map $(OBJ) $(STACK_USAGE_OBJ) $(GCNO_OBJ) $(KERNEL_FILENAME)
