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
NASM = /usr/bin/nasm

GIT_COMMIT = $(shell git rev-parse HEAD)
GIT_COMMIT_SHORT = $(shell git rev-parse --short HEAD)

BMP_SOURCES = $(shell find ./ -type f -name '*.bmp')
PSF_SOURCES = $(shell find ./ -type f -name '*.psf')
ifeq ($(OSARCH), amd64)
ASM_SOURCES = $(shell find ./ -type f -name '*.asm' -not -path "./Architecture/i686/*" -not -path "./Architecture/aarch64/*")
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./Architecture/i686/*" -not -path "./Architecture/aarch64/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./Architecture/i686/*" -not -path "./Architecture/aarch64/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./Architecture/i686/*" -not -path "./Architecture/aarch64/*")
else ifeq ($(OSARCH), i686)
ASM_SOURCES = $(shell find ./ -type f -name '*.asm' -not -path "./Architecture/amd64/*" -not -path "./Architecture/aarch64/*")
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./Architecture/amd64/*" -not -path "./Architecture/aarch64/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./Architecture/amd64/*" -not -path "./Architecture/aarch64/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./Architecture/amd64/*" -not -path "./Architecture/aarch64/*")
else ifeq ($(OSARCH), aarch64)
ASM_SOURCES = $(shell find ./ -type f -name '*.asm' -not -path "./Architecture/amd64/*" -not -path "./Architecture/i686/*")
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./Architecture/amd64/*" -not -path "./Architecture/i686/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./Architecture/amd64/*" -not -path "./Architecture/i686/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./Architecture/amd64/*" -not -path "./Architecture/i686/*")
endif
HEADERS = $(sort $(dir $(wildcard ./include/*)))
OBJ = $(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o) $(ASM_SOURCES:.asm=.o) $(S_SOURCES:.S=.o) $(PSF_SOURCES:.psf=.o) $(BMP_SOURCES:.bmp=.o)
INCLUDE_DIR = ./include

LDFLAGS := -Wl,-Map kernel.map -shared -nostdlib -nodefaultlibs -nolibc

# Disable all warnings by adding "-w" in WARNCFLAG and if you want to treat the warnings as errors, add "-Werror"
WARNCFLAG = -Wall -Wextra

# https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html
CFLAGS :=										\
	-I$(INCLUDE_DIR)							\
	-DKERNEL_NAME='"$(OSNAME)"' 				\
	-DKERNEL_VERSION='"$(KERNEL_VERSION)"'		\
	-DGIT_COMMIT='"$(GIT_COMMIT)"'				\
	-DGIT_COMMIT_SHORT='"$(GIT_COMMIT_SHORT)"'

ifeq ($(OSARCH), amd64)

CFLAGS += -fno-pic -fno-pie -mno-80387 -mno-mmx -mno-3dnow	\
		  -mno-red-zone -mno-sse -mno-sse2					\
		  -march=x86-64 -pipe								\
		  -mcmodel=kernel -msoft-float -fno-builtin
CFLAG_STACK_PROTECTOR := -fstack-protector-all
LDFLAGS += -TArchitecture/amd64/linker.ld 	\
	-fno-pic -fno-pie 						\
	-Wl,-static,--no-dynamic-linker,-ztext 	\
	-nostdlib -nodefaultlibs -nolibc  		\
	-zmax-page-size=0x1000					\
	-Wl,-Map kernel.map -shared

else ifeq ($(OSARCH), i686)

CFLAGS += -fno-pic -fno-pie -mno-80387 -mno-mmx -mno-3dnow	\
		  -mno-red-zone -mno-sse -mno-sse2					\
		  -march=i686 -pipe -msoft-float -fno-builtin -fpermissive
CFLAG_STACK_PROTECTOR := -fstack-protector-all
LDFLAGS += -TArchitecture/i686/linker.ld 	\
	-fno-pic -fno-pie 						\
	-Wl,-static,--no-dynamic-linker,-ztext 	\
	-nostdlib -nodefaultlibs -nolibc  		\
	-zmax-page-size=0x1000					\
	-Wl,-Map kernel.map -shared

else ifeq ($(OSARCH), aarch64)

CFLAGS += -pipe -fno-builtin -fPIC -mgeneral-regs-only
CFLAG_STACK_PROTECTOR := -fstack-protector-all
LDFLAGS += -TArchitecture/aarch64/linker.ld -fPIC

endif

ifeq ($(OSARCH), amd64)
NASMFLAGS := -f elf64
else ifeq ($(OSARCH), i686)
NASMFLAGS := -f elf32
else ifeq ($(OSARCH), aarch64)
NASMFLAGS :=
endif

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -ggdb -O0 -fdiagnostics-color=always
	LDFLAGS += -ggdb -O0 -g
	NASMFLAGS += -felf32 -g
	WARNCFLAG += -Wno-unused-function -Wno-maybe-uninitialized -Wno-builtin-declaration-mismatch -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-variable
ifeq ($(TESTING), 1)
	CFLAGS += -DTESTING
endif
endif

default:
	$(error Please specify a target)

prepare:
	rm -f ./Files/ter-powerline-v12n.psf
	wget https://raw.githubusercontent.com/powerline/fonts/master/Terminus/PSF/ter-powerline-v12n.psf.gz -P Files
	gzip -d Files/ter-powerline-v12n.psf.gz

build: $(KERNEL_FILENAME)
	$(OBJDUMP) -D -d $(KERNEL_FILENAME) > kernel_dump.map

$(KERNEL_FILENAME): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@

%.o: %.c $(HEADERS)
	$(info Compiling $<)
	$(CC) $(CFLAGS) $(CFLAG_STACK_PROTECTOR) $(WARNCFLAG) -std=c17 -c $< -o $@

# https://gcc.gnu.org/projects/cxx-status.html
%.o: %.cpp $(HEADERS)
	$(info Compiling $<)
	$(CPP) $(CFLAGS) $(CFLAG_STACK_PROTECTOR) $(WARNCFLAG) -std=c++20 -fexceptions -c $< -o $@ -fno-rtti

%.o: %.asm
	$(info Compiling $<)
	$(NASM) $< $(NASMFLAGS) -o $@

%.o: %.S
	$(info Compiling $<)
ifeq ($(OSARCH), amd64)
	$(AS) -c $< -o $@
else ifeq ($(OSARCH), i686)
	$(AS) -c $< -o $@
else ifeq ($(OSARCH), aarch64)
	$(AS) -c $< -o $@
endif

%.o: %.psf
ifeq ($(OSARCH), amd64)
	$(OBJCOPY) -O elf64-x86-64 -I binary $< $@
else ifeq ($(OSARCH), i686)
	$(OBJCOPY) -O elf32-i386 -I binary $< $@
else ifeq ($(OSARCH), aarch64)
	$(OBJCOPY) -O elf64-littleaarch64 -I binary $< $@
endif
	$(NM) $@

%.o: %.bmp
ifeq ($(OSARCH), amd64)
	$(OBJCOPY) -O elf64-x86-64 -I binary $< $@
else ifeq ($(OSARCH), i686)
	$(OBJCOPY) -O elf32-i386 -I binary $< $@
else ifeq ($(OSARCH), aarch64)
	$(OBJCOPY) -O elf64-littlearch64 -I binary $< $@
endif
	$(NM) $@

clean:
	rm -f *.bin *.o *.elf *.sym kernel.map kernel_dump.map initrd.tar.gz $(OBJ) $(KERNEL_FILENAME)
