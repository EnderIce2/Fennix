# Config file
include ../../../Makefile.conf

FILENAME = aip.drv

CC = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)gcc
CPP = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)g++
LD = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)ld
AS = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)as
OBJDUMP = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)objdump

S_SOURCES = $(shell find ./ -type f -name '*.S')
C_SOURCES = $(shell find ./ -type f -name '*.c')
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp')
HEADERS = $(sort $(dir $(wildcard ../../include/*)))
OBJ = $(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o) $(ASM_SOURCES:.asm=.o) $(S_SOURCES:.S=.o) $(PSF_SOURCES:.psf=.o) $(BMP_SOURCES:.bmp=.o)
STACK_USAGE_OBJ = $(C_SOURCES:.c=.su) $(CPP_SOURCES:.cpp=.su)
INCLUDE_DIR = ../../include

LIBS := ../../out/dcrt0.o -L../../out -ldriver

LDFLAGS := 															\
	-fPIC -fPIE -pie												\
	-Wl,--no-dynamic-linker,-ztext,--no-warn-rwx-segment 			\
	-nostdlib -nodefaultlibs -nolibc  								\
	-zmax-page-size=0x1000											\
	-Wl,-Map file.map -shared -fvisibility=hidden

WARNCFLAG = -Wall -Wextra

CFLAGS := -I$(INCLUDE_DIR) -fvisibility=hidden

ifeq ($(OSARCH), amd64)

CFLAGS += -fPIC -fPIE -pie -mno-80387 -mno-mmx -mno-3dnow	\
		  -mno-red-zone -mno-sse -mno-sse2					\
		  -march=x86-64 -pipe -ffunction-sections			\
		  -msoft-float -fno-builtin

else ifeq ($(OSARCH), i386)

CFLAGS += -fPIC -fPIE -pie -mno-80387 -mno-mmx -mno-3dnow		\
		  -mno-red-zone -mno-sse -mno-sse2 -ffunction-sections	\
		  -march=i386 -pipe -msoft-float -fno-builtin

else ifeq ($(OSARCH), aarch64)

CFLAGS += -pipe -fno-builtin -fPIC

endif

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fstack-usage
ifeq ($(OSARCH), amd64)
	CFLAGS += -fverbose-asm
endif
ifneq ($(OSARCH), aarch64)
	CFLAGS += -fstack-check
endif
	LDFLAGS += -ggdb3 -O0
endif

build: $(FILENAME)
	mv $(FILENAME) ../../out/$(FILENAME)

$(FILENAME): $(OBJ)
	$(info Linking $@)
	$(CC) $(LDFLAGS) $(OBJ) $(LIBS) -o $@

%.o: %.c $(HEADERS)
	$(info Compiling $<)
	$(CC) $(CFLAGS) $(WARNCFLAG) -std=c17 -c $< -o $@

%.o: %.cpp $(HEADERS)
	$(info Compiling $<)
	$(CPP) $(CFLAGS) $(WARNCFLAG) -std=c++20 -fno-exceptions -fno-rtti -c $< -o $@

%.o: %.S
	$(info Compiling $<)
	$(AS) -o $@ $<

clean:
	rm -f file.map $(OBJ) $(STACK_USAGE_OBJ)
